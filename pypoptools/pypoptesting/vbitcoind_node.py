import distutils.spawn
import http
import logging
import pathlib
import subprocess
import tempfile

from .framework.bin_util import assert_dir_accessible, get_open_port
from .framework.entities import *
from .framework.jsonrpc_api import JsonRpcApi, JSONRPCException
from .framework.node import Node
from .framework.util import wait_until

PORT_MIN = 15000
PORT_MAX = 25000
BIND_TO = '127.0.0.1'


class VBitcoindNode(Node):
    def __init__(self, number: int, datadir: pathlib.Path):
        self.number = number
        self.datadir = datadir
        assert_dir_accessible(datadir)

        self.rpc_timeout = 60  # sec
        self.stderr = None
        self.stdout = None

        # quickly check that vbitcoind is installed
        self.exe = distutils.spawn.find_executable("vbitcoind")
        if not self.exe:
            raise Exception("VBitcoinNode: vbitcoind is not found in PATH")

        self.p2p_port = get_open_port(PORT_MIN, PORT_MAX, BIND_TO)
        self.rpc_port = get_open_port(PORT_MIN, PORT_MAX, BIND_TO)
        self.user = 'testuser'
        self.password = 'testpassword'

        self.bitcoinconf = pathlib.Path(self.datadir, "vbitcoin.conf")
        with open(self.bitcoinconf, 'w', encoding='utf8') as f:
            f.write("regtest=1\n")
            f.write("[{}]\n".format("regtest"))
            f.write("port={}\n".format(self.p2p_port))
            f.write("rpcport={}\n".format(self.rpc_port))
            f.write("rpcuser={}\n".format(self.user))
            f.write("rpcpassword={}\n".format(self.password))
            f.write("fallbackfee=0.0002\n")
            f.write("server=1\n")
            f.write("keypool=1\n")
            f.write("discover=0\n")
            f.write("dnsseed=0\n")
            f.write("listenonion=0\n")
            f.write("printtoconsole=0\n")
            f.write("upnp=0\n")
            f.write("shrinkdebugfile=0\n")
            f.write("popvbknetwork=regtest\n")
            f.write("popbtcnetwork=regtest\n")
            f.write("poplogverbosity=info\n")

        self.url = "http://{}:{}/".format(BIND_TO, self.rpc_port)
        self.rpc = JsonRpcApi(url=self.url, user=self.user, password=self.password)
        # Add a new stdout and stderr file each time vbitcoind is started

        self.running = False
        self.args = [
            self.exe,
            "-datadir=" + str(self.datadir),
            "-logtimemicros",
            "-logthreadnames",
            "-debug",
            "-debugexclude=libevent",
            "-debugexclude=leveldb",
            "-txindex",
            "-uacomment=testnode{}".format(number)
        ]

        self.log = logging.getLogger('TestFramework.node%d' % number)
        self.process = None

    def __del__(self):
        # Ensure that we don't leave any vbitcoind processes lying around after
        # the test ends
        if self.process is not None:
            # Should only happen on test failure
            # Avoid using logger, as that may have already been shutdown when
            # this destructor is called.
            self.process.kill()

    def _node_msg(self, msg: str) -> str:
        """Return a modified msg that identifies this node by its index as a debugging aid."""
        return "[node %d] %s" % (self.number, msg)

    def start(self) -> None:
        self.stderr = tempfile.NamedTemporaryFile(prefix="stderr", dir=self.datadir, delete=False)
        self.stdout = tempfile.NamedTemporaryFile(prefix="stdout", dir=self.datadir, delete=False)
        self.process = subprocess.Popen(
            self.args,
            cwd=self.datadir,
            stdout=self.stdout,
            stderr=self.stderr
        )
        self.running = True
        self.log.debug("vbitcoind started, waiting for RPC to come up")

    def stop(self) -> None:
        """Stop the node."""
        if not self.running:
            return
        self.log.debug("Stopping node")
        self.running = False

        try:
            self.rpc.stop(wait=0)
        except http.client.CannotSendRequest:
            self.log.exception("Unable to stop node.")

        self.stdout.close()
        self.stderr.close()

    def restart(self) -> None:
        self.stop()
        self.start()

    def is_started(self) -> bool:
        if not self.process:
            return False

        if self.process.poll() is not None:
            self.running = False
            return False

        return self.running

    def is_rpc_available(self) -> bool:
        if self.process.poll() is not None:
            raise Exception(self._node_msg(
                'vbitcoind exited with status {} during initialization'.format(self.process.returncode)))
        try:
            self.rpc.getblockcount()
            # If the call to getblockcount() succeeds then the RPC connection is up
            self.log.debug("RPC successfully started")
            return True
        except Exception as e:
            self.log.debug("No RPC connection... {}".format(e))
            return False

    def connect(self, node):
        ip_port = "{}:{}".format(BIND_TO, node.p2p_port)
        self.rpc.addnode(ip_port, 'onetry')
        # poll until version handshake complete to avoid race conditions
        # with transaction relaying
        wait_until(lambda: all(peer['version'] != 0 for peer in self.rpc.getpeerinfo()))

    def disconnect(self, node):
        node_num = node.number
        for peer_id in [peer['id'] for peer in self.rpc.getpeerinfo() if
                        "testnode{}".format(node_num) in peer['subver']]:
            try:
                self.rpc.disconnectnode(address='', nodeid=peer_id)
            except JSONRPCException as e:
                # If this node is disconnected between calculating the peer id
                # and issuing the disconnect, don't worry about it.
                # This avoids a race condition if we're mass-disconnecting peers.
                if e.error['code'] != -29:  # RPC_CLIENT_NODE_NOT_CONNECTED
                    raise

        # wait to disconnect
        wait_until(
            lambda: not any(["testnode{}".format(node_num) in peer['subver'] for peer in self.rpc.getpeerinfo()]),
            timeout=5)

    def getnewaddress(self) -> str:
        return self.rpc.getnewaddress()

    def getpeerinfo(self) -> List[PeerInfo]:
        s = self.rpc.getpeerinfo()
        return [
            PeerInfo(
                id=x['id'],
                banscore=x['banscore']
            ) for x in s
        ]

    def getpayoutinfo(self, address: Optional[str]) -> Hexstr:
        if address is None:
            addr = self.getnewaddress()
            return self.getpayoutinfo(addr)

        s = self.rpc.validateaddress(address)
        return s['scriptPubKey']

    def getpopparams(self) -> PopParamsResponse:
        s = self.rpc.getpopparams()
        bootstrap = GenericBlock(
            hash=s['bootstrapBlock']['hash'],
            prevhash=s['bootstrapBlock']['previousBlock'],
            height=s['bootstrapBlock']['height']
        )

        vbkBootstrap = BlockAndNetwork(
            block=GenericBlock(
                hash=s['vbkBootstrapBlock']['hash'],
                prevhash=s['vbkBootstrapBlock'].get('previousBlock', ''),
                height=s['vbkBootstrapBlock']['height']
            ),
            network=s['vbkBootstrapBlock']['network']
        )

        btcBootstrap = BlockAndNetwork(
            block=GenericBlock(
                hash=s['btcBootstrapBlock']['hash'],
                prevhash=s['btcBootstrapBlock'].get('previousBlock', ''),
                height=s['btcBootstrapBlock']['height']
            ),
            network=s['btcBootstrapBlock']['network']
        )

        return PopParamsResponse(
            popActivationHeight=s['popActivationHeight'],
            popRewardPercentage=s['popRewardPercentage'],
            popRewardCoefficient=s['popRewardCoefficient'],
            popPayoutDelay=s['payoutParams']['popPayoutDelay'],
            bootstrapBlock=bootstrap,
            vbkBootstrap=vbkBootstrap,
            btcBootstrap=btcBootstrap,
            networkId=s['networkId'],
            maxVbkBlocksInAltBlock=s['maxVbkBlocksInAltBlock'],
            maxVTBsInAltBlock=s['maxVTBsInAltBlock'],
            maxATVsInAltBlock=s['maxATVsInAltBlock'],
            endorsementSettlementInterval=s['endorsementSettlementInterval'],
            finalityDelay=s['finalityDelay'],
            keystoneInterval=s['keystoneInterval'],
            maxAltchainFutureBlockTime=s['maxAltchainFutureBlockTime']
        )

    def submitpopatv(self, atv: Hexstr) -> SubmitPopResponse:
        s = self.rpc.submitpopatv(atv)
        return SubmitPopResponse(
            accepted=s['accepted'],
            code=s.get('code', ''),
            message=s.get('message', '')
        )

    def submitpopvtb(self, vtb: Hexstr) -> SubmitPopResponse:
        s = self.rpc.submitpopvtb(vtb)
        return SubmitPopResponse(
            accepted=s['accepted'],
            code=s.get('code', ''),
            message=s.get('message', '')
        )

    def submitpopvbk(self, vbk: Hexstr) -> SubmitPopResponse:
        s = self.rpc.submitpopvbk(vbk)
        return SubmitPopResponse(
            accepted=s['accepted'],
            code=s.get('code', ''),
            message=s.get('message', '')
        )

    def getpopdatabyheight(self, height: int) -> GetpopdataResponse:
        s = self.rpc.getpopdatabyheight(height)
        return GetpopdataResponse(
            header=s['block_header'],
            authenticated_context=s['authenticated_context']['serialized'],
            last_known_vbk_block=s['last_known_veriblock_blocks'][-1],
            last_known_btc_block=s['last_known_bitcoin_blocks'][-1],
        )

    def getpopdatabyhash(self, hash: Hexstr) -> GetpopdataResponse:
        s = self.rpc.getpopdatabyheight(hash)
        return GetpopdataResponse(
            header=s['block_header'],
            authenticated_context=s['authenticated_context']['serialized'],
            last_known_vbk_block=s['last_known_veriblock_blocks'][-1],
            last_known_btc_block=s['last_known_bitcoin_blocks'][-1],
        )

    def getbtcbestblockhash(self) -> Hexstr:
        return self.rpc.getbtcbestblockhash()

    def getvbkbestblockhash(self) -> Hexstr:
        return self.rpc.getvbkbestblockhash()

    def getbestblockhash(self) -> Hexstr:
        return self.rpc.getbestblockhash()

    def getbalance(self) -> float:
        return self.rpc.getbalance()

    def getrawatv(self, atvid: Hexstr) -> AtvResponse:
        s = self.rpc.getrawatv(atvid, 1)
        r = AtvResponse()
        r.in_active_chain = s['in_active_chain']
        r.confirmations = s['confirmations']
        if r.confirmations > 0:
            # in block
            r.blockhash = s['blockhash']
            r.blockheight = s['blockheight']
            r.containingBlocks = s['containing_blocks']

        a = s['atv']
        tx = a['transaction']
        pd = tx['publicationData']
        bop = a['blockOfProof']
        r.atv = ATV(
            id=a['id'],
            tx=VbkTx(
                hash=tx['hash'],
                publicationData=PublicationData(**pd)
            ),
            blockOfProof=GenericBlock(
                hash=bop['hash'],
                prevhash=bop['previousBlock'],
                height=bop['height']
            )
        )

        return r

    def getrawvtb(self, vtbid: Hexstr) -> VtbResponse:
        s = self.rpc.getrawvtb(vtbid, 1)
        r = VtbResponse()
        r.in_active_chain = s['in_active_chain']
        r.confirmations = s['confirmations']
        if r.confirmations > 0:
            # in block
            r.blockhash = s['blockhash']
            r.blockheight = s['blockheight']
            r.containingBlocks = s['containing_blocks']

        v = s['vtb']
        tx = v['transaction']
        cb = v['containingBlock']
        r.atv = VTB(
            id=v['id'],
            tx=VbkPopTx(
                hash=tx['hash'],
                publishedBlock=VbkBlock(**tx['publishedBlock']),
                blockOfProof=BtcBlock(**tx['blockOfProof']),
                blockOfProofContext=[BtcBlock(**x) for x in tx['blockOfProofContext']]
            ),
            containingBlock=GenericBlock(
                hash=cb['hash'],
                prevhash=cb['previousBlock'],
                height=cb['height']
            )
        )

        return r

    def getrawvbkblock(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        s = self.rpc.getrawvbkblock(vbkblockid, 1)
        r = VbkBlockResponse()
        r.in_active_chain = s['in_active_chain']
        r.confirmations = s['confirmations']
        if r.confirmations > 0:
            # in block
            r.blockhash = s['blockhash']
            r.blockheight = s['blockheight']
            r.containingBlocks = s['containing_blocks']

        r.vbkblock = VbkBlock(**s['vbkblock'])
        return r

    def getrawpopmempool(self) -> RawPopMempoolResponse:
        s = self.rpc.getrawpopmempool()
        return RawPopMempoolResponse(**s)

    def generate(self, nblocks: int, address: Optional[str] = None) -> List[Hexstr]:
        return self.rpc.generatetoaddress(nblocks, address or self.getnewaddress())

    def getblockhash(self, height: int) -> Hexstr:
        return self.rpc.getblockhash(height)

    def getblock(self, hash: Hexstr) -> BlockWithPopData:
        s = self.rpc.getblock(hash)
        return BlockWithPopData(
            hash=s['hash'],
            height=s['height'],
            prevhash=s['previousblockhash'],
            confirmations=s['confirmations'],
            endorsedBy=s['pop']['state']['endorsedBy'],
            blockOfProofEndorsements=[],
            containingATVs=s['pop']['data']['atvs'],
            containingVTBs=s['pop']['data']['vtbs'],
            containingVBKs=s['pop']['data']['vbkblocks']
        )

    def getblockcount(self) -> int:
        return self.rpc.getblockcount()

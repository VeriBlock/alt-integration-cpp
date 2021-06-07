import os

from pathlib import Path

from pypoptools.pypoptesting.framework.bin_util import assert_dir_accessible, get_open_port
from pypoptools.pypoptesting.framework.entities import *
from pypoptools.pypoptesting.framework.json_rpc import JsonRpcApi, JsonRpcException
from pypoptools.pypoptesting.framework.managers import ProcessManager
from pypoptools.pypoptesting.framework.node import Node
from pypoptools.pypoptesting.framework.sync_util import wait_until
from .altchain_family_enum import AltchainFamily

PORT_MIN = 15000
PORT_MAX = 25000
BIND_TO = '127.0.0.1'


class VGethNode(Node):
    def __init__(self, number: int, datadir: Path):
        self.number = number
            
        self.p2p_port = get_open_port(PORT_MIN, PORT_MAX, BIND_TO)

        rpc_port = get_open_port(PORT_MIN, PORT_MAX, BIND_TO)
        rpc_url = "http://{}:{}/".format(BIND_TO, rpc_port)
        self.rpc = JsonRpcApi(rpc_url)

        geth_path = os.environ.get('GETH_PATH')
        if geth_path == None:
            raise Exception("GETH_PATH env var is not set. Set up the path to the geth binary to the GETH_PATH env var")
        
        exe = Path(Path.cwd(), geth_path)
        if not exe:
            raise Exception("VGethNode: geth is not found in PATH")

        assert_dir_accessible(datadir)
        args = [
            exe,
            "--popdev",
            '--port=' + str(self.p2p_port),
            "--http",
            "--http.addr=" + BIND_TO,
            "--http.port=" + str(rpc_port),
            "--http.api=admin,web3,eth,personal,pop,miner,net",
            "--datadir=" + str(datadir),
            "--nodiscover",
            "--debug",
            "--poplogverbosity=info"
        ]
        self.manager = ProcessManager(args, datadir)

    def enode(self):
        enode = self.rpc.admin_nodeInfo()['enode']
        enode_prefix = enode.split('@')[0]
        return "{}@{}:{}".format(enode_prefix, BIND_TO, self.p2p_port)

    def start(self) -> None:
        self.manager.start()
        wait_until(lambda: self.is_rpc_available(), timeout=60)

    def is_rpc_available(self) -> bool:
        try:
            self.rpc.eth_blockNumber()
            return True
        except Exception as e:
            return False

    def stop(self) -> None:
        self.manager.stop()
        wait_until(lambda: not self.is_rpc_available(), timeout=60)

    def connect(self, node):
        self.rpc.admin_addPeer(node.enode())
        # poll until version handshake complete to avoid race conditions
        # with transaction relaying
        
        wait_until(lambda: any(node.enode().split(':')[1] == peer['enode'].split(':')[1] and peer['protocols']['eth'] != 'handshake' for peer in self.rpc.admin_peers()), timeout=60)

    def disconnect(self, node):
        try:
            self.rpc.admin_removePeer(node.enode())
        except JsonRpcException as e:
            # If this node is disconnected between calculating the peer id
            # and issuing the disconnect, don't worry about it.
            # This avoids a race condition if we're mass-disconnecting peers.
            if e.error['code'] != -29:  # RPC_CLIENT_NODE_NOT_CONNECTED
                raise

        # wait to disconnect
        wait_until(lambda: not any(node.enode().split(':')[1] == peer['enode'].split(':')[1] for peer in self.rpc.admin_peers()), timeout=60)

    def getpeers(self) -> List[Peer]:
        s = self.rpc.admin_peers()
        return [
            Peer(
                id=x['id'],
                banscore=0
            )
            for x in s
        ]

    def getbalance(self) -> float:
        balance = 0
        for account in self.rpc.eth_accounts():
            account_balance_hex = self.rpc.eth_getBalance(account, 'latest')
            account_balance = int(account_balance_hex, 16)
            balance += account_balance
        return balance

    def getnewaddress(self) -> str:
        return self.rpc.personal_newAccount("")

    def getpayoutinfo(self, address: str = None) -> Hexstr:
        address = address or self.getnewaddress()
        return Hexstr(address)

    def generate(self, nblocks: int, address: str = None) -> None:
        address = address or self.getnewaddress()
        self.rpc.miner_generateBlocks(nblocks, address)

    def getbestblockhash(self) -> Hexstr:
        return self.rpc.eth_getBlockByNumber('latest', False)['hash']

    def getblock(self, hash: Hexstr) -> BlockWithPopData:
        s = self.rpc.pop_getBlockByHash(hash)
        return BlockWithPopData(
            hash=s['hash'],
            height=int(s['number'], 16),
            prevhash=s['parentHash'],
            confirmations=s['confirmations'],
            blockOfProofEndorsements=[],
            containingATVs=[atv['id'] for atv in s['pop']['data']['atvs']],
            containingVTBs=[vtb['id'] for vtb in s['pop']['data']['vtbs']],
            containingVBKs=[vbk['id'] for vbk in s['pop']['data']['vbkblocks']],
            endorsedBy=s['pop']['state']['endorsedBy']
        )

    def getblockcount(self) -> int:
        return int(self.rpc.eth_blockNumber(), 16)

    def getblockhash(self, height: int) -> Hexstr:
        return self.rpc.eth_getBlockByNumber(height, False)['hash']

    def getbtcbestblockhash(self) -> Hexstr:
        return self.rpc.pop_getBtcBestBlockHash()

    def getpopdatabyhash(self, hash: Hexstr) -> GetpopdataResponse:
        s = self.rpc.pop_getPopDataByHash(hash)
        return GetpopdataResponse(
            header=s['block_header'],
            authenticated_context=s['authenticated_context'],
            last_known_vbk_block=s['last_known_veriblock_block'],
            last_known_btc_block=s['last_known_bitcoin_block'],
        )

    def getpopdatabyheight(self, height: int) -> GetpopdataResponse:
        s = self.rpc.pop_getPopDataByHeight(height)
        return GetpopdataResponse(
            header=s['block_header'],
            authenticated_context=s['authenticated_context'],
            last_known_vbk_block=s['last_known_veriblock_block'],
            last_known_btc_block=s['last_known_bitcoin_block'],
        )

    def getpopparams(self) -> PopParamsResponse:
        s = self.rpc.pop_getPopParams()
        bootstrap = GenericBlock(
            hash=s['bootstrapBlock']['hash'],
            prevhash=s['bootstrapBlock']['previousBlock'],
            height=s['bootstrapBlock']['height']
        )

        vbkBootstrap = BlockAndNetwork(
            block=GenericBlock(
                hash=s['vbkBootstrapBlock']['hash'],
                prevhash=s['vbkBootstrapBlock']['previousBlock'],
                height=s['vbkBootstrapBlock']['height']
            ),
            network=s['vbkBootstrapBlock']['network']
        )

        btcBootstrap = BlockAndNetwork(
            block=GenericBlock(
                hash=s['btcBootstrapBlock']['hash'],
                prevhash=s['btcBootstrapBlock']['previousBlock'],
                height=s['btcBootstrapBlock']['height']
            ),
            network=s['btcBootstrapBlock']['network']
        )

        return PopParamsResponse(
            popActivationHeight=s['popActivationHeight'],
            popRewardPercentage=s['popRewardPercentage'],
            popRewardCoefficient=s['popRewardCoefficient'],
            popPayoutDelay=s['popPayoutDelay'],
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

    def getrawatv(self, atvid: Hexstr) -> AtvResponse:
        s = self.rpc.pop_getRawAtv(atvid, 1)
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

    def getrawpopmempool(self) -> RawPopMempoolResponse:
        s = self.rpc.pop_getRawPopMempool()
        return RawPopMempoolResponse(**s)

    def getrawvbkblock(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        s = self.rpc.pop_getRawVbk(vbkblockid, 1)
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

    def getrawvtb(self, vtbid: Hexstr) -> VtbResponse:
        s = self.rpc.pop_getRawAtv(vtbid, 1)
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

    def getvbkbestblockhash(self) -> Hexstr:
        return self.rpc.pop_getVbkBestBlockHash()

    def submitpopatv(self, atv: Hexstr) -> SubmitPopResponse:
        s = self.rpc.pop_submitPopAtv(atv)
        return SubmitPopResponse(
            accepted=bool(s['accepted']),
            code=s['code'],
            message=s['message']
        )

    def submitpopvbk(self, vbk: Hexstr) -> SubmitPopResponse:
        s = self.rpc.pop_submitPopVbk(vbk)
        return SubmitPopResponse(
            accepted=s['accepted'],
            code=s['code'],
            message=s['message']
        )

    def submitpopvtb(self, vtb: Hexstr) -> SubmitPopResponse:
        s = self.rpc.pop_submitPopVtb(vtb)
        return SubmitPopResponse(
            accepted=s['accepted'],
            code=s['code'],
            message=s['message']
        )

    def altchainfamilyname(self):
        return AltchainFamily.ETHEREUM
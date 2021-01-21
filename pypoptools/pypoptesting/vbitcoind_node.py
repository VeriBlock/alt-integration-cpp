import errno
import http
import logging
import os
import pathlib
import subprocess
import tempfile
import time

from requests import post
from requests.auth import HTTPBasicAuth

from pypoptesting.framework.bin_util import assert_dir_accessible, get_open_port
from pypoptesting.framework.entities import Hexstr, BlockWithPopData, RawPopMempoolResponse, VbkBlockResponse, \
    VtbResponse, AtvResponse, GetpopdataResponse, SubmitPopResponse, PopParamsResponse
from pypoptesting.framework.jsonrpc_api import JsonRpcApi, JSONRPCException
from pypoptesting.framework.node import Node

import distutils.spawn

from pypoptesting.framework.util import wait_until

PORT_MIN = 15000
PORT_MAX = 25000
BIND_TO = '127.0.0.1'


class VBitcoindNode(Node):
    def __init__(self, number: int, datadir: pathlib.Path):
        self.number = number
        self.datadir = datadir
        assert_dir_accessible(datadir)

        self.rpc_timeout = 60  # sec
        self.stderr = tempfile.NamedTemporaryFile(dir=self.datadir, delete=False)
        self.stdout = tempfile.NamedTemporaryFile(dir=self.datadir, delete=False)

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

        self.url = "http://{h}:{p}/".format(
            h=BIND_TO,
            p=self.rpc_port
        )
        self.rpc = JsonRpcApi(url=self.user, user=self.user, password=self.password)
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
        if self.process:
            # Should only happen on test failure
            # Avoid using logger, as that may have already been shutdown when
            # this destructor is called.
            print(self._node_msg("Cleaning up leftover process"))
            self.process.kill()

    def _node_msg(self, msg: str) -> str:
        """Return a modified msg that identifies this node by its index as a debugging aid."""
        return "[node %d] %s" % (self.number, msg)

    def start(self) -> None:
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

        if self.process.poll() is None:
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
            self.rpc_connected = True
            return True
        except IOError as e:
            if e.errno != errno.ECONNREFUSED:  # Port not yet open?
                raise  # unknown IO error
        except JSONRPCException as e:  # Initialization phase
            # -28 RPC in warmup
            # -342 Service unavailable, RPC server started but is shutting down due to error
            if e.error['code'] != -28 and e.error['code'] != -342:
                raise  # unknown JSON RPC exception
        except ValueError as e:  # cookie file not found and no rpcuser or rpcassword. vbitcoind still starting
            if "No RPC credentials" not in str(e):
                raise

        return False

    def connect(self, node):
        ip_port = "{}:{}".format(BIND_TO, node.p2p_port)
        self.rpc.addnode(ip_port, 'onetry')
        # poll until version handshake complete to avoid race conditions
        # with transaction relaying
        wait_until(lambda: all(peer['version'] != 0 for peer in self.rpc.getpeerinfo()))

    def disconnect(self, node):
        from_connection = self.rpc
        node_num = node.number
        for peer_id in [peer['id'] for peer in from_connection.getpeerinfo() if
                        "testnode%d" % node_num in peer['subver']]:
            try:
                from_connection.disconnectnode(nodeid=peer_id)
            except JSONRPCException as e:
                # If this node is disconnected between calculating the peer id
                # and issuing the disconnect, don't worry about it.
                # This avoids a race condition if we're mass-disconnecting peers.
                if e.error['code'] != -29:  # RPC_CLIENT_NODE_NOT_CONNECTED
                    raise

        # wait to disconnect
        wait_until(lambda: [peer['id'] for peer in from_connection.getpeerinfo() if
                            "testnode%d" % node_num in peer['subver']] == [], timeout=5)

    def getpopparams(self) -> PopParamsResponse:
        pass

    def submitpopatv(self, atv: Hexstr) -> SubmitPopResponse:
        pass

    def submitpopvtb(self, vtb: Hexstr) -> SubmitPopResponse:
        pass

    def submitpopvbk(self, vbk: Hexstr) -> SubmitPopResponse:
        pass

    def getpopdatabyheight(self, height: int) -> GetpopdataResponse:
        pass

    def getpopdatabyhash(self, hash: Hexstr) -> GetpopdataResponse:
        pass

    def getbtcbestblockhash(self) -> Hexstr:
        pass

    def getvbkbestblockhash(self) -> Hexstr:
        pass

    def getbestblockhash(self) -> Hexstr:
        pass

    def getrawatv(self, atvid: Hexstr) -> AtvResponse:
        pass

    def getrawvtb(self, vtbid: Hexstr) -> VtbResponse:
        pass

    def getrawvbkblock(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        pass

    def getrawpopmempool(self) -> RawPopMempoolResponse:
        pass

    def generate(self, nblocks: int) -> None:
        pass

    def getblockhash(self, height: int) -> Hexstr:
        pass

    def getblock(self, hash: Hexstr) -> BlockWithPopData:
        pass

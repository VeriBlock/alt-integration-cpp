from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.sync_util import start_all

class RpcPopRpcFunctionsTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def setup_nodes(self):
        start_all(self.nodes)

    def run_test(self):
        res = self.nodes[0].getpoprpcfunctions()

        assert len(res) == 12
        assert res['getpopdatabyhash'] != None
        assert res['submitpopatv'] != None
        assert res['submitpopvtb'] != None
        assert res['submitpopvbk'] != None
        assert res['getrawvtb'] != None
        assert res['getrawvbk'] != None
        assert res['getrawatv'] != None
        assert res['getrawpopmempool'] != None
        assert res['getpopparams'] != None
        assert res['getpopdatabyheight'] != None
        assert res['getbtcbestblockhash'] != None
        assert res['getvbkbestblockhash'] != None

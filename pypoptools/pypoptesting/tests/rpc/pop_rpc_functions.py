from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.sync_util import start_all

class RpcPopRpcFunctionsTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def setup_nodes(self):
        start_all(self.nodes)

    def run_test(self):
        res = self.nodes[0].getpoprpcfunctions()
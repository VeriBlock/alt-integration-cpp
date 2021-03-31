from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.sync_util import start_all

class NodeStartTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def run_test(self):
        start_all(self.nodes)

        for node in self.nodes:
            assert node.getbestblock().height == 0

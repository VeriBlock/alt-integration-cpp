from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.sync_util import start_all

class NodeStopTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def setup_nodes(self):
        start_all(self.nodes)

    def run_test(self):
        for node in self.nodes:
            node.stop()

        for node in self.nodes:
            try:
                node.getbestblock()
                assert False
            except Exception as e:
                pass

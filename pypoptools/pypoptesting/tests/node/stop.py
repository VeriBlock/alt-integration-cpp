from pypoptesting.framework.test_framework import PopIntegrationTestFramework


class NodeStopTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def run_test(self):
        for node in self.nodes:
            node.stop()

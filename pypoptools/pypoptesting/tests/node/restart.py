from pypoptesting.framework.test_framework import PopIntegrationTestFramework


class NodeRestartTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def run_test(self):
        for node in self.nodes:
            node.restart()

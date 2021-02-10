from ...framework.test_framework import PopIntegrationTestFramework


class NodeStartTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def setup_nodes(self):
        pass

    def run_test(self):
        for node in self.nodes:
            node.start()

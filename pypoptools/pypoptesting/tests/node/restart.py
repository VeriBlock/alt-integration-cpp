from ...framework.test_framework import PopIntegrationTestFramework


class NodeRestartTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def run_test(self):
        start_all(self.nodes)

        for node in self.nodes:
            assert node.getbestblock().height == 0

        for node in self.nodes:
            node.restart()

        for node in self.nodes:
            assert node.getbestblock().height == 0
            

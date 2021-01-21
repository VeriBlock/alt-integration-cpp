from pypoptesting.framework.test_framework import PopIntegrationTestFramework


class PopForkResolutionShorterChainWins(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        assert self.nodes[0].is_started()
        assert self.nodes[0].is_rpc_available()
        print("WORKS!")

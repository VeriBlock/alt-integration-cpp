from ..framework.test_framework import PopIntegrationTestFramework


class PopForkResolutionShorterChainWins(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        assert self.nodes[0].isstarted()
        assert self.nodes[0].isrpcavailable()

        addr = self.nodes[0].getnewaddress()

        best = self.nodes[0].getbestblockhash()
        self.nodes[0].generate(100, addr)
        best2 = self.nodes[0].getbestblockhash()

        assert best != best2, "Best blocks are equal! Pre={}, post={}".format(best, best2)

        # TODO: implement actual test
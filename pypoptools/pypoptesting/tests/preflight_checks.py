from ..framework.test_framework import PopIntegrationTestFramework


class PreflightChecks(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        assert self.nodes[0].isstarted()
        assert self.nodes[0].isrpcavailable()

        addr = self.nodes[0].getnewaddress()
        self.log.info("new address={}".format(addr))

        best = self.nodes[0].getbestblockhash()
        balance1 = self.nodes[0].getbalance()

        N = 500
        self.log.info("generating {} blocks...".format(N))
        self.nodes[0].generate(N, addr)
        best2 = self.nodes[0].getbestblockhash()
        balance2 = self.nodes[0].getbalance()

        assert best != best2
        assert balance1 < balance2

        self.nodes[0].restart()

        best3 = self.nodes[0].getbestblockhash()
        assert best != best3

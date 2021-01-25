from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.test_util import assert_greater_than, assert_not_equal
from ..framework.util import wait_for_rpc_availability


class PreflightChecks(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        assert self.nodes[0].is_started()
        assert self.nodes[0].is_rpc_available()

        addr = self.nodes[0].getnewaddress()
        self.log.info("new address={}".format(addr))

        best = self.nodes[0].getbestblockhash()
        balance1 = self.nodes[0].getbalance(addr)

        N = 500
        self.log.info("generating {} blocks...".format(N))
        self.nodes[0].generate(N, addr)
        best2 = self.nodes[0].getbestblockhash()
        balance2 = self.nodes[0].getbalance(addr)

        assert_not_equal(best, best2)
        assert_greater_than(balance2, balance1)

        self.nodes[0].restart()
        wait_for_rpc_availability(self.nodes[0])

        best3 = self.nodes[0].getbestblockhash()
        assert_not_equal(best, best3)

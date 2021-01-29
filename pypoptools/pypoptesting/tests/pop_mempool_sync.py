"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import endorse_block


class PopMempoolSync(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypopminer import MockMiner
        apm = MockMiner()
        addr0 = self.nodes[0].getnewaddress()

        self.log.info("node0 endorses block 5")
        self.nodes[0].generate(nblocks=10)

        tip_height = self.nodes[0].getbestblock().height
        atv_id = endorse_block(self.nodes[0], apm, tip_height - 5, addr0)

        self.sync_pop_mempools(self.nodes, timeout=20)
        self.log.info("nodes[0,1] have syncd pop mempools")

        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id in rawpopmempool1.atvs
        self.log.info("node1 contains atv1 in its pop mempool")

        self.nodes[1].restart()
        self.log.info("node1 has been restarted")
        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id not in rawpopmempool1.atvs
        self.log.info("node1 does not contain atv1 in its pop mempool after restart")

        self.nodes[0].connect(self.nodes[1])
        self.log.info("node1 connect to node0")

        self.sync_pop_mempools(self.nodes, timeout=20)
        self.log.info("nodes[0,1] have syncd pop mempools")

        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id in rawpopmempool1.atvs

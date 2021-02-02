"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.pop_util import endorse_block, mine_until_pop_enabled
from ..framework.sync_util import (
    start_all_and_wait, connect_all,
    sync_all, sync_pop_mempools, wait_for_rpc_availability
)


class PopMempoolSync(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all_and_wait(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        connect_all(self.nodes)
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self.log.info("node0 endorses block 5")
        self.nodes[0].generate(nblocks=10)

        tip_height = self.nodes[0].getblockcount()
        atv_id = endorse_block(self.nodes[0], apm, tip_height - 5)

        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id not in rawpopmempool1.atvs
        self.log.info("node1 does not contain atv1 in its pop mempool after restart")

        sync_pop_mempools(self.nodes, timeout=20)
        self.log.info("nodes[0,1] have syncd pop mempools")

        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id in rawpopmempool1.atvs
        self.log.info("node1 contains atv1 in its pop mempool")

        self.nodes[1].restart()
        wait_for_rpc_availability(self.nodes[1])
        self.log.info("node1 has been restarted")
        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id not in rawpopmempool1.atvs
        self.log.info("node1 does not contain atv1 in its pop mempool after restart")

        self.nodes[0].connect(self.nodes[1])
        self.log.info("node1 connect to node0")

        sync_pop_mempools(self.nodes, timeout=20)
        self.log.info("nodes[0,1] have syncd pop mempools")

        rawpopmempool1 = self.nodes[1].getrawpopmempool()
        assert atv_id in rawpopmempool1.atvs

"""
Start 1 node.
Mine MaxReorgDistance + PreserveBlocksBehindFinal of blocks.
Make sure that the first block was erased.
"""
from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.generate_util import generate_endorsed
from ...framework.pop_util import create_endorsed_chain, endorse_block, mine_until_pop_enabled
from ...framework.sync_util import (
    start_all, connect_all,
    sync_all, sync_blocks, sync_pop_tips,
    wait_for_block_height, disconnect_all
)


class PopFinalizationTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])

        popparams = self.nodes[0].getpopparams()

        self.log.info(popparams)
        self.max_reorg = popparams.maxReorgDistance
        self.endorsement_settlement = popparams.endorsementSettlementInterval

        self.log.info(self.max_reorg)
        self.log.info(self.endorsement_settlement)


    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._basic_finalization_test(apm)

    def _basic_finalization_test(self, apm):
        self.log.info("starting _basic_finalization_test()")
        last_block = self.nodes[0].getblockcount()

        to_mine = self.max_reorg + self.endorsement_settlement + 5
        self.nodes[0].generate(nblocks=to_mine)
        self.log.info("node0 mined {} blocks".format(to_mine))
        assert self.nodes[0].getbestblock().height == last_block + to_mine

        # connect all nodes to each other
        connect_all(self.nodes)

        self.log.info("all nodes connected")
        sync_blocks(self.nodes, timeout=60)
        sync_pop_tips(self.nodes, timeout=60)
        self.log.info("all nodes have common tip")

        expected_best = self.nodes[0].getbestblock()
        best_blocks = [node.getbestblock() for node in self.nodes]
        for best in best_blocks:
            assert best == expected_best

        self.log.info("_basic_finalization_test() succeeded!")
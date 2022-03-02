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

        self.max_reorg = popparams.maxReorgBlocks
        self.endorsement_settlement = popparams.endorsementSettlementInterval

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._basic_finalization_test(apm)

    def _basic_finalization_test(self, apm):
        self.log.info("starting _basic_finalization_test()")
        last_block = self.nodes[0].getblockcount()
        self.nodes[0].generate(nblocks=5)

        self.log.info("endorse {} alt block".format(last_block + 1))
        endorse_block(self.nodes[0], apm, last_block + 1)

        to_mine = self.max_reorg + self.endorsement_settlement
        self.nodes[0].generate(nblocks=to_mine)
        self.log.info("node0 mined {} blocks".format(to_mine))
        assert self.nodes[0].getbestblock().height == last_block + to_mine + 5

        before_erased_block_hash = self.nodes[0].getblockhash(last_block + 1)
        before_erased_block = self.nodes[0].getblock(before_erased_block_hash)

        self.log.info(before_erased_block)

        # make sure blocks are saved on disk before finalizing
        self.log.info("restarting node0")
        self.nodes[0].restart()

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

        self.log.info("nodes successfully synced")
        self.log.info("trying getblock {}".format(last_block + 1))

        erased_block = self.nodes[0].getblock(before_erased_block_hash)

        assert erased_block != before_erased_block

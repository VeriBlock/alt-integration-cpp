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

        addr0 = self.nodes[0].getnewaddress()
        atv_id = endorse_block(self.nodes[0], apm, last_block, addr0)
        self.log.info("node0 endorsed block %d", last_block)
        # mine pop tx on node0
        self.nodes[0].generate(nblocks=1)

        erased_block_hash = self.nodes[0].getblockhash(last_block + 1)
        erased_block = self.nodes[0].getblock(erased_block_hash)
        self.log.info(erased_block)

        to_mine = self.max_reorg + self.endorsement_settlement + 5
        self.nodes[0].generate(nblocks=to_mine)
        self.log.info("node0 mined {} blocks".format(to_mine))
        assert self.nodes[0].getbestblock().height == last_block + 1 + to_mine

        erased_block_hash = self.nodes[0].getblockhash(last_block + 1)
        erased_block = self.nodes[0].getblock(erased_block_hash)
        self.log.info(erased_block)

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

        addr0 = self.nodes[0].getnewaddress()
        atv_id = endorse_block(self.nodes[0], apm, last_block + 1, addr0)
        self.log.info("node0 endorsed block %d (should be deleted after finalization)", last_block + 1)
        # mine pop tx on node0
        self.nodes[0].generate(nblocks=1)

        containing_block = self.nodes[0].getbestblock()
        self.nodes[0].generate(nblocks=9)
        self.log.info("node0 mines 10 more blocks")
        sync_all([self.nodes[0], self.nodes[1]])

        assert self.nodes[1].getblock(containing_block.hash).hash == containing_block.hash

        self.log.info("_basic_finalization_test() succeeded!")
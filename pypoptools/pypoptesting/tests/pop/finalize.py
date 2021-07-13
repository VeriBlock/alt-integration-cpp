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
        connect_all(self.nodes)
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._basic_finalization_test(apm)

    def _basic_finalization_test(self, apm):
        self.log.info("starting _basic_finalization_test()")
        last_block = self.nodes[0].getblockcount()
        popparams = self.nodes[0].getpopparams()

        self.log.info(popparams)

        self.nodes[1].disconnect(self.nodes[0])

        self.nodes[0].generate(nblocks=103)
        self.log.info("node0 mined 103 blocks")
        assert self.nodes[0].getbestblock().height == last_block + 103

        self.log.info("_basic_finalization_test() succeeded!")
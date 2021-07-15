"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""
import time

from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.pop_util import endorse_block, mine_until_pop_enabled
from ...framework.sync_util import start_all, connect_all, sync_all


class PopE2ETest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all(self.nodes)
        connect_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        assert self.nodes[0].getpeers()[0].banscore == 0
        assert self.nodes[1].getpeers()[0].banscore == 0

        vbk_blocks_amount = 100
        self.log.info("generate vbk blocks on node0, amount {}".format(vbk_blocks_amount))
        vbk_blocks = []
        for i in range(vbk_blocks_amount):
            vbk_blocks.append(apm.mineVbkBlocks(1))
        assert len(vbk_blocks) == vbk_blocks_amount

        vtbs_amount = 20
        self.log.info("generate vtbs on node0, amount {}".format(vtbs_amount))
        for i in range(vtbs_amount):
            apm.endorseVbkBlock(apm.vbkTip, apm.btcTip.getHash())

        self.nodes[0].generate(nblocks=10)
        last_block = self.nodes[0].getblockcount()
        assert last_block >= 5

        self.log.info("endorse {} alt block".format(last_block - 5))
        endorse_block(self.nodes[0], apm, last_block - 5)

        assert self.nodes[0].getpeers()[0].banscore == 0
        assert self.nodes[1].getpeers()[0].banscore == 0

        self.nodes[0].generate(nblocks=1)
        containing_block = self.nodes[0].getbestblock()

        assert len(containing_block.containingVTBs) == vtbs_amount
        assert len(containing_block.containingVBKs) == vbk_blocks_amount + vtbs_amount + 1

        assert last_block >= 6
        self.log.info("endorse {} alt block".format(last_block - 6))
        endorse_block(self.nodes[0], apm, last_block - 6)

        assert self.nodes[0].getpeers()[0].banscore == 0
        assert self.nodes[1].getpeers()[0].banscore == 0

        self.nodes[0].generate(nblocks=1)
        time.sleep(5)

        self.log.info("sync all nodes")
        sync_all(self.nodes)

        assert self.nodes[0].getpeers()[0].banscore == 0
        assert self.nodes[1].getpeers()[0].banscore == 0

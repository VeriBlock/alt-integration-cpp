from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.pop_util import mine_vbk_blocks, mine_until_pop_enabled
from ...framework.sync_util import start_all, connect_all, sync_all


class PopMempoolReorgTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        connect_all(self.nodes)
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self.nodes[0].generate(nblocks=10)
        sync_all(self.nodes)

        self.log.info("disconnect nodes 0 and 1")
        self.nodes[0].disconnect(self.nodes[1])

        vbk_blocks_amount = 10
        self.log.info("generate {} vbk blocks".format(vbk_blocks_amount))
        vbk_blocks = mine_vbk_blocks(self.nodes[0], apm, vbk_blocks_amount)

        # mine a block on node[0] with these vbk blocks
        node0_tip_hash = self.nodes[0].generate(nblocks=1)[0]
        node0_tip = self.nodes[0].getblock(node0_tip_hash)

        assert len(vbk_blocks) == vbk_blocks_amount
        assert len(node0_tip.containingVBKs) == vbk_blocks_amount
        assert node0_tip == self.nodes[0].getbestblock()

        node1_tip = self.nodes[1].getbestblock()

        assert node1_tip.hash != node0_tip.hash

        self.log.info("node 1 mine 10 blocks")
        node1_tip_hash = self.nodes[1].generate(nblocks=10)[9]
        node1_tip = self.nodes[1].getblock(node1_tip_hash)

        assert node1_tip == self.nodes[1].getbestblock()

        self.nodes[0].connect(self.nodes[1])
        self.log.info("connect node 1 and node 0")

        sync_all(self.nodes, timeout=30)
        self.log.info("nodes[0,1] are in sync")

        assert self.nodes[1].getbestblock() == self.nodes[0].getbestblock()

        # mine a block on node[1] with these vbk blocks
        tip_hash = self.nodes[1].generate(nblocks=1)[0]
        tip = self.nodes[1].getblock(tip_hash)

        assert len(vbk_blocks) == vbk_blocks_amount
        assert len(tip.containingVBKs) == vbk_blocks_amount

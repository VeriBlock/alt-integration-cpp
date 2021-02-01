"""
Start 2 nodes.
Mine 100 blocks on node0.
Disconnect node[1].
node[0] endorses block 100 (fork A tip).
node[0] mines pop tx in block 101 (fork A tip)

Pop is disabled before block 200 therefore can't handle Pop data
"""

from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import endorse_block, mine_until_pop_enabled, get_best_block, wait_for_block_height


class PopActivate(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        self.apm = MockMiner()
        self._cannot_endorse()
        self.nodes[0].restart()
        self._can_endorse()

    def _cannot_endorse(self):
        self.log.warning("starting _cannot_endorse()")

        # node0 start with 100 blocks
        tip_height = self.nodes[0].getblockcount()
        self.nodes[0].generate(nblocks=100)
        wait_for_block_height(self.nodes[0], 100)
        assert get_best_block(self.nodes[0]).height == tip_height + 100
        self.log.info("node0 mined 100 blocks")

        self.log.warning("_cannot_endorse() succeeded!")

    def _can_endorse(self):
        self.log.warning("starting _can_endorse()")
        self.nodes[0].connect(self.nodes[1])
        self.sync_all()
        self.nodes[1].disconnect(self.nodes[0])

        mine_until_pop_enabled(self.nodes[0])
        tip_height = self.nodes[0].getblockcount()

        # endorse block 200 (fork A tip)
        addr0 = self.nodes[0].getnewaddress()
        endorse_block(self.nodes[0], self.apm, tip_height, addr0)
        self.log.info("node0 endorsed block {} (fork A tip)".format(tip_height))
        # mine pop tx on node0
        self.nodes[0].generate(nblocks=1)
        tip = get_best_block(self.nodes[0])
        self.log.info("node0 tip is {}".format(tip.height))

        self.nodes[1].generate(nblocks=250)
        tip2 = get_best_block(self.nodes[1])
        self.log.info("node1 tip is {}".format(tip2.height))

        self.nodes[0].connect(self.nodes[1])
        self.sync_all()
        best_blocks = [get_best_block(node) for node in self.nodes]
        assert best_blocks[0].hash == best_blocks[1].hash
        self.log.info("all nodes switched to common block")

        for i in range(len(best_blocks)):
            assert best_blocks[i].height == tip.height, \
                "node[{}] expected to select shorter chain ({}) with higher pop score\n" \
                "but selected longer chain ({})".format(i, tip.height, best_blocks[i].height)

        self.log.info("all nodes selected fork A as best chain")
        self.log.warning("_can_endorse() succeeded!")

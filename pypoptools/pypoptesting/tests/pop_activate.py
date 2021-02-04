"""
Start 2 nodes.
Mine 100 blocks on node0.
Disconnect node[1].
node[0] endorses block 100 (fork A tip).
node[0] mines pop tx in block 101 (fork A tip)

Pop is disabled before block 200 therefore can't handle Pop data
"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.pop_util import endorse_block, mine_until_pop_enabled
from ..framework.sync_util import sync_all, wait_for_block_height


class PopActivate(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        assert self.nodes[0].getblockcount() == 0
        pop_activation_height = self.nodes[0].getpopparams().popActivationHeight
        assert pop_activation_height == 200

        # node0 start with 100 blocks
        self.nodes[0].generate(nblocks=100)
        wait_for_block_height(self.nodes[0], 100)
        assert self.nodes[0].getblockcount() == 100
        self.log.info("node0 mined 100 blocks")

        # endorse block 100 (fork A tip)
        self.log.info('Should not accept POP data before activation block height')
        try:
            endorse_block(self.nodes[0], apm, 100)
            assert False
        except:
            self.log.info("Endorse block failed as expected")
            pass

        self.nodes[0].restart()
        self.nodes[0].connect(self.nodes[1])
        sync_all(self.nodes)
        self.nodes[1].disconnect(self.nodes[0])

        mine_until_pop_enabled(self.nodes[0])
        tip_height = self.nodes[0].getblockcount()

        # endorse block 200 (fork A tip)
        endorse_block(self.nodes[0], apm, tip_height)
        self.log.info("node0 endorsed block {} (fork A tip)".format(tip_height))
        # mine pop tx on node0
        self.nodes[0].generate(nblocks=1)
        tip = self.nodes[0].getbestblock()
        self.log.info("node0 tip is {}".format(tip.height))

        self.nodes[1].generate(nblocks=250)
        tip2 = self.nodes[1].getbestblock()
        self.log.info("node1 tip is {}".format(tip2.height))

        self.nodes[0].connect(self.nodes[1])
        sync_all(self.nodes)
        best_blocks = [node.getbestblock() for node in self.nodes]
        assert best_blocks[0].hash == best_blocks[1].hash
        self.log.info("all nodes switched to common block")

        for i in range(len(best_blocks)):
            assert best_blocks[i].height == tip.height, \
                "node[{}] expected to select shorter chain ({}) with higher pop score\n" \
                "but selected longer chain ({})".format(i, tip.height, best_blocks[i].height)

        self.log.info("all nodes selected fork A as best chain")

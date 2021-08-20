"""
Start 4 nodes.
Stop node[3] at 200 blocks.
Mine 103 blocks on node0.
Disconnect node[2].
node[2] mines 97 blocks, total height is 400 (fork B)
node[0] mines 10 blocks, total height is 313 (fork A)
node[0] endorses block 313 (fork A tip).
node[0] mines pop tx in block 314 (fork A tip)
node[0] mines 9 more blocks
node[2] is connected to nodes[0,1]
node[3] started with 0 blocks.

After sync has been completed, expect all nodes to be on same height (fork A, block 323)
"""
from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.generate_util import generate_endorsed
from ...framework.pop_util import create_endorsed_chain, endorse_block, mine_until_pop_enabled
from ...framework.sync_util import (
    start_all, connect_all,
    sync_all, sync_blocks, sync_pop_tips,
    wait_for_block_height, disconnect_all
)


class PopForkResolutionTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 4

    def setup_nodes(self):
        start_all(self.nodes)
        connect_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        sync_all(self.nodes)
        self.keystoneInterval = self.nodes[0].getpopparams().keystoneInterval

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._shorter_endorsed_chain_wins(apm)
        self._4_chains_converge(apm)
        self._4_chains_converge_random(apm)

    def _shorter_endorsed_chain_wins(self, apm):
        self.log.info("starting _shorter_endorsed_chain_wins()")
        last_block = self.nodes[3].getblockcount()

        # stop node3
        self.nodes[3].stop()
        self.log.info("node3 stopped with block height %d", last_block)

        # all nodes start with last_block + 103 blocks
        self.nodes[0].generate(nblocks=103)
        self.log.info("node0 mined 103 blocks")
        sync_blocks([self.nodes[0], self.nodes[1], self.nodes[2]], timeout=60)
        assert self.nodes[0].getbestblock().height == last_block + 103
        assert self.nodes[1].getbestblock().height == last_block + 103
        assert self.nodes[2].getbestblock().height == last_block + 103
        self.log.info("nodes[0,1,2] synced are at block %d", last_block + 103)

        # node2 is disconnected from others
        self.nodes[2].disconnect(self.nodes[0])
        self.nodes[2].disconnect(self.nodes[1])
        self.log.info("node2 is disconnected")

        # node2 mines another 97 blocks, so total height is last_block + 200
        self.nodes[2].generate(nblocks=97)

        # fork A is at 303 (last_block = 200)
        # fork B is at 400
        wait_for_block_height(self.nodes[2], last_block + 200)
        self.log.info("node2 mined 97 more blocks, total height is %d", last_block + 200)

        best_blocks = [node.getbestblock() for node in self.nodes[0:3]]

        assert best_blocks[0] != best_blocks[2], "node[0,2] have same best hashes"
        assert best_blocks[0] == best_blocks[1], "node[0,1] have different best hashes: {} vs {}".format(best_blocks[0],
                                                                                                         best_blocks[1])

        # mine a keystone interval of blocks to fork A
        self.nodes[0].generate(nblocks=self.keystoneInterval)
        sync_all([self.nodes[0], self.nodes[1]])
        self.log.info("nodes[0,1] are in sync and are at fork A (%d...%d blocks)", last_block + 103, last_block + 103 + self.keystoneInterval)

        # fork B is at 400
        assert best_blocks[2].height == last_block + 200, "unexpected tip: {}".format(best_blocks[2])
        self.log.info("node2 is at fork B (%d...%d blocks)", last_block + 103, last_block + 200)

        assert 200 > 103 + self.keystoneInterval + 10, "keystone interval is set too high"

        # endorse block 303 + keystone interval (fork A tip)
        addr0 = self.nodes[0].getnewaddress()
        atv_id = endorse_block(self.nodes[0], apm, last_block + 103 + self.keystoneInterval, addr0)
        self.log.info("node0 endorsed block %d (fork A tip)", last_block + 103 + self.keystoneInterval)
        # mine pop tx on node0
        self.nodes[0].generate(nblocks=1)
        containing_block = self.nodes[0].getbestblock()
        self.nodes[0].generate(nblocks=9)
        self.log.info("node0 mines 10 more blocks")
        sync_all([self.nodes[0], self.nodes[1]])

        assert self.nodes[1].getblock(containing_block.hash).hash == containing_block.hash

        tip = self.nodes[0].getbestblock()
        assert atv_id in containing_block.containingATVs, "pop tx is not in containing block"
        sync_blocks([self.nodes[0], self.nodes[1]], timeout=60)
        self.log.info("nodes[0,1] are in sync, pop tx containing block is {}".format(containing_block.height))
        self.log.info("node0 tip is {}".format(tip.height))

        self.nodes[0].connect(self.nodes[2])
        self.nodes[1].connect(self.nodes[2])
        self.log.info("node2 connected to nodes[0,1]")

        self.nodes[3].start()
        self.nodes[3].connect(self.nodes[0])
        self.nodes[3].connect(self.nodes[2])
        self.log.info("node3 started with 0 blocks, connected to nodes[0,2]")

        sync_blocks(self.nodes, timeout=60)
        self.log.info("nodes[0,1,2,3] are in sync")

        # expected best block hash is fork A (has higher pop score)
        best_blocks = [node.getbestblock() for node in self.nodes]
        assert best_blocks[0].hash == best_blocks[1].hash
        assert best_blocks[0].hash == best_blocks[2].hash
        assert best_blocks[0].hash == best_blocks[3].hash
        self.log.info("all nodes switched to common block")

        for i in range(len(best_blocks)):
            assert best_blocks[i].height == tip.height, \
                "node[{}] expected to select shorter chain ({}) with higher pop score\n" \
                "but selected longer chain ({})".format(i, tip.height, best_blocks[i].height)

        self.log.info("_shorter_endorsed_chain_wins() succeeded!")

    def _4_chains_converge(self, apm):
        self.log.info("_4_chains_converge() started!")

        sync_blocks(self.nodes, timeout=5)
        self.log.info("nodes[0,1,2,3] are in sync")

        # disconnect all nodes
        disconnect_all(self.nodes)

        self.log.info("all nodes disconnected")
        last_block = self.nodes[3].getblockcount()
        print("last_block: ", last_block)

        # node[i] creates endorsed chain
        to_mine = 15
        for i, node in enumerate(self.nodes):
            self.log.info("node[{}] started to create endorsed chain of {} blocks".format(i, to_mine))
            addr = node.getnewaddress()
            create_endorsed_chain(node, apm, to_mine, addr)

        # all nodes have different tips at height 303 + keystone interval
        best_blocks = [node.getbestblock() for node in self.nodes]
        for b in best_blocks:
            assert b.height == last_block + to_mine
        assert len(set([block.hash for block in best_blocks])) == len(best_blocks)
        self.log.info("all nodes have different tips")

        # connect all nodes to each other
        connect_all(self.nodes)
        self.log.info("all nodes connected")

        # vGeth fix
        self.nodes[0].generate(1)
        self.log.info("node[0] mines 1 block")

        sync_blocks(self.nodes, timeout=60)
        sync_pop_tips(self.nodes, timeout=60)
        self.log.info("all nodes have common tip")

        self.log.info("_4_chains_converge() succeeded!")

    def _4_chains_converge_random(self, apm):
        self.log.info("_4_chains_random_converge() started!")

        # disconnect all nodes
        disconnect_all(self.nodes)

        self.log.info("all nodes disconnected")
        last_block = self.nodes[3].getblockcount()

        # node[i] creates endorsed chain
        for i, node in enumerate(self.nodes):
            self.log.info("node[{}] started to generate random endorsed chain".format(i))
            generate_endorsed(node, apm, atvs=20, vtbs=20, alt_blocks=20, vbk_blocks=20, btc_blocks=20, seed=12345)

        # all nodes have different tips at height 323
        best_blocks = [node.getbestblock() for node in self.nodes]
        for b in best_blocks:
            assert b.height == last_block + 20
        assert len(set([block.hash for block in best_blocks])) == len(best_blocks)
        self.log.info("all nodes have different tips")

        # connect all nodes to each other
        connect_all(self.nodes)

        # node[0] will mine a little more, so that all chains will converge into A
        self.nodes[0].generate(nblocks=50)

        self.log.info("all nodes connected")
        sync_blocks(self.nodes, timeout=60)
        sync_pop_tips(self.nodes, timeout=60)
        self.log.info("all nodes have common tip")

        best_blocks = [node.getbestblock() for node in self.nodes]
        expected_best = best_blocks[0]
        for best in best_blocks:
            assert best == expected_best, "all={}, expected={}".format([x.short_str() for x in best_blocks],
                                                                       expected_best.short_str())

        self.log.info("_4_chains_random_converge() succeeded!")

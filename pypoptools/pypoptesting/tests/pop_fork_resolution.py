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
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.pop_util import create_endorsed_chain, endorse_block, mine_until_pop_enabled
from ..framework.sync_util import (
    start_all_and_wait, connect_all, sync_all, sync_blocks, sync_pop_tips,
    wait_for_block_height, wait_for_rpc_availability
)


class PopForkResolution(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 4

    def setup_nodes(self):
        start_all_and_wait(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        connect_all(self.nodes)
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._shorter_endorsed_chain_wins(apm)
        self._4_chains_converge(apm)

    def _shorter_endorsed_chain_wins(self, apm):
        self.log.warning("starting _shorter_endorsed_chain_wins()")
        last_block = self.nodes[3].getblockcount()

        # stop node3
        self.nodes[3].stop()
        self.log.info("node3 stopped with block height %d", last_block)

        # all nodes start with last_block + 103 blocks
        self.nodes[0].generate(nblocks=103)
        self.log.info("node0 mined 103 blocks")
        sync_blocks(self.nodes[0:2], timeout=20)
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

        # mine 10 more blocks to fork A
        self.nodes[0].generate(nblocks=10)
        sync_all(self.nodes[0:2])
        self.log.info("nodes[0,1] are in sync and are at fork A (%d...%d blocks)", last_block + 103, last_block + 113)

        # fork B is at 400
        assert best_blocks[2].height == last_block + 200, "unexpected tip: {}".format(best_blocks[2])
        self.log.info("node2 is at fork B (%d...%d blocks)", last_block + 103, last_block + 200)

        # endorse block 313 (fork A tip)
        addr0 = self.nodes[0].getnewaddress()
        atv_id = endorse_block(self.nodes[0], apm, last_block + 113, addr0)
        self.log.info("node0 endorsed block %d (fork A tip)", last_block + 113)
        # mine pop tx on node0
        block_hashes = self.nodes[0].generate(nblocks=10)
        self.log.info("node0 mines 10 more blocks")
        sync_all(self.nodes[0:2])
        containing_block = self.nodes[0].getblock(block_hashes[0])

        assert self.nodes[1].getblock(block_hashes[0]).hash == containing_block.hash

        tip = self.nodes[0].getbestblock()
        assert atv_id in containing_block.containingATVs, "pop tx is not in containing block"
        sync_blocks(self.nodes[0:2])
        self.log.info("nodes[0,1] are in sync, pop tx containing block is {}".format(containing_block.height))
        self.log.info("node0 tip is {}".format(tip.height))

        self.nodes[0].connect(self.nodes[2])
        self.nodes[1].connect(self.nodes[2])
        self.log.info("node2 connected to nodes[0,1]")

        self.nodes[3].start()
        wait_for_rpc_availability(self.nodes[3])
        self.nodes[3].connect(self.nodes[0])
        self.nodes[3].connect(self.nodes[2])
        self.log.info("node3 started with 0 blocks, connected to nodes[0,2]")

        sync_blocks(self.nodes, timeout=30)
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

        self.log.warning("_shorter_endorsed_chain_wins() succeeded!")

    def _4_chains_converge(self, apm):
        self.log.warning("_4_chains_converge() started!")

        # disconnect all nodes
        for i in range(self.num_nodes):
            for node in self.nodes:
                node.disconnect(self.nodes[i])

        self.log.info("all nodes disconnected")
        last_block = self.nodes[3].getblockcount()

        # node[i] creates endorsed chain
        to_mine = 15
        for i, node in enumerate(self.nodes):
            self.log.info("node[{}] started to create endorsed chain of {} blocks".format(i, to_mine))
            addr = node.getnewaddress()
            create_endorsed_chain(node, apm, to_mine, addr)

        # all nodes have different tips at height 323
        best_blocks = [node.getbestblock() for node in self.nodes]
        for b in best_blocks:
            assert b.height == last_block + to_mine
        assert len(set([block.hash for block in best_blocks])) == len(best_blocks)
        self.log.info("all nodes have different tips")

        # connect all nodes to each other
        for i in range(self.num_nodes):
            for node in self.nodes:
                node.connect(self.nodes[i])

        self.log.info("all nodes connected")
        sync_blocks(self.nodes, timeout=60)
        sync_pop_tips(self.nodes, timeout=60)
        self.log.info("all nodes have common tip")

        expected_best = best_blocks[0]
        best_blocks = [node.getbestblock() for node in self.nodes]
        for best in best_blocks:
            assert best == expected_best

        self.log.warning("_4_chains_converge() succeeded!")

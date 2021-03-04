"""
Start 3 nodes. Node0 has no -txindex, Nodes[1,2] have -txindex
Create a chain of 20 blocks, where every next block contains 1 pop tx that endorses previous block.
Restart nodes[0,1] without -reindex.
Node[2] is a control node.

Expect that BTC/VBK tree state on nodes[0,1] is same as before shutdown (test against control node).
"""

import time

from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.pop_util import create_endorsed_chain, mine_until_pop_enabled
from ...framework.sync_util import start_all, connect_all, sync_all, sync_blocks


class PopInitTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def setup_nodes(self):
        start_all(nodes=self.nodes)
        mine_until_pop_enabled(node=self.nodes[0])
        connect_all(nodes=self.nodes)
        sync_all(nodes=self.nodes, timeout=300)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()
        size = 20

        # 100 blocks without endorsements
        self.nodes[0].generate(nblocks=100)
        self.log.info("node0 started mining of {} endorsed blocks".format(size))
        create_endorsed_chain(self.nodes[0], apm, size)
        self.log.info("node0 finished creation of {} endorsed blocks".format(size))

        sync_blocks(self.nodes)
        self.log.info("nodes are in sync")

        # stop node0
        self.nodes[0].restart()
        self.nodes[1].restart()
        self.log.info("nodes[0,1] restarted")
        sync_all(self.nodes, timeout=30)
        self.log.info("nodes are in sync")

        best_blocks = [node.getbestblock() for node in self.nodes]
        pop_data = [node.getpopdatabyheight(best_blocks[0].height) for node in self.nodes]

        # when node0 stops, its VBK/BTC trees get cleared. When we start it again, it MUST load payloads into trees.
        # if this assert fails, it means that node restarted, but NOT loaded its VBK/BTC state into memory.
        # node[2] is a control node that has never been shut down.
        assert pop_data[0] == pop_data[2]
        assert pop_data[1] == pop_data[2]

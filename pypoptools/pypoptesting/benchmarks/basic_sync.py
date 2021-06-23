import time

from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.sync_util import (
    start_all, connect_all,
    sync_all, disconnect_all,
    wait_for_block_height
)


class NodeBasicSyncBenchmark(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all(self.nodes)

    def run_test(self):
        # disconnect all nodes
        disconnect_all(self.nodes)

        start = time.time()
        self.log.info("node[0] mine 1000 blocks")
        self.nodes[0].generate(nblocks=1000)
        wait_for_block_height(self.nodes[0], 1000)

        self.log.info("node[0] mine 2000 blocks")
        self.nodes[0].generate(nblocks=1000)
        wait_for_block_height(self.nodes[0], 2000)

        self.log.info("node[0] mine 3000 blocks")
        self.nodes[0].generate(nblocks=1000)
        wait_for_block_height(self.nodes[0], 3000)

        node_0_tip = self.nodes[0].getbestblock()
        node_1_tip = self.nodes[1].getbestblock()

        assert node_0_tip != node_1_tip

        self.log.info("restart all nodes")
        for node in self.nodes:
            node.restart()

        assert node_0_tip == self.nodes[0].getbestblock()
        assert node_1_tip == self.nodes[1].getbestblock()

        self.log.info("connect all nodes")
        connect_all(self.nodes)
        sync_all(self.nodes, timeout=300)

        assert self.nodes[0].getbestblock() == self.nodes[1].getbestblock()

        elapsed = time.time() - start

        print('Basic node sync benchmark finished in {:.3f} sec'.format(elapsed))

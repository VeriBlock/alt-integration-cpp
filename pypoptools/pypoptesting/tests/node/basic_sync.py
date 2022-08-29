from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.sync_util import (
    start_all, connect_all,
    sync_all, disconnect_all
)


class NodeBasicSyncTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def setup_nodes(self):
        start_all(self.nodes)

    def run_test(self):
        # disconnect all nodes
        disconnect_all(self.nodes)

        self.log.info("node[0] mine 200 blocks")
        self.nodes[0].generate(nblocks=200)
        self.log.info("node[1] mine 130 blocks")
        self.nodes[1].generate(nblocks=130)
        self.log.info("node[2] mine 300 blocks")
        self.nodes[2].generate(nblocks=300)

        node_0_tip = self.nodes[0].getbestblock()
        node_1_tip = self.nodes[1].getbestblock()
        node_2_tip = self.nodes[2].getbestblock()

        assert node_0_tip != node_1_tip
        assert node_1_tip != node_2_tip
        assert node_2_tip != node_0_tip

        self.log.info("restart all nodes")
        for node in self.nodes:
            node.restart()

        assert node_0_tip == self.nodes[0].getbestblock()
        assert node_1_tip == self.nodes[1].getbestblock()
        assert node_2_tip == self.nodes[2].getbestblock()

        self.log.info("connect all nodes")
        connect_all(self.nodes)
        sync_all(self.nodes, timeout=300)

        assert self.nodes[0].getbestblock() == self.nodes[1].getbestblock()
        assert self.nodes[1].getbestblock() == self.nodes[2].getbestblock()
        assert self.nodes[2].getbestblock() == self.nodes[0].getbestblock()

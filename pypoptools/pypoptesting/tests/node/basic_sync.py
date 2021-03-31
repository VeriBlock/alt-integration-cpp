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

        self.log.info("node[0] mine 1000 blocks")
        self.nodes[0].generate(nblocks=1000)
        self.log.info("node[1] mine 650 blocks")
        self.nodes[1].generate(nblocks=650)
        self.log.info("node[2] mine 1500 blocks")
        self.nodes[2].generate(nblocks=1500)

        assert self.nodes[0].getbestblock() != self.nodes[1].getbestblock()
        assert self.nodes[1].getbestblock() != self.nodes[2].getbestblock()
        assert self.nodes[2].getbestblock() != self.nodes[0].getbestblock()

        self.log.info("connect all nodes")
        connect_all(self.nodes)
        sync_all(self.nodes)





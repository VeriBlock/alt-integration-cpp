from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.pop_util import mine_until_pop_enabled
from ...framework.sync_util import (
    start_all, connect_all,
    sync_all, sync_blocks, sync_pop_tips,
    wait_for_block_height, disconnect_all
)

class PopStressTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 50

    def setup_nodes(self):
        start_all(self.nodes)
        # connect_all(self.nodes)
        for i in range(len(self.nodes) - 1):
            print("Connecting node: ", i+1)
            self.nodes[i+1].connect(self.nodes[0])
            print("Node connected: ", i+1)
        mine_until_pop_enabled(self.nodes[0])
        sync_all(self.nodes)

    def run_test(self):
        self.nodes[0].generate(nblocks=100)
        sync_all(self.nodes)
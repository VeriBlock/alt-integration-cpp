from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.sync_util import connect_all, sync_all, wait_for_block_height


class RestartAndSync(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def run_test(self):
        for node in self.nodes:
            assert node.getblockcount() == 0

        self.nodes[0].generate(nblocks=100)
        wait_for_block_height(self.nodes[0], 100)

        assert self.nodes[0].getblockcount() == 100

        self.nodes[0].restart()

        assert self.nodes[0].getblockcount() == 100

        connect_all(self.nodes)
        sync_all(self.nodes)

        assert self.nodes[1].getblockcount() == 100


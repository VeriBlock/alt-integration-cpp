from pypoptesting.framework.test_framework import PopIntegrationTestFramework
from pypoptesting.framework.sync_util import wait_for_block_height


class RpcGenerateTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        assert self.nodes[0].getblockcount() == 0

        self.nodes[0].generate(nblocks=100)
        wait_for_block_height(self.nodes[0], 100)

        assert self.nodes[0].getblockcount() == 100

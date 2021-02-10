from pypoptesting.framework.test_framework import PopIntegrationTestFramework
from pypoptesting.framework.sync_util import connect_all, sync_all, wait_for_block_height


class RpcConnectTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def run_test(self):
        for node in self.nodes:
            assert node.getblockcount() == 0

        self.nodes[0].generate(nblocks=100)
        wait_for_block_height(self.nodes[0], 100)

        for i, node in enumerate(self.nodes):
            if i == 0:
                assert node.getblockcount() == 100
            else:
                assert node.getblockcount() == 0

        connect_all(self.nodes)
        sync_all(self.nodes)

        for node in self.nodes:
            assert node.getblockcount() == 100

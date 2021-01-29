"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import endorse_block, wait_for_block_height, KEYSTONE_INTERVAL


class PopSync(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        addr0 = self.nodes[0].getnewaddress()
        addr1 = self.nodes[1].getnewaddress()
        addr2 = self.nodes[2].getnewaddress()

        for height in range(self.nodes[0].getblockcount(), 52):
            self.nodes[0].generate(nblocks=1)
            # endorse every block
            wait_for_block_height(self.nodes[2], height)
            self.log.info("node2 endorsing block {} by miner {}".format(height, addr2))
            node2_atv_id = endorse_block(self.nodes[2], apm, height, addr2)

            # endorse each keystone
            if height % KEYSTONE_INTERVAL == 0:
                wait_for_block_height(self.nodes[0], height)
                self.log.info("node0 endorsing block {} by miner {}".format(height, addr0))
                node0_atv_id = endorse_block(self.nodes[0], apm, height, addr0)

                wait_for_block_height(self.nodes[1], height)
                self.log.info("node1 endorsing block {} by miner {}".format(height, addr1))
                node1_atv_id = endorse_block(self.nodes[1], apm, height, addr1)

                # wait until node[1] gets relayed pop tx
                self.sync_all(self.nodes, timeout=20)
                self.log.info("transactions relayed")

                # mine a block on node[1] with this pop tx
                containing_block_hash = self.nodes[1].generate(nblocks=1)[0]
                containing_block = self.nodes[1].getblock(containing_block_hash)
                self.log.info("node1 mined containing block={}".format(containing_block.hash))
                wait_for_block_height(self.nodes[0], containing_block.height)
                wait_for_block_height(self.nodes[2], containing_block.height)
                self.log.info("node0 and node2 got containing block over p2p")

                # assert that all atv_ids exist in this block
                for node in self.nodes:
                    block = node.getblock(containing_block_hash)
                    for atv_id in [node0_atv_id, node1_atv_id, node2_atv_id]:
                        assert atv_id in block.containingATVs, \
                            "containing block {} does not contain ATV {}".format(containing_block_hash, atv_id)

                # assert that node height matches
                assert self.nodes[0].getblockcount() == self.nodes[1].getblockcount() == self.nodes[2].getblockcount()

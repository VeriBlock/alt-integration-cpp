"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""
from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.pop_util import endorse_block, mine_until_pop_enabled
from ...framework.sync_util import start_all, connect_all, sync_all, wait_for_block_height


class PopSyncTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 3

    def setup_nodes(self):
        start_all(self.nodes)
        connect_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        sync_all(self.nodes)

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        keystone_interval = self.nodes[0].getpopparams().keystoneInterval

        for height in range(self.nodes[0].getblockcount(), 52):
            self.nodes[0].generate(nblocks=1)
            # endorse every block
            wait_for_block_height(self.nodes[2], height)
            self.log.info("node2 endorsing block {}".format(height))
            node2_atv_id = endorse_block(self.nodes[2], apm, height)

            # endorse each keystone
            if height % keystone_interval == 0:
                wait_for_block_height(self.nodes[0], height)
                self.log.info("node0 endorsing block {}".format(height))
                node0_atv_id = endorse_block(self.nodes[0], apm, height)

                wait_for_block_height(self.nodes[1], height)
                self.log.info("node1 endorsing block {}".format(height))
                node1_atv_id = endorse_block(self.nodes[1], apm, height)

                # wait until node[1] gets relayed pop tx
                sync_all(self.nodes, timeout=20)
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
                assert self.nodes[0].getblockcount() == self.nodes[1].getblockcount()
                assert self.nodes[0].getblockcount() == self.nodes[2].getblockcount()

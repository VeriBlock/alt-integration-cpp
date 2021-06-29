import time

from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.pop_util import endorse_block, mine_until_pop_enabled, mine_alt_block
from ..framework.sync_util import (
    start_all, connect_all,
    sync_all, disconnect_all,
    wait_for_block_height
)

class NodeBasicSyncBenchmark(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.to_mine = 5000
        self.vtbs_in_payload = 10
        self.atvs_in_payload = 10
        self.vbks_in_payload = 10
        self.payloads_to_mine = 50

        # we want to endorse previous blocks to generate ATVs
        # make sure we have enough blocks and POP enabled
        assert self.to_mine >= self.atvs_in_payload + 5

    def setup_nodes(self):
        start_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        start = time.time()

        # disconnect all nodes
        disconnect_all(self.nodes)

        self.log.info("node[0] mine {} blocks".format(self.to_mine))
        for i in range(self.to_mine):
            self.log.info("node[0] mine block #{}".format(i + 1))
            mine_alt_block(self.nodes[0], nblocks=1)

        assert self.nodes[0].getblockcount() >= self.to_mine

        elapsed1 = time.time() - start
        print("{} blocks mined in {:.3f} sec".format(self.to_mine, elapsed1))

        for e in range(self.payloads_to_mine):
            self.log.info("generate vbk blocks on node0, amount {}".format(self.vbks_in_payload))
            vbk_blocks = []
            for i in range(self.vbks_in_payload):
                vbk_blocks.append(apm.mineVbkBlocks(1))
            assert len(vbk_blocks) == self.vbks_in_payload

            self.log.info("generate vtbs on node0, amount {}".format(self.vtbs_in_payload))
            for i in range(self.vtbs_in_payload):
                apm.endorseVbkBlock(apm.vbkTip, apm.btcTip.getHash())

            last_block = self.nodes[0].getblockcount()
            assert last_block >= self.atvs_in_payload + 5

            self.log.info("generate atvs on node0, amount {}".format(self.atvs_in_payload))
            for i in range(self.atvs_in_payload):
                to_endorse = last_block - self.atvs_in_payload - 5 + i
                endorse_block(self.nodes[0], apm, to_endorse)

            self.nodes[0].generate(nblocks=1)
            containing_block = self.nodes[0].getbestblock()

            assert len(containing_block.containingATVs) == self.atvs_in_payload
            assert len(containing_block.containingVTBs) == self.vtbs_in_payload
            assert len(containing_block.containingVBKs) == self.vbks_in_payload + self.vtbs_in_payload + self.atvs_in_payload

        node_0_tip = self.nodes[0].getbestblock()
        node_1_tip = self.nodes[1].getbestblock()

        assert node_0_tip != node_1_tip

        elapsed2 = time.time() - start - elapsed1
        print("{} payloads generated in {:.3f} sec".format(self.payloads_to_mine, elapsed2))

        self.log.info("restart all nodes")
        for node in self.nodes:
            node.restart()

        assert node_0_tip == self.nodes[0].getbestblock()
        assert node_1_tip == self.nodes[1].getbestblock()

        self.log.info("connect all nodes")
        connect_all(self.nodes)
        sync_all(self.nodes, timeout=300)

        assert self.nodes[0].getbestblock() == self.nodes[1].getbestblock()

        elapsed3 = time.time() - start - elapsed2 - elapsed1
        print("{} nodes synced in {:.3f} sec".format(self.num_nodes, elapsed3))

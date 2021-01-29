"""
Test with multiple nodes, and multiple PoP endorsements, checking to make sure nodes stay in sync.
"""

from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import NETWORK_ID

import time


class PopE2E(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypopminer import MockMiner, PublicationData
        apm = MockMiner()

        self._assert_nodes_peer_info()

        vbk_blocks_amount = 100
        self.log.info("generate vbk blocks on node0, amount {}".format(vbk_blocks_amount))
        vbk_blocks = []
        for i in range(vbk_blocks_amount):
            vbk_blocks.append(apm.mineVbkBlocks(1))

        assert len(vbk_blocks) == vbk_blocks_amount

        vtbs_amount = 20
        self.log.info("generate vtbs on node0, amount {}".format(vtbs_amount))
        for i in range(vtbs_amount):
            apm.endorseVbkBlock(apm.vbkTip, apm.btcTip.getHash(), 1)

        self.nodes[0].generate(nblocks=10)
        last_block = self.nodes[0].getblockcount()

        assert last_block >= 5
        self.log.info("endorse {} alt block".format(last_block - 5))
        pop_data = self.nodes[0].getpopdatabyheight(last_block - 5)
        p = PublicationData()
        p.identifier = NETWORK_ID
        p.header = pop_data.header
        p.contextInfo = pop_data.authenticated_context
        p.payoutInfo = "0014aaddff"

        pop_data = apm.endorseAltBlock(p, vbk_blocks[0].getHash())
        assert len(pop_data.vtbs) == vtbs_amount

        [self.nodes[0].submitpopvbk(b.toVbkEncodingHex()) for b in vbk_blocks]
        [self.nodes[0].submitpopvtb(b.toVbkEncodingHex()) for b in pop_data.vtbs]
        self.nodes[0].submitpopatv(pop_data.atv.toVbkEncodingHex())

        self._assert_nodes_peer_info()

        containing_block_hash = self.nodes[0].generate(nblocks=1)[0]
        containing_block = self.nodes[0].getblock(containing_block_hash)

        assert len(containing_block.containingVTBs) == vtbs_amount
        assert len(containing_block.containingVBKs) == vbk_blocks_amount + vtbs_amount + 1

        assert last_block >= 6
        self.log.info("endorse {} alt block".format(last_block - 6))
        p = PublicationData()
        p.identifier = NETWORK_ID
        p.header = self.nodes[0].getpopdatabyheight(last_block - 6).header
        p.payoutInfo = "0014aaddff"

        pop_data = apm.endorseAltBlock(p, vbk_blocks[0].getHash())
        assert len(pop_data.vtbs) == vtbs_amount

        [self.nodes[0].submitpopvbk(b.toVbkEncodingHex()) for b in vbk_blocks]
        [self.nodes[0].submitpopvtb(b.toVbkEncodingHex()) for b in pop_data.vtbs]
        self.nodes[0].submitpopatv(pop_data.atv.toVbkEncodingHex())

        self._assert_nodes_peer_info()

        self.nodes[0].generate(nblocks=1)
        time.sleep(5)

        self.log.info("sync all nodes")
        self.sync_all(self.nodes)

    def _assert_nodes_peer_info(self):
        self._assert_node_peer_info(self.nodes[0])
        self._assert_node_peer_info(self.nodes[1])

    def _assert_node_peer_info(self, node):
        peer_info = node.getpeerinfo()
        assert len(peer_info) == 1
        assert peer_info[0]['banscore'] == 0

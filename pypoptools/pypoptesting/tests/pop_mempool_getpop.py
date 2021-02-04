"""
Feature POP popdata max size test

"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.pop_util import endorse_block, mine_vbk_blocks, mine_until_pop_enabled
from ..framework.sync_util import start_all, connect_all, sync_all


class PopMempoolGetpop(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def setup_nodes(self):
        start_all(self.nodes)
        mine_until_pop_enabled(self.nodes[0])
        connect_all(self.nodes)
        sync_all(self.nodes)

    def run_test(self):
        self.nodes[0].generate(nblocks=10)
        sync_all(self.nodes)

        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        self._test_case_vbk(apm, 113)
        self._test_case_vbk(apm, 13)
        self._test_case_vbk(apm, 75)

        self._test_case_atv(apm, 42)
        self._test_case_atv(apm, 135)
        self._test_case_atv(apm, 25)

    def _test_case_vbk(self, apm, payloads_amount):
        self.log.info("running _test_case_vbk()")

        vbk_blocks = mine_vbk_blocks(self.nodes[0], apm, payloads_amount)

        # mine a block on node[1] with this pop tx
        containing_block_hash = self.nodes[0].generate(nblocks=1)[0]
        containing_block = self.nodes[0].getblock(containing_block_hash)

        assert len(vbk_blocks) == payloads_amount
        assert len(containing_block.containingVBKs) == payloads_amount

        self.log.info("success! _test_case_vbk()")

    def _test_case_atv(self, apm, payloads_amount):
        self.log.info("running _test_case_vbk()")

        # endorse block last_block - 5
        last_block = self.nodes[0].getblockcount()
        assert last_block >= 5
        for i in range(payloads_amount):
            self.log.info("endorsing block {} on node0".format(last_block - 5))
            endorse_block(self.nodes[0], apm, last_block - 5)

        # mine a block on node[1] with this pop tx
        containing_block_hash = self.nodes[0].generate(nblocks=1)[0]
        containing_block = self.nodes[0].getblock(containing_block_hash)

        assert len(containing_block.containingATVs) == payloads_amount

        self.log.info("success! _test_case_atv()")

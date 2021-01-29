"""
Feature POP Payout test

Consists of multiple test cases.

Case1:
node0 endorses block 5, node1 confirms it and mines blocks until reward for this block is paid.
node0 mines 100 blocks and checks balance.
Expected balance is POW_PAYOUT * 10 + pop payout. (node0 has only 10 mature coinbases)

"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import endorse_block, POW_PAYOUT, POP_PAYOUT_DELAY


class PopPayouts(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypopminer import MockMiner
        apm = MockMiner()

        last_block = self.nodes[0].getblockcount()
        self.nodes[0].generate(nblocks=10)
        self.nodes[0].waitforblockheight(last_block + 10)
        last_block = self.nodes[0].getblockcount()
        self.log.info("node0 mined 10 more blocks")

        # endorse block 5
        assert last_block >= 5
        addr = self.nodes[0].getnewaddress()
        endorsed_height = last_block - 5
        self.log.info("endorsing block {} on node0 by miner {}".format(endorsed_height, addr))
        atv_id = endorse_block(self.nodes[0], apm, endorsed_height, addr)

        # wait until node[1] gets relayed pop tx
        self.sync_pop_mempools(self.nodes)
        self.log.info("node1 got relayed transaction")

        # mine a block on node[1] with this pop tx
        containing_block_hash = self.nodes[1].generate(nblocks=1)[0]
        containing_block = self.nodes[1].getblock(containing_block_hash)
        self.log.info("node1 mined containing block={}".format(containing_block.hash))
        self.nodes[0].waitforblockheight(containing_block.height)
        self.log.info("node0 got containing block over p2p")

        # assert that txid exists in this block
        block = self.nodes[0].getblock(containing_block_hash)

        assert atv_id in block.containingATVs

        # target height is last_block - 5 + POP_PAYOUT_DELAY
        target_payout_block = endorsed_height + POP_PAYOUT_DELAY
        n = target_payout_block - block.height
        self.log.info("endorsed block height {}, expected payout block height {}".format(
            endorsed_height,
            target_payout_block
        ))
        payout_block_hash = self.nodes[1].generate(nblocks=n)[-1]
        self.sync_blocks(self.nodes)
        self.log.info("pop rewards paid")

        # check that expected block pays for this endorsement
        block = self.nodes[0].getblock(payout_block_hash)
        coinbase_tx_hash = block['tx'][0]
        coinbase_tx = self.nodes[0].getrawtransaction(coinbase_tx_hash, 1)
        outputs = coinbase_tx['vout']
        assert len(outputs) > 2, "block with payout does not contain pop payout: {}".format(outputs)
        assert outputs[1]['n'] == 1
        assert outputs[1]['value'] > 0, "expected non-zero output at n=1, got: {}".format(outputs[1])

        # mine 100 blocks and check balance
        self.nodes[0].generate(nblocks=100)
        balance = self.nodes[0].getbalance()

        # node[0] has 210 (last_block) mature coinbases and a single pop payout
        assert last_block == 210, "calculation below are only valid for POP activation height = 210"
        pop_payout = float(outputs[1]['value'])
        half_payout = POW_PAYOUT / 2
        assert balance == POW_PAYOUT * 149 + half_payout * 50 + half_payout * .6 * 11 + pop_payout
        self.log.warning("success! _case1_endorse_keystone_get_paid()")

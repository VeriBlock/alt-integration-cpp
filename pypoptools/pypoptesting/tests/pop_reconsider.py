"""
Start 1 node.
Mine 100 blocks.
Endorse block 100, mine pop tx in 101.
Mine 19 blocks.
Invalidate block 101.
Ensure tip is block 100 and is not endorsed.
Mine 200 blocks.
Ensure tip is block 300.
Reconsider block 101.
Ensure shorter fork wins and new tip is block 120.
"""
from ..framework.test_framework import PopIntegrationTestFramework
from ..framework.util import endorse_block, wait_until, get_best_block


class PopReconsider(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def skip_test_if_missing_module(self):
        self.skip_if_no_pypopminer()

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()
        
        last_block = self.nodes[0].getblockcount()

        # start with last_block + 100 blocks
        chain_a_tip_hash = self.nodes[0].generate(nblocks=100)[-1]
        self.log.info("node mined 100 blocks")

        assert chain_a_tip_hash == self.nodes[0].getbestblockhash()

        # endorse block 300 (fork A tip)
        addr0 = self.nodes[0].getnewaddress()
        atv_id = endorse_block(self.nodes[0], apm, last_block + 100, addr0)
        self.log.info("node endorsed block %d (fork A tip)", last_block + 100)
        # mine pop tx on node0
        chain_a_tip_hash = self.nodes[0].generate(nblocks=1)[-1]
        assert chain_a_tip_hash == self.nodes[0].getbestblockhash()

        containing_block = self.nodes[0].getblock(chain_a_tip_hash)

        tip = get_best_block(self.nodes[0])
        assert atv_id in containing_block.containingATVs, "pop tx is not in containing block"
        self.log.info("node tip is {}".format(tip.height))

        chain_a_tip_hash = self.nodes[0].generate(nblocks=19)[-1]
        chain_a_tip_height = self.nodes[0].getblock(chain_a_tip_hash).height
        fork_height = last_block + 50
        fork_hash = self.nodes[0].getblockhash(fork_height)

        self.log.info("tip={}:{}, fork={}:{}".format(chain_a_tip_height, chain_a_tip_hash, fork_height, fork_hash))

        invalidated_height = fork_height + 1
        invalidated = self.nodes[0].getblockhash(invalidated_height)
        self.log.info("invalidating block next to fork block {}:{}".format(invalidated_height, invalidated))
        self.nodes[0].invalidateblock(invalidated)

        tip = get_best_block(self.nodes[0])
        assert tip.height == fork_height
        assert tip.hash == fork_hash

        tip = self.nodes[0].getblock(tip.hash)
        assert len(tip.endorsedBy) == 0, "block should not be endorsed after invalidation"

        # rewrite invalid block with generate
        # otherwise next block will be a duplicate
        chain_b_tip_hash = self.nodes[0].generate(nblocks=1)[-1]
        assert chain_b_tip_hash == self.nodes[0].getbestblockhash()

        # start with last_block + 1 + 199 blocks
        chain_b_tip_hash = self.nodes[0].generate(nblocks=199)[-1]
        self.log.info("node mined 199 blocks")

        tip = get_best_block(self.nodes[0])
        self.log.info("node tip is {}".format(tip.height))
        assert tip.hash == self.nodes[0].getbestblockhash()

        # Reconsider invalidated block
        self.log.info("reconsider block {}:{}".format(invalidated_height, invalidated))
        self.nodes[0].reconsiderblock(invalidated)

        wait_until(
            predicate=lambda: chain_a_tip_hash == self.nodes[0].getbestblockhash(),
            timeout=10
        )

import logging

from random import Random

from rpc.pop_miner import PopMiner

logger = logging.getLogger()


class RandomMiner:

    def __init__(self, node, apm, seed):
        self.miner = PopMiner(node, apm)
        self.random = Random(seed)
        self.btc_txs = []
        self.btc_tx_vbk_block = {}
        self.vbk_pop_txs = []
        self.vtbs = []
        self.vbk_block_vtbs = {}
        self.vbk_blocks = []
        self.vbk_txs = []
        self.atvs = []

    def execute_command(self):
        self.random_run([
            self.mine_btc_block,
            self.mine_vbk_block,
            self.mine_alt_block
        ])

    def mine_btc_block(self):
        logger.info('Executing mine BTC block')
        btc_tip = self.pick_btc_block()
        btc_txs = self.random_pop(self.btc_txs)
        btc_block = self.miner.mine_btc_block(btc_tip, btc_txs)

        for btc_tx in btc_txs:
            vbk_block = self.btc_tx_vbk_block[btc_tx]
            vbk_pop_tx = self.miner.create_vbk_pop_tx(vbk_block, btc_tx, btc_block)
            self.vbk_pop_txs.append(vbk_pop_tx)

    def mine_vbk_block(self):
        logger.info('Executing mine VBK block')
        vbk_tip = self.pick_vbk_block()
        vbk_pop_txs = self.random_pop(self.vbk_pop_txs)
        vbk_txs = self.random_pop(self.vbk_txs)
        vbk_block, vtbs, atvs = self.miner.mine_vbk_block(vbk_tip, vbk_pop_txs + vbk_txs)
        self.vbk_blocks.append(vbk_block)
        self.vtbs.extend(vtbs)
        self.vbk_block_vtbs[vbk_block] = vtbs
        self.atvs.extend(atvs)

        btc_tx = self.miner.create_btc_tx(vbk_block)
        self.btc_txs.append(btc_tx)
        self.btc_tx_vbk_block[btc_tx] = vbk_block

    def mine_alt_block(self):
        logger.info('Executing mine ALT block')
        alt_tip = self.pick_alt_block()
        vbk_blocks = self.random_pop(self.vbk_blocks)
        vtbs = self.random_pop(self.vtbs)
        atvs = self.random_pop(self.atvs)
        alt_block = self.miner.mine_alt_block(alt_tip, vbk_blocks, vtbs, atvs)

        vbk_tx = self.miner.create_vbk_tx(alt_block)
        self.vbk_txs.append(vbk_tx)

    def pick_btc_block(self):
        return self.miner.get_btc_tip()

    def pick_vbk_block(self):
        return self.miner.get_vbk_tip()

    def pick_alt_block(self):
        return self.miner.get_alt_tip()

    def random_run(self, tasks):
        target = self.random_value(len(tasks))
        return tasks[target]()

    def random_pop(self, items):
        target = self.random_value(len(items))
        return items.pop(target)

    def random_pick(self, items):
        target = self.random_value(len(items))
        return items.pop(target)

    def random_value(self, bound):
        target = self.random.randrange(bound)
        logger.info('Picked random value: {} from [0, {})'.format(target, bound))
        return target

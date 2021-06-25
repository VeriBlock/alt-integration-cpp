from random import Random
from .pop_util import mine_alt_block as pop_mine_alt_block

class RandomPopMiner:
    def __init__(self, node, mock_miner, seed):
        self.node = node
        self.mm = mock_miner
        self.random = Random(seed)
        self.vbk_endorsed_interval = 10
        self.alt_endorsed_interval = 10
        self.btc_mempool = []
        self.vbk_pop_data_queue = []
        self.vbk_pop_mempool = []
        self.vbk_mempool = []
        self.alt_pop_data_queue = []

    def submit_btc_tx(self):
        tip_height = self.mm.vbkTip.height
        endorsed_height = self.random.randrange(
            max(0, tip_height - self.vbk_endorsed_interval),
            tip_height + 1)

        vbk_published_block = self.mm.getAncestor(self.mm.vbkTip, endorsed_height)
        btc_tx = self.mm.createBtcTxEndorsingVbkBlock(vbk_published_block)

        self.btc_mempool.append((btc_tx, vbk_published_block, self.mm.btcTip.getHash()))

    def mine_btc_block(self):
        btc_txs = [btc_tx for btc_tx, _, _ in self.btc_mempool]
        btc_block_of_proof = self.mm.mineBtcBlocks(1, self.mm.btcTip.getHash(), btc_txs)
        for tx_data in self.btc_mempool:
            vbk_pop_tx = self.mm.createVbkPopTxEndorsingVbkBlock(btc_block_of_proof, *tx_data)
            self.vbk_pop_data_queue.append(vbk_pop_tx)
        self.btc_mempool.clear()

    def submit_vbk_pop_tx(self):
        if len(self.vbk_pop_data_queue) == 0:
            return
        index = self.random.randrange(0, len(self.vbk_pop_data_queue))
        vbk_pop_tx = self.vbk_pop_data_queue.pop(index)
        self.vbk_pop_mempool.append(vbk_pop_tx)

    def submit_vbk_tx(self):
        tip_height = self.node.getblockcount()
        endorsed_height = self.random.randrange(
            max(0, tip_height - self.alt_endorsed_interval),
            tip_height + 1)

        pop_data = self.node.getpopdatabyheight(endorsed_height)
        from pypoptools.pypopminer import PublicationData
        pub = PublicationData()
        pub.header = str(pop_data.header)
        pub.payoutInfo = str(self.node.getpayoutinfo(self.node.getnewaddress()))
        pub.identifier = self.node.getpopparams().networkId
        pub.contextInfo = str(pop_data.authenticated_context)

        vbk_tx = self.mm.createVbkTxEndorsingAltBlock(pub)

        self.vbk_mempool.append((vbk_tx, pop_data.last_known_vbk_block))

    def mine_vbk_block(self, pop):
        if pop:
            self.mm.mineVbkBlocks(1, self.mm.vbkTip.getHash(), self.vbk_pop_mempool, True)
            self.vbk_pop_mempool.clear()
        else:
            vbk_txs = [vbk_tx for vbk_tx, _ in self.vbk_mempool]
            vbk_block_of_proof = self.mm.mineVbkBlocks(1, self.mm.vbkTip.getHash(), vbk_txs, False)
            for tx_data in self.vbk_mempool:
                pop_data = self.mm.createPopDataEndorsingAltBlock(vbk_block_of_proof, *tx_data)
                self.alt_pop_data_queue.append(pop_data)
            self.vbk_mempool.clear()

    def submit_alt_pop_data(self):
        if len(self.alt_pop_data_queue) == 0:
            return
        index = self.random.randrange(0, len(self.alt_pop_data_queue))
        pop_data = self.alt_pop_data_queue.pop(index)
        for vbk_block in pop_data.context:
            self.node.submitpopvbk(vbk_block.toVbkEncodingHex())
        for vtb in pop_data.vtbs:
            self.node.submitpopvtb(vtb.toVbkEncodingHex())
        for atv in pop_data.atvs:
            self.node.submitpopatv(atv.toVbkEncodingHex())

    def mine_alt_block(self):
        pop_mine_alt_block(self.node, nblocks=1)

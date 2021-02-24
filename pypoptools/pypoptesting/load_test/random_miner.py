from random import Random


class RandomVbkPopMiner:
    def __init__(self, mock_miner, seed):
        self.mm = mock_miner
        self.random = Random(seed)
        self.btc_mempool = []
        self.vbk_pop_data_queue = []
        self.vbk_pop_mempool = []

    def submit_btc_tx(self):
        tip_height = self.mm.vbkTip.height
        endorsed_interval = 10
        endorsed_height = self.random.randrange(max(0, tip_height - endorsed_interval), tip_height - 1)

        vbk_published_block = self.mm.getAncestor(self.mm.vbkTip, endorsed_height)
        btc_tx = self.mm.createBtcTxEndorsingVbkBlock(vbk_published_block)

        self.btc_mempool.append((btc_tx, vbk_published_block, self.mm.btcTip))

    def mine_btc_block(self):
        btc_txs = [btc_tx for btc_tx, vbk_published_block in self.btc_mempool]
        btc_block_of_proof = self.mm.mineBtcBlocks(1, self.mm.btcTip, btc_txs)
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

    def mine_vbk_block(self):
        self.mm.mineVbkBlocks(1, self.mm.vbkTip, self.vbk_pop_mempool)
        self.vbk_pop_mempool.clear()


class RandomAltPopMiner:
    def __init__(self, node, mock_miner, seed):
        self.node = node
        self.mm = mock_miner
        self.random = Random(seed)
        self.vbk_mempool = []
        self.alt_pop_data_queue = []

    def submit_vbk_tx(self):
        tip_height = self.node.getblockcount()
        endorsed_interval = 10
        endorsed_height = self.random.randrange(max(0, tip_height - endorsed_interval), tip_height - 1)

        pop_data = self.node.getpopdatabyheight(endorsed_height)
        from pypoptools.pypopminer import PublicationData
        pub = PublicationData()
        pub.header = pop_data.header
        pub.payoutInfo = self.node.getpayoutinfo(self.node.getnewaddress())
        pub.identifier = self.node.getpopparams().networkId

        vbk_tx = self.mm.createVbkTxEndorsingAltBlock(pub)

        self.vbk_mempool.append((vbk_tx, pop_data.last_known_vbk_block))

    def mine_vbk_block(self):
        vbk_txs = [vbk_tx for vbk_tx, last_known_vbk_block in self.vbk_mempool]
        vbk_block_of_proof = self.mm.mineVbkBlocks(1, self.mm.vbkTip, vbk_txs)
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
        self.node.generate(1)

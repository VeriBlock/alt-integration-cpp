from pypopminer import PublicationData

from util.logging import logger


class PopMiner:

    def __init__(self, node, apm):
        self.node = node
        self.apm = apm

    def mine_btc_block(self, tip, txs):
        logger.info('Mining BTC block with tip={} and txs={}'.format(self.encode(tip), self.encode(txs)))
        self.apm.submitBtcTxs(txs)
        btc_block = self.apm.mineBtcBlocks(tip, 1)
        logger.info('Mined BTC block={}'.format(self.encode(btc_block)))
        return btc_block

    def mine_vbk_block(self, tip, txs):
        logger.info('Mining VBK block with tip={} and txs={}'.format(self.encode(tip), self.encode(txs)))
        self.apm.submitVbkTxs(txs)
        vbk_block = self.apm.mineVbkBlocks(tip, 1)
        vtbs = self.apm.getVTBs(vbk_block)
        atvs = self.apm.getATVs(vbk_block)
        logger.info('Mined VBK block={} with VTBs={} and ATVs={}'
                    .format(self.encode(vbk_block), self.encode(vtbs), self.encode(atvs)))
        return vbk_block, vtbs, atvs

    def mine_alt_block(self, tip, vbk_blocks, vtbs, atvs):
        logger.info('Mining ALT block with tip={} and VBK blocks={} and VTBs={} and ATVs={}'
                    .format(self.encode(tip), self.encode(vbk_blocks), self.encode(vtbs), self.encode(atvs)))
        self.node.submitpop(self.encode(vbk_blocks), self.encode(vtbs), self.encode(atvs))
        address = self.node.getnewaddress()
        alt_block = self.node.generatetoaddress(1, address)[0]
        logger.info('Mined ALT block={}'.format(alt_block))
        return alt_block

    def create_btc_tx(self, vbk_block):
        logger.info('Creating BTC tx with VBK block={}'.format(self.encode(vbk_block)))
        btc_tx = self.apm.createBtcTxEndorsingVbkBlock(vbk_block)
        logger.info('Created BTC tx={}'.format(self.encode(btc_tx)))
        return btc_tx

    def create_vbk_pop_tx(self, vbk_block, btc_tx, btc_block, last_btc_block_hash):
        logger.info('Creating VBK POP tx with VBK block={} and BTC tx={} and BTC block={}'
                    .format(self.encode(vbk_block), self.encode(btc_tx), self.encode(btc_block)))
        vbk_pop_tx = self.apm.createVbkPopTxEndorsingVbkBlock(btc_block, btc_tx, vbk_block, last_btc_block_hash)
        logger.info('Created VBK POP tx={}'.format(self.encode(vbk_pop_tx)))
        return vbk_pop_tx

    def create_vbk_tx(self, alt_block):
        logger.info('Creating VBK tx with ALT block={}'.format(alt_block))
        height = self.node.getblockheader(alt_block)['height']
        pop_data = self.node.getpopdata(height)
        header = pop_data['block_header']

        address = self.node.getnewaddress()
        script_pub_key = self.node.getaddressinfo(address)['scriptPubKey']
        pub = PublicationData()
        pub.header = header
        pub.payoutInfo = script_pub_key
        pub.identifier = 0x3ae6ca

        vbk_tx = self.apm.createVbkTxEndorsingAltBlock(pub)
        logger.info('Created VBK tx={}'.format(self.encode(vbk_tx)))
        return vbk_tx

    def get_btc_tip(self):
        btc_tip = self.apm.btcTip
        logger.info('Requested BTC tip={}'.format(self.encode(btc_tip)))
        return btc_tip

    def get_vbk_tip(self):
        vbk_tip = self.apm.vbkTip
        logger.info('Requested VBK tip={}'.format(self.encode(vbk_tip)))
        return vbk_tip

    def get_alt_tip(self):
        alt_tip = self.node.getbestblockhash()
        logger.info('Returned ALT tip={}'.format(alt_tip))
        return alt_tip

    @staticmethod
    def encode(obj):
        if obj is list:
            return [item.toVbkEncodingHex() for item in obj]
        return obj.toVbkEncodingHex()


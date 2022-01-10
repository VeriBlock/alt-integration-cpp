#!/usr/bin/env python3
# Copyright (c) 2019-2022 Xenios SEZC
# https://www.veriblock.org
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

import unittest


class MockMinerTestCase(unittest.TestCase):
    def test_mock_miner_can_produce_publications(self):
        from pypoptools.pypopminer import MockMiner, PublicationData
        m = MockMiner()
        self.assertEqual(m.vbkTip.previousBlock, "000000000000000000000000")
        self.assertEqual(m.vbkTip.height, 0)

        endorsed = m.mineVbkBlocks(100)
        self.assertEqual(endorsed.height, 100)
        m.endorseVbkBlock(endorsed, m.btcTip.getHash())

        p = PublicationData()
        p.identifier = 1337
        p.header = "0011223344"
        p.payoutInfo = "0014aaddff"
        payloads = m.endorseAltBlock(p, endorsed.getHash())
        print(repr(payloads))
        self.assertEqual(len(payloads.vtbs), 1)
        self.assertEqual(m.vbkTip.height, 102)

    def test_uints_handle_both_bytes_and_uint8(self):
        from pypoptools.pypopminer import PublicationData, BtcBlock
        p = PublicationData()
        p.header = '11'
        p.payoutInfo = b'22'

        self.assertEqual(str(p.header), '11')
        self.assertEqual(str(p.payoutInfo), '22')

        b = BtcBlock()
        self.assertEqual(str(b.merkleRoot), "0000000000000000000000000000000000000000000000000000000000000000")
        b.merkleRoot = '1111111111111111111111111111111111111111111111111111111111111111'
        self.assertEqual(str(b.merkleRoot), "1111111111111111111111111111111111111111111111111111111111111111")
        b.merkleRoot = b'2222222222222222222222222222222222222222222222222222222222222222'
        self.assertEqual(str(b.merkleRoot), "2222222222222222222222222222222222222222222222222222222222222222")

    def test_entities_default_constructible(self):
        from pypoptools.pypopminer import MockMiner, PublicationData, BtcBlock, VbkBlock
        a = MockMiner()
        b = PublicationData()
        c = BtcBlock()
        d = VbkBlock()

    def test_atomic_methods(self):
        from pypoptools.pypopminer import MockMiner, PublicationData
        m = MockMiner()

        last_known_btc_hash = m.btcTip.getHash()
        last_known_vbk_hash = m.vbkTip.getHash()
        m.mineBtcBlocks(1)
        m.mineVbkBlocks(1)

        endorsed_block = m.vbkTip
        btc_tx = m.createBtcTxEndorsingVbkBlock(endorsed_block)
        btc_block_of_proof = m.mineBtcBlocks(1, [btc_tx])
        vbk_pop_tx = m.createVbkPopTxEndorsingVbkBlock(
            btc_block_of_proof, btc_tx, endorsed_block, last_known_btc_hash)
        self.assertEqual(vbk_pop_tx.publishedBlock.toHex(), endorsed_block.toHex())
        self.assertEqual(vbk_pop_tx.bitcoinTransaction.getHash(), btc_tx.getHash())
        self.assertEqual(vbk_pop_tx.blockOfProof.getHash(), btc_block_of_proof.getHash())
        self.assertEqual(len(vbk_pop_tx.blockOfProofContext), 1)

        vbk_containing_block = m.mineVbkBlocks(1, [vbk_pop_tx], True)
        vtb = m.createVTB(vbk_containing_block, vbk_pop_tx)
        self.assertEqual(vtb.transaction.getHash(), vbk_pop_tx.getHash())
        self.assertEqual(vtb.containingBlock.getHash(), vbk_containing_block.getHash())

        pub_data = PublicationData()
        pub_data.identifier = 1337
        pub_data.header = "0011223344"
        pub_data.payoutInfo = "0014aaddff"
        vbk_tx = m.createVbkTxEndorsingAltBlock(pub_data)
        vbk_block_of_proof = m.mineVbkBlocks(1, [vbk_tx], False)
        atv = m.createATV(vbk_block_of_proof, vbk_tx)
        self.assertEqual(atv.transaction.getHash(), vbk_tx.getHash())
        self.assertEqual(atv.blockOfProof.getHash(), vbk_block_of_proof.getHash())

        pop_data = m.createPopDataEndorsingAltBlock(vbk_block_of_proof, vbk_tx, last_known_vbk_hash)
        self.assertEqual(len(pop_data.atvs), 1)
        self.assertEqual(len(pop_data.vtbs), 1)
        self.assertEqual(len(pop_data.context), 2)

    def test_complex_methods(self):
        from pypoptools.pypopminer import MockMiner, PublicationData
        m = MockMiner()

        last_known_btc_hash = m.btcTip.getHash()
        m.mineBtcBlocks(1)

        endorsed_block = m.vbkTip
        vbk_pop_tx = m.createVbkPopTxEndorsingVbkBlock(endorsed_block, last_known_btc_hash)
        self.assertEqual(vbk_pop_tx.publishedBlock.getHash(), endorsed_block.getHash())
        self.assertEqual(len(vbk_pop_tx.blockOfProofContext), 1)

        last_known_vbk_hash = m.vbkTip.getHash()
        m.mineVbkBlocks(1, [vbk_pop_tx], True)

        endorsed_block = m.vbkTip
        vtb = m.endorseVbkBlock(endorsed_block, last_known_btc_hash)
        self.assertEqual(vtb.transaction.publishedBlock.getHash(), endorsed_block.getHash())
        self.assertEqual(len(vtb.transaction.blockOfProofContext), 2)

        pub_data = PublicationData()
        pub_data.identifier = 1337
        pub_data.header = "0011223344"
        pub_data.payoutInfo = "0014aaddff"
        pop_data = m.endorseAltBlock(pub_data, last_known_vbk_hash)
        self.assertEqual(len(pop_data.atvs), 1)
        self.assertEqual(len(pop_data.vtbs), 2)
        self.assertEqual(len(pop_data.context), 2)


if __name__ == '__main__':
    unittest.main()

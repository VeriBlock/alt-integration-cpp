#!/usr/bin/env python3
# Copyright (c) 2019-2020 Xenios SEZC
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


if __name__ == '__main__':
    unittest.main()

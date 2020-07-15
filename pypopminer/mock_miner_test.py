#!/usr/bin/env python3
# Copyright (c) 2019-2020 Xenios SEZC
# https://www.veriblock.org
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

import unittest

class MockMinerTestCase(unittest.TestCase):
    def test_can_import_entities(self):
        try:
            import pypopminer
        except Exception as e:
            self.fail("pypopminer can not be imported: " + e)

    def test_mock_miner_can_produce_publications(self):
        from pypopminer import MockMiner, PublicationData
        m = MockMiner()
        self.assertEqual(m.vbkTip.getPreviousBlock().toHex(), "000000000000000000000000")
        self.assertEqual(m.vbkTip.getHeight(), 0)

        endorsed = m.mineVbkBlocks(100)
        self.assertEqual(endorsed.getHeight(), 100)
        m.endorseVbkBlock(endorsed, m.btcTip.getHash(), 10)

        p = PublicationData()
        p.identifier = 1337
        p.header = "0011223344"
        p.payoutInfo = "0014aaddff"
        payloads = m.endorseAltBlock(p, m.vbkTip.getHash())
        print(repr(payloads))
        self.assertEqual(len(payloads.vtbs), 10)
        self.assertEqual(m.vbkTip.getHeight(), 102)

    def test_uints_handle_both_bytes_and_uint8(self):
        from pypopminer import PublicationData, BtcBlock
        p = PublicationData()
        p.header = '11'
        p.payoutInfo = b'22'

        self.assertEqual(p.header.toHex(), '11')
        self.assertEqual(p.payoutInfo.toHex(), '22')

        b = BtcBlock()
        self.assertEqual(b.getMerkleRoot().toHex(), "0000000000000000000000000000000000000000000000000000000000000000")
        b.setMerkleRoot('1111111111111111111111111111111111111111111111111111111111111111')
        self.assertEqual(b.getMerkleRoot().toHex(), "1111111111111111111111111111111111111111111111111111111111111111")
        b.setMerkleRoot(b'2222222222222222222222222222222222222222222222222222222222222222')
        self.assertEqual(b.getMerkleRoot().toHex(), "2222222222222222222222222222222222222222222222222222222222222222")

if __name__ == '__main__':
    unittest.main()


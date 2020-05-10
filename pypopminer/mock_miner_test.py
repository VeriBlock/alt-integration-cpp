import unittest

class MockMinerTestCase(unittest.TestCase):
    def test_can_import_entities(self):
        try:
            import pypopminer
        except Exception as e:
            self.fail("pypopminer can not be imported: " + e)

    def test_mock_miner_can_produce_publications(self):
        from pypopminer import MockMiner, makePublicationData
        m = MockMiner()
        self.assertEqual(str(m.vbkTip.previousBlock), "000000000000000000000000")
        self.assertEqual(m.vbkTip.height, 0)

        endorsed = m.mineVbkBlocks(100)
        self.assertEqual(endorsed.height, 100)

        btctx = m.createBtcTxEndorsingVbkBlock(endorsed)
        btccontaining = m.mineBtcBlocks(1)
        vbkpoptx = m.createVbkPopTxEndorsingVbkBlock(btccontaining, btctx, endorsed, str(m.vbkTip.getHash()))
        vbkcontaining = m.mineVbkBlocks(1)

        p = makePublicationData(1337, "0011223344", "0014aaddff")
        payloads = m.endorseAltBlock(p, m.vbkTip.getHash())
        self.assertEqual(len(payloads.vtbs), 1)
        print(payloads)


if __name__ == '__main__':
    unittest.main()


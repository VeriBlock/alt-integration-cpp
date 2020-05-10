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
        m.endorseVbkBlock(endorsed, m.btcTip.getHash(), 10)

        p = makePublicationData(1337, "0011223344", "0014aaddff")
        payloads = m.endorseAltBlock(p, m.vbkTip.getHash())
        print(repr(payloads))
        self.assertEqual(len(payloads.vtbs), 10)


if __name__ == '__main__':
    unittest.main()


import unittest


class ImportsTest(unittest.TestCase):
    def test_can_import_pypoptools(self):
        import pypoptools

    def test_can_import_node(self):
        from pypoptools.pypoptesting import Node

    def test_can_import_mock_miner(self):
        from pypoptools.pypopminer import MockMiner



if __name__ == '__main__':
    unittest.main()

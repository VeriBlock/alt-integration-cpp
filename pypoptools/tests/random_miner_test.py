import random
import pathlib
import shutil
import tempfile
import unittest

from pypoptools.pypopminer import MockMiner
from pypoptools.pypoptesting import RandomPopMiner, VBitcoindNode, mine_until_pop_enabled


class RandomMinerTestCase(unittest.TestCase):
    def test(self):
        datadir = tempfile.mkdtemp()
        node = VBitcoindNode(0, pathlib.Path(datadir))
        mock_miner = MockMiner()
        seed = random.randrange(0, 1000)
        m = RandomPopMiner(node, mock_miner, seed)

        node.start()
        mine_until_pop_enabled(node)

        m.submit_btc_tx()
        m.mine_btc_block()
        m.submit_vbk_pop_tx()
        m.mine_vbk_block(pop=True)
        m.submit_vbk_tx()
        m.mine_vbk_block(pop=False)
        m.submit_alt_pop_data()
        m.mine_alt_block()

        node.stop()
        shutil.rmtree(datadir)


if __name__ == '__main__':
    unittest.main()

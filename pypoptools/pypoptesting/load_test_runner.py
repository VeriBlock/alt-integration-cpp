import argparse
import os
import random
import sys
import time
import tempfile
import traceback

from pypoptools.pypopminer import MockMiner
from pypoptools.pypoptesting import VBitcoindNode, generate_endorsed, mine_until_pop_enabled


def main():
    parser = argparse.ArgumentParser(prog=os.path.basename(sys.argv[0]), usage='python3 %(prog)s [options]')
    parser.add_argument('--alt-blocks', type=int, default=10, nargs='?',
                        help='Maximum blocks to generate in ALT blockchain (default: 10)')
    parser.add_argument('--vbk-blocks', type=int, default=10, nargs='?',
                        help='Maximum blocks to generate in VBK blockchain (default: 10)')
    parser.add_argument('--btc-blocks', type=int, default=10, nargs='?',
                        help='Maximum blocks to generate in BTC blockchain (default: 10)')
    parser.add_argument('--atvs', type=int, default=10, nargs='?',
                        help='Maximum ATVs to endorse ALT blockchain (default: 10)')
    parser.add_argument('--vtbs', type=int, default=10, nargs='?',
                        help='Maximum VTBs to endorse VBK blockchain (default: 10)')
    parser.add_argument('--timeout', type=int, default=60, nargs='?',
                        help='Maximum execution time in seconds (default: 60)')
    parser.add_argument('--seed', type=int, nargs='?', help='Seed to instantiate random generator (default: random)')
    args = parser.parse_args()

    seed = args.seed or random.randrange(0, 2 ** 32)

    print('Configuration:')
    print('- Max ALT blocks: {}'.format(args.alt_blocks))
    print('- Max VBK blocks: {}'.format(args.vbk_blocks))
    print('- Max BTC blocks: {}'.format(args.btc_blocks))
    print('- Max VTBs: {}'.format(args.vtbs))
    print('- Max ATVs: {}'.format(args.vtbs))
    print('- Timeout: {} secs'.format(args.timeout))
    print('- Seed: {}'.format(seed))

    datadir = tempfile.mkdtemp()
    node = VBitcoindNode(0, datadir)
    print('Node logs available at {}'.format(datadir))

    mock_miner = MockMiner()

    try:
        print('Load test started')

        node.start()
        mine_until_pop_enabled(node)

        start = time.time()
        generate_endorsed(node, mock_miner,
                          alt_blocks=args.alt_blocks,
                          vbk_blocks=args.vbk_blocks,
                          btc_blocks=args.btc_blocks,
                          vtbs=args.vtbs, atvs=args.vtbs,
                          seed=seed, timeout=args.timeout)
        elapsed = time.time() - start

        print('Load test finished in {:.3f} sec'.format(elapsed))
    except Exception:
        print(traceback.format_exc())
        print('Load test failed')
    finally:
        node.stop()


if __name__ == '__main__':
    main()

import argparse
import os
import sys

from rpc.node_rpc import NodeRpc
from util.load_test_util import load_test
from pypopminer import MockMiner
from random import randrange


def main():
    parser = argparse.ArgumentParser(
        prog=os.path.basename(sys.argv[0]),
        usage='python3 %(prog)s [options]')
    parser.add_argument(
        '--url',
        default='http://127.0.0.1:18443',
        nargs='?',
        help='vBTC JSON RPC url (default: "http://127.0.0.1:18443")')
    parser.add_argument(
        '--user',
        help='vBTC JSON RPC username')
    parser.add_argument(
        '--password',
        help='vBTC JSON RPC password')
    parser.add_argument(
        '--max-blocks',
        type=int,
        default=100,
        nargs='?',
        help='Testing stops when height reaches this value (default: 100)')
    parser.add_argument(
        '--max-hours',
        type=int,
        default=1,
        nargs='?',
        help='Testing stops when execution time in hours reaches this value (default: 1)')
    parser.add_argument(
        '--seed',
        type=int,
        nargs='?',
        help='Seed to instantiate random generator with (default: random)')
    args = parser.parse_args()

    if args.user is None or args.password is None:
        print('Error: --user and --password should be specified')
        parser.print_help()
        sys.exit(2)

    node = NodeRpc(args.url, args.user, args.password)
    apm = MockMiner()
    seed = args.seed or randrange(sys.maxsize)

    print('Load test starting')
    print('- Max height: {} blocks'.format(args.max_blocks))
    print('- Max execution time: {} hours'.format(args.max_hours))
    print('- Seed: {}'.format(seed))

    try:
        blocks, elapsed = load_test(node, apm, args.max_blocks, args.max_hours, seed)

        print('Load test finished')
        print('- Height: {} blocks'.format(blocks))
        print('- Execution time: {:.3f} sec'.format(elapsed))

    except Exception as e:
        print('Load test failed')
        print('Error: {}'.format(e))


if __name__ == '__main__':
    main()

import argparse
import os
import sys
import time

from pypoptools.pypopminer import MockMiner

from ..framework import JsonRpcApi
from .generate_util import generate_endorsed


def main():
    parser = argparse.ArgumentParser(prog=os.path.basename(sys.argv[0]), usage='python3 %(prog)s [options]')
    parser.add_argument('--host', default='127.0.0.1', nargs='?', help='JSON RPC url (default: "127.0.0.1")')
    parser.add_argument('--port', type=int, default=18443, nargs='?', help='JSON RPC port (default: 18443)')
    parser.add_argument('--user', default=None, nargs='?', help='JSON RPC username (default: None)')
    parser.add_argument('--password', default=None, nargs='?', help='JSON RPC password (default: None)')
    parser.add_argument('--alt-blocks', type=int, default=100, nargs='?',
                        help='Maximum blocks to generate in ALT blockchain (default: 100)')
    parser.add_argument('--vbk-blocks', type=int, default=100, nargs='?',
                        help='Maximum blocks to generate in VBK blockchain (default: 100)')
    parser.add_argument('--btc-blocks', type=int, default=100, nargs='?',
                        help='Maximum blocks to generate in BTC blockchain (default: 100)')
    parser.add_argument('--atvs', type=int, default=100, nargs='?',
                        help='Maximum ATVs to endorse ALT blockchain (default: 100)')
    parser.add_argument('--vtbs', type=int, default=100, nargs='?',
                        help='Maximum VTBs to endorse VBK blockchain (default: 100)')
    parser.add_argument('--timeout', type=int, default=60, nargs='?',
                        help='Maximum execution time in seconds (default: 60)')
    parser.add_argument('--seed', type=int, nargs='?', help='Seed to instantiate random generator (default: random)')
    args = parser.parse_args()

    url = 'http://{}:{}'.format(args.host, args.port)
    node = JsonRpcApi(url, args.user, args.password)

    info = node.getblockchaininfo()
    chain = info['chain']
    if chain != 'regtest':
        raise Exception('Unexpected chain (expected: regtest, actual: {})'.format(chain))
    blocks = info['blocks']
    if blocks > 0:
        raise Exception('Unexpected blocks count (expected: 0, actual: {})'.format(blocks))

    print('Configuration:')
    print('- Max ALT blocks: {}'.format(args.alt_blocks))
    print('- Max VBK blocks: {}'.format(args.vbk_blocks))
    print('- Max BTC blocks: {}'.format(args.btc_blocks))
    print('- Max VTBs={}'.format(args.vtbs))
    print('- Max ATVs={}'.format(args.vtbs))
    print('- Timeout: {} secs'.format(args.timeout))
    print('- Seed: {}'.format(args.seed))
    print('Load test started')

    try:
        mock_miner = MockMiner()

        start = time.time()
        generate_endorsed(node, mock_miner,
                          alt_blocks=args.alt_blocks,
                          vbk_blocks=args.vbk_blocks,
                          btc_blocks=args.btc_blocks,
                          vtbs=args.vtbs, atvs=args.vtbs,
                          timeout=args.timeout, seed=args.seed)
        elapsed = time.time() - start

        print('Load test finished')
        print('- ALT block count: {}'.format(node.getblockcount()))
        print('- VBK block count: {}'.format(mock_miner.vbkTip.height))
        print('- BTC block count: {}'.format(mock_miner.btcTip.height))
        print('- Execution time: {:.3f} sec'.format(elapsed))

    except Exception as e:
        print('Load test failed')
        print('Error: {}'.format(e))


if __name__ == '__main__':
    main()

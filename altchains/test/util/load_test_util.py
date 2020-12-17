import time

from rpc.random_miner import RandomMiner


def load_test(node, apm, max_blocks, max_hours, seed=None):
    info = node.getblockchaininfo()
    chain = info['chain']
    if chain != 'regtest':
        raise Exception('Unexpected chain (expected: regtest, actual: {})'.format(chain))
    blocks = info['blocks']
    if blocks > 0:
        raise Exception('Unexpected blocks count (expected: 0, actual: {})'.format(blocks))

    miner = RandomMiner(node, apm, seed)

    start = time.time()

    while True:
        blocks = node.getblockcount()
        elapsed = time.time() - start
        if blocks >= max_blocks or elapsed >= max_hours * 60 * 60:
            return blocks, elapsed

        miner.execute_command()




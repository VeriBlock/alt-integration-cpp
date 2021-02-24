import itertools
import random
import time

from .random_miner import RandomVbkPopMiner, RandomAltPopMiner


def generate_endorsed(node, mock_miner, atvs, vtbs, alt_blocks, vbk_blocks, btc_blocks, timeout=60, seed=None):
    if seed is None:
        seed = random.randrange(2 ** 32)
    vpm = RandomVbkPopMiner(mock_miner, seed)
    apm = RandomAltPopMiner(node, mock_miner, seed)

    commands = chain_random(
        [
            itertools.repeat(apm.mine_alt_block, times=alt_blocks),
            itertools.repeat(vpm.mine_btc_block, times=btc_blocks),
            iter(random.choices([apm.mine_vbk_block, vpm.mine_vbk_block], weights=[atvs, vtbs], k=vbk_blocks)),
            iter(parentheses_sequence(vpm.submit_btc_tx, vpm.submit_vbk_pop_tx, n=vtbs)),
            iter(parentheses_sequence(apm.submit_vbk_tx, apm.submit_alt_pop_data, n=atvs))
        ],
        sizes=[
            alt_blocks,
            btc_blocks,
            vbk_blocks,
            2 * vtbs,
            2 * atvs
        ]
    )

    start = time.time()
    for command in commands:
        if time.time() - start >= timeout:
            break
        command()


def parentheses_sequence(open_object, close_object, n):
    c = [0] * (n + 1)
    c[0] = 1
    for i in range(1, n + 1):
        for j in range(i):
            c[i] += c[j] * c[i - 1 - j]

    seq = [None] * (2 * n)

    def rec(a, b):
        assert (b - a) % 2 == 0
        k = (b - a) / 2
        if k == 0:
            return
        weights = [c[i] * c[k - 1 - i] for i in range(k)]
        index = random.choices(range(k), weights=weights)[0]
        m = a + 1 + 2 * index
        seq[a] = open_object
        seq[m] = close_object
        rec(a + 1, m)
        rec(m + 1, b)

    rec(0, 2 * n)

    return seq


def chain_random(iterables, sizes):
    assert len(iterables) == len(sizes)
    n = len(iterables)
    for i in range(sum(sizes)):
        index = random.choices(range(n), weights=sizes)[0]
        yield next(iterables[index])
        sizes[index] -= 1


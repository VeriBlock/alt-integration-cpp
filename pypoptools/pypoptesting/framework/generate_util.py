import itertools
import random
import time

from .random_miner import RandomPopMiner


def generate_endorsed(node, mock_miner, atvs, vtbs, alt_blocks, vbk_blocks, btc_blocks, seed, timeout=60):
    m = RandomPopMiner(node, mock_miner, seed)

    commands = chain_random(
        [
            itertools.repeat(m.mine_alt_block, times=alt_blocks),
            itertools.repeat(m.mine_btc_block, times=btc_blocks),
            iter(random.choices(
                [lambda: m.mine_vbk_block(pop=True), lambda: m.mine_vbk_block(pop=False)],
                weights=[atvs, vtbs],
                k=vbk_blocks)),
            iter(parentheses_sequence(m.submit_btc_tx, m.submit_vbk_pop_tx, n=vtbs)),
            iter(parentheses_sequence(m.submit_vbk_tx, m.submit_alt_pop_data, n=atvs))
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
    def calc():
        c[0] = 1
        for k in range(1, n + 1):
            for i in range(k):
                c[k] += c[i] * c[k - 1 - i]

    def rec(a, b):
        if a == b:
            return
        assert (b - a) % 2 == 0
        k = int((b - a) / 2)
        weights = [c[i] * c[k - 1 - i] for i in range(k)]
        index = random.choices(range(k), weights=weights)[0]
        m = a + 1 + 2 * index
        seq[a] = open_object
        seq[m] = close_object
        rec(a + 1, m)
        rec(m + 1, b)

    c = [0] * (n + 1)
    calc()

    seq = [None] * (2 * n)
    rec(0, 2 * n)

    return seq


def chain_random(iterables, sizes):
    assert len(iterables) == len(sizes)
    n = len(iterables)
    for i in range(sum(sizes)):
        index = random.choices(range(n), weights=sizes)[0]
        yield next(iterables[index])
        sizes[index] -= 1


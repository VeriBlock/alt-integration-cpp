import itertools
import random
import time

from typing import List, Iterator

from .node import Node
from .random_miner import RandomPopMiner


def generate_endorsed(node: Node, mock_miner,
                      atvs: int, vtbs: int,
                      alt_blocks: int, vbk_blocks: int, btc_blocks: int,
                      seed: int, timeout=300) -> None:
    """
    Randomly executes `Node` and `MockMiner` methods.
    Instances of `Node` and `MockMiner` are wrapped with `RandomPopMiner`.
    There are following `RandomPopMiner` methods will be executed:
      - `submit_btc_tx`
      - `mine_btc_block`
      - `submit_vbk_pop_tx`
      - `submit_vbk_tx`
      - `mine_vbk_block`
      - `submit_pop_data`
      - `mine_alt_block`

    Invocation of `mine_vbk_blocks` differs for vbk pop transactions or usual vbk transactions.
    So we randomly choose one of them with weights=[`vtbs`, `atvs`], but keep `vbk_blocks` executions in total.

    Each invocation of `submit_vbk_pop_tx` or `submit_pop_data` requires at least one invocation of
    `submit_btc_tx` or `submit_vbk_tx` respectively before.
    So pairs of methods (`submit_btc_tx`, `submit_vbk_pop_tx`) and (`submit_vbk_tx`, `submit_pop_data`)
    should be executed in parentheses sequence order (see `parentheses_sequence` below).

    All of the invocations shuffled between each other (see `chain_random` below).

    Number of executions of each of these methods are defined by the corresponding arguments:
        `mine_xxx_blocks` will be executed exactly `xxx_blocks` times.
        `submit_btc_tx` and `submit_vbk_pop_tx` will be executed `vtbs` times.
        `submit_vbk_tx` and `submit_pop_data` will be executed `atvs` times.

    :param node: `Node` implementation to execute altchain commands
    :param mock_miner: `MockMiner` instance to execute commands on vbk and btc blockchains
    :param atvs: number of executions of `submit_vbk_tx` and `submit_pop_data`
    :param vtbs: number of executions of `submit_btc_tx` and `submit_vbk_pop_tx`
    :param alt_blocks: number of executions of `mine_alt_block`
    :param vbk_blocks: number of executions of `mine_vbk_block`
    :param btc_blocks: number of executions of `mine_btc_block`
    :param seed: Number to seed the random generator
    :param timeout: Maximum execution time in seconds
    :return: None
    """
    m = RandomPopMiner(node, mock_miner, seed)

    commands = chain_random(
        [
            itertools.repeat(m.mine_alt_block, times=alt_blocks),
            itertools.repeat(m.mine_btc_block, times=btc_blocks),
            iter(random.choices(
                [lambda: m.mine_vbk_block(pop=True), lambda: m.mine_vbk_block(pop=False)],
                weights=[vtbs, atvs],
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


def parentheses_sequence(open_item: object, close_item: object, n: int) -> List[object]:
    """
    Make a random list with 2*n items, which forms a parentheses sequence, e.g. (()())
    Each item is either `open_item` or `close_item`. Both of them exactly n times.

    If A and B are parentheses sequences, then (A)B is also parentheses sequence.
    Empty sequence is considered as correct parentheses sequence.

    Following the idea above, creating a parentheses sequence is made recursively.
    We put '(' at first position, then pick position for ')'.
    After that, we recursively create A and B sequences.

    The naive implementation is not correct, because possibilities of positions for ')' are not equal.
    We can't choose position uniformly, we need choose it with weights.
    These weights are defined by numbers of A and B sequences in form (A)B.
    Number of parentheses sequences with size 2*n is equal to nth Catalan number.
    Catalan number c[n] can be calculated recursively by formula:
        c[n] = { 1,                               for n = 0
               { sum[k=0->n-1] (c[k] * c[n-1-k]), for n > 0
        c[0] = 1
        c[1] = 1   ()
        c[2] = 2   ()(), (())
        c[3] = 5   ()()(), ()(()), (())(), (()()), ((()))

    Now, we can define weights for choosing position for ')'.
    Consider we have sequence with size 2*4 and we pick position 5 for ')'
        01234567
        (....)..
    There are c[2] = 2 sequences to put from 1 to 4 position,
    and c[1] = 1 sequence to put from 6 to 7 position.
    The weight for this position is equal to c[2] * c[1] = 2 * 1 = 2

    Finally, to create random parentheses sequence uniformly, we calculate Catalan numbers,
    then recursively generate parentheses sequence, picking position with weights.

    :param open_item: item to put on place of open bracket
    :param close_item: item to put on place of close bracket
    :param n: half-size of generated sequence
    :return: list with size 2*n of open_item and close_item
    """
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
        seq[a] = open_item
        seq[m] = close_item
        rec(a + 1, m)
        rec(m + 1, b)

    c = [0] * (n + 1)
    calc()

    seq = [None] * (2 * n)
    rec(0, 2 * n)

    return seq


def chain_random(iterators: List[Iterator], sizes: List[int]) -> Iterator:
    """
    Make an iterator that returns elements from the random iterable, until all of them are exhausted.
    chain_random(['ABCD', 'XYZ'], [4, 3]) -> A X B C Y Z D
    chain_random(['ABCD', 'XYZ'], [4, 3]) -> A B X Y C D Z

    Can be represented like card decks shuffling. For example, for 2 iterators of 'ABCD' and 'XYZ':
        A ->
           <- X
        B ->
        C ->
           <- Y
        D ->
           <- Z

    :param iterators: list of iterators to combine and shuffle
    :param sizes: numbers of elements in each of iterators
    :return: iterator which combines and shuffles specified ones
    """
    assert len(iterators) == len(sizes)
    n = len(iterators)
    for i in range(sum(sizes)):
        index = random.choices(range(n), weights=sizes)[0]
        yield next(iterators[index])
        sizes[index] -= 1

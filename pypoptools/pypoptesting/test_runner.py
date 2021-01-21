import pathlib

from pypoptesting.framework.node import Node
from pypoptesting.framework.test_util import run_tests
from pypoptesting.test_list import ALL_TESTS
from pypoptesting.vbitcoind_node import VBitcoindNode


def create_node(number: int, path: pathlib.Path) -> Node:
    return VBitcoindNode(number=number, datadir=path)


if __name__ == '__main__':
    run_tests(ALL_TESTS, create_node)

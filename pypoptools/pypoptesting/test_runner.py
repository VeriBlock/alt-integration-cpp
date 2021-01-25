import pathlib

from .framework.node import Node
from .framework.test_util import run_tests
from .test_list import ALL_TESTS
from .vbitcoind_node import VBitcoindNode


def create_node(number: int, path: pathlib.Path) -> Node:
    return VBitcoindNode(number=number, datadir=path)


if __name__ == '__main__':
    run_tests(ALL_TESTS, create_node)

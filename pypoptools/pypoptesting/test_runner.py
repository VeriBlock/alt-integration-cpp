import pathlib

from pypoptools.pypoptesting.framework.node import Node
from pypoptools.pypoptesting.framework.test_util import run_tests
from pypoptools.pypoptesting.tests import all_tests
from pypoptools.pypoptesting.vbitcoind_node import VBitcoindNode


def create_node(number: int, path: pathlib.Path) -> Node:
    return VBitcoindNode(number=number, datadir=path)


if __name__ == '__main__':
    run_tests(all_tests, create_node)

import pathlib
import sys
from typing import List 

from .framework.node import Node
from .framework.test_util import run_tests, CreateNodeFunction
from .tests import all_tests

def tests_running(create_node: CreateNodeFunction, test_names: List[str] = [], timeout=float('inf')):
    test_list = all_tests
    if len(test_names) != 0:
        all_tests_by_name = dict([(test.name(), test) for test in all_tests])
        test_list = [all_tests_by_name[name] for name in test_names]
    run_tests(test_list=test_list, create_node=create_node, timeout=timeout)

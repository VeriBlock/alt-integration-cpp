from typing import List 

from .framework.test_util import run_tests, CreateNodeFunction
from .tests import all_tests
from .benchmarks import all_benchmarks

def tests_running(create_node: CreateNodeFunction, test_names: List[str] = [], disabled_test_names: List[str] = [], timeout=float('inf')) -> bool:
    test_list = all_tests
    if len(test_names) != 0:
        all_tests_by_name = dict([(test.name(), test) for test in all_tests])
        test_list = [all_tests_by_name[name] for name in test_names]

    if len(disabled_test_names) != 0:
        test_list = [test for test in test_list if disabled_test_names.count(test.name()) == 0]
    
    return run_tests(test_list=test_list, create_node=create_node, timeout=timeout)

def benchmark_running(create_node: CreateNodeFunction, bench_names: List[str] = [], timeout=float('inf')) -> bool:
    bench_list = all_benchmarks
    if len(bench_names) != 0:
        all_benchmarks_by_name = dict([(bench.name(), bench) for bench in all_benchmarks])
        bench_list = [all_benchmarks_by_name[name] for name in bench_names]
    return run_tests(test_list=bench_list, create_node=create_node, timeout=timeout)

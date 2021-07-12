import datetime
import logging
import multiprocessing as mp
import os
import pathlib
import sys
import tempfile
import time
from typing import Callable

from .node import Node

CreateNodeFunction = Callable[[int, pathlib.Path], Node]

TEST_EXIT_PASSED = 0
TEST_EXIT_FAILED = 1
TEST_EXIT_SKIPPED = 77

BOLD, GREEN, RED, GREY = ("", ""), ("", ""), ("", ""), ("", "")
try:
    # Make sure python thinks it can write unicode to its stdout
    "\u2713".encode("utf_8").decode(sys.stdout.encoding)
    TICK = "✓ "
    CROSS = "✖ "
    CIRCLE = "○ "
except:
    TICK = "P "
    CROSS = "x "
    CIRCLE = "o "

if os.name != 'nt' or sys.getwindowsversion() >= (10, 0, 14393):
    if os.name == 'nt':
        import ctypes

        kernel32 = ctypes.windll.kernel32
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 4
        STD_OUTPUT_HANDLE = -11
        STD_ERROR_HANDLE = -12
        # Enable ascii color control to stdout
        stdout = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
        stdout_mode = ctypes.c_int32()
        kernel32.GetConsoleMode(stdout, ctypes.byref(stdout_mode))
        kernel32.SetConsoleMode(stdout, stdout_mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
        # Enable ascii color control to stderr
        stderr = kernel32.GetStdHandle(STD_ERROR_HANDLE)
        stderr_mode = ctypes.c_int32()
        kernel32.GetConsoleMode(stderr, ctypes.byref(stderr_mode))
        kernel32.SetConsoleMode(stderr, stderr_mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    # primitive formatting on supported
    # terminal via ANSI escape sequences:
    BOLD = ('\033[0m', '\033[1m')
    GREEN = ('\033[0m', '\033[0;32m')
    RED = ('\033[0m', '\033[0;31m')
    GREY = ('\033[0m', '\033[1;30m')


class TestResult:
    def __init__(self, name, status, time):
        self.name = name
        self.status = status
        self.time = time
        self.padding = 0

    def sort_key(self):
        if self.status == "Passed":
            return 0, self.name.lower()
        elif self.status == "Failed":
            return 2, self.name.lower()
        elif self.status == "Skipped":
            return 1, self.name.lower()

    def __repr__(self):
        glyph = None
        color = None
        if self.status == "Passed":
            color = GREEN
            glyph = TICK
        elif self.status == "Failed":
            color = RED
            glyph = CROSS
        elif self.status == "Skipped":
            color = GREY
            glyph = CIRCLE

        return color[1] + "%s | %s%s | %s s\n" % (
            self.name.ljust(self.padding), glyph, self.status.ljust(7), self.time) + color[0]

    @property
    def was_successful(self):
        return self.status != "Failed"


class TestHandler:
    """
    Trigger the test scripts passed in via the list.
    """

    def __init__(self, *, create_node, num_tests_parallel, tmpdir, test_list, timeout_duration):
        assert num_tests_parallel >= 1
        self.create_node = create_node
        self.num_jobs = num_tests_parallel
        self.parent = tmpdir
        self.timeout_duration = timeout_duration
        self.test_list = test_list
        self.num_running = 0
        self.jobs = []

    def get_next_test_name(self):
        return type(self.test_list[0]).__name__

    def get_next(self):

        while self.num_running < self.num_jobs and self.test_list:
            # Add tests
            self.num_running += 1
            test = self.test_list.pop(0)
            p = mp.Process(target=lambda: test.main(self.create_node, self.parent))
            p.start()
            self.jobs.append((test,
                              time.time(),
                              p))
        if not self.jobs:
            raise IndexError('pop from empty list')

        while True:
            # Return first proc that finishes
            time.sleep(.5)
            for job in self.jobs:
                (test, start_time, proc) = job
                name = type(test).__name__
                if int(time.time() - start_time) > self.timeout_duration:
                    # Timeout individual tests if timeout is specified (to stop
                    # tests hanging and not providing useful output).
                    proc.kill()
                if not proc.is_alive():
                    if proc.exitcode == TEST_EXIT_PASSED:
                        status = "Passed"
                    elif proc.exitcode == TEST_EXIT_SKIPPED:
                        status = "Skipped"
                    else:
                        status = "Failed"
                    self.num_running -= 1
                    self.jobs.remove(job)
                    return TestResult(name, status, int(time.time() - start_time)), test.dir

    def kill_and_join(self):
        """Send SIGKILL to all jobs and block until all have ended."""
        procs = [i[2] for i in self.jobs]

        for proc in procs:
            proc.kill()

        for proc in procs:
            proc.close()


def run_tests(test_list, create_node: CreateNodeFunction, timeout=float('inf')):
    try:
        import pypoptools.pypopminer
    except ImportError:
        logging.error("pypopminer module not available.")
        sys.exit(1)

    mp.set_start_method('fork')

    timestamp = datetime.datetime.now().strftime("%y%m%d%H%M%S")
    tmpdir = tempfile.mkdtemp(prefix="pop_{}_".format(timestamp))
    job_queue = TestHandler(
        create_node=create_node,
        tmpdir=tmpdir,
        num_tests_parallel=1,
        test_list=test_list,
        timeout_duration=timeout,
    )
    test_results = []
    test_count = len(test_list)
    if test_count != 0:
        max_len_name = len(max(test_list, key=lambda x: len(x.name())).name())
    else:
        max_len_name = 0
    start_time = time.time()

    for i in range(test_count):
        test_str = "{}/{} - {}{}{}".format(i + 1, test_count, BOLD[1], job_queue.get_next_test_name(), BOLD[0])
        print("%s running." % test_str)
        test_result, testdir = job_queue.get_next()
        test_results.append(test_result)
        if test_result.status == "Passed":
            print("%s passed, Duration: %s s" % (test_str, test_result.time))
        elif test_result.status == "Skipped":
            print("%s skipped" % (test_str))
        else:
            print("%s failed, Duration: %s s\n" % (test_str, test_result.time))

    print_results(test_results, max_len_name, (int(time.time() - start_time)))

    # Clear up the temp directory if all subdirectories are gone
    if not os.listdir(tmpdir):
        os.rmdir(tmpdir)

    all_passed = all(map(lambda test_result: test_result.was_successful, test_results))
    sys.exit(not all_passed)


def print_results(test_results, max_len_name, runtime):
    results = "\n" + BOLD[1] + "%s | %s | %s\n\n" % ("TEST".ljust(max_len_name), "STATUS   ", "DURATION") + BOLD[0]

    test_results.sort(key=TestResult.sort_key)
    all_passed = True
    time_sum = 0

    for test_result in test_results:
        all_passed = all_passed and test_result.was_successful
        time_sum += test_result.time
        test_result.padding = max_len_name
        results += str(test_result)

    status = TICK + "Passed" if all_passed else CROSS + "Failed"
    if not all_passed:
        results += RED[1]
    results += BOLD[1] + "\n%s | %s | %s s (accumulated) \n" % ("ALL".ljust(max_len_name), status.ljust(9), time_sum) + \
               BOLD[0]
    if not all_passed:
        results += RED[0]
    results += "Runtime: %s s\n" % (runtime)
    print(results)

import datetime
import logging
import shutil
import subprocess
import sys
import tempfile
import time
import os
from abc import abstractmethod
from enum import Enum
from typing import List

from .node import Node
from .sync_util import start_all
from .test_util import CreateNodeFunction, TEST_EXIT_PASSED, TEST_EXIT_SKIPPED, TEST_EXIT_FAILED


class TestStatus(Enum):
    PASSED = 1
    FAILED = 2
    SKIPPED = 3


TMPDIR_PREFIX = "pypoptesting"


class SkipTest(Exception):
    """This exception is raised to skip a test"""

    def __init__(self, message):
        self.message = message


class PopIntegrationTestMetaClass(type):
    """Metaclass for PopIntegrationTestFramework.
    Ensures that any attempt to register a subclass of `PopIntegrationTestFramework`
    adheres to a standard whereby the subclass overrides `set_test_params` and
    `run_test` but DOES NOT override either `__init__` or `main`. If any of
    those standards are violated, a ``TypeError`` is raised."""

    def __new__(cls, clsname, bases, dct):
        if not clsname == 'PopIntegrationTestFramework':
            if not ('run_test' in dct and 'set_test_params' in dct):
                raise TypeError("PopIntegrationTestFramework subclasses must override "
                                "'run_test', 'set_test_params'")
            if '__init__' in dct or 'main' in dct:
                raise TypeError("PopIntegrationTestFramework subclasses may not override "
                                "'__init__' or 'main'")

        return super().__new__(cls, clsname, bases, dct)


class PopIntegrationTestFramework(metaclass=PopIntegrationTestMetaClass):
    """Base class for a pop integration test script.
    Individual pop integration test scripts should subclass this class and override the
    set_test_params(), setup_nodes() and run_test() methods.
    The __init__() and main() methods should not be overridden.
    This class also contains various public and private helper methods."""

    def __init__(self):
        """Sets test framework defaults. Do not override this method. Instead, override the set_test_params() method"""
        self.success: bool = False
        self.nodes: List[Node] = []
        self.num_nodes: int = 0
        self.dir: str = ""
        self.set_test_params()

    def name(self) -> str:
        return type(self).__name__

    def main(self, create_node: CreateNodeFunction, tmpdir):
        """Main function. This should not be overridden by the subclass test scripts."""

        assert hasattr(self, "num_nodes"), "Test must set self.num_nodes in set_test_params()"

        try:
            self._setup(tmpdir)
            self._create_nodes(create_node)
            self.setup_nodes()
            self.run_test()
        except SkipTest as e:
            self.log.warning("Test Skipped: %s" % e.message)
            self.success = TestStatus.SKIPPED
        except AssertionError:
            self.log.exception("Assertion failed")
            self.success = TestStatus.FAILED
        except KeyError:
            self.log.exception("Key error")
            self.success = TestStatus.FAILED
        except subprocess.CalledProcessError as e:
            self.log.exception("Called Process failed with '{}'".format(e.output))
            self.success = TestStatus.FAILED
        except Exception:
            self.log.exception("Unexpected exception caught during testing")
            self.success = TestStatus.FAILED
        except KeyboardInterrupt:
            self.log.warning("Exiting after keyboard interrupt")
            self.success = TestStatus.FAILED
        finally:
            exit_code = self._shutdown()
            sys.exit(exit_code)

    @abstractmethod
    def set_test_params(self):
        """Tests must this method to change default values for number of nodes, topology, etc"""
        raise NotImplementedError

    @abstractmethod
    def run_test(self):
        """Tests must override this method to define test logic"""
        raise NotImplementedError

    def skip_test_if_missing_module(self):
        """Override this method to skip a test if a module is not compiled"""
        pass

    def setup_nodes(self):
        """"Override this method to customize the node setup"""
        start_all(self.nodes)

    def _setup(self, parent):
        """Call this method to start up the test framework object with options set."""
        # Set up temp directory and start logging
        self.dir = tempfile.mkdtemp(dir=parent, prefix="{}_".format(self.name()))
        self._start_logging()
        self.skip_test_if_missing_module()
        self.success = TestStatus.PASSED

    def _create_nodes(self, create_node: CreateNodeFunction):
        for i in range(self.num_nodes):
            datadir = os.path.join(self.dir, "node" + str(i))
            if not os.path.exists(datadir):
              os.makedirs(datadir)
            self.nodes.append(create_node(i, datadir))

    def _start_logging(self):
        # Add logger and logging handlers
        self.log = logging.getLogger('TestFramework')
        self.log.setLevel(logging.DEBUG)
        # Create file handler to log all messages
        fh = logging.FileHandler(os.path.join(self.dir, 'test_framework.log'), encoding='utf-8')
        fh.setLevel(logging.DEBUG)
        # Create console handler to log messages to stderr.
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.DEBUG)

        formatter = logging.Formatter(fmt='%(asctime)s.%(msecs)03d000Z %(name)s (%(levelname)s): %(message)s',
                                      datefmt='%Y-%m-%dT%H:%M:%S')
        formatter.converter = time.gmtime
        fh.setFormatter(formatter)
        ch.setFormatter(formatter)
        # add the handlers to the logger
        self.log.addHandler(fh)
        self.log.addHandler(ch)

    def _shutdown(self) -> int:
        if self.nodes:
            # stop all nodes
            [x.stop() for x in self.nodes]

        for h in list(self.log.handlers):
            h.flush()
            h.close()
            self.log.removeHandler(h)

        should_clean_up = self.success != TestStatus.FAILED
        if should_clean_up:
            self.log.info("Cleaning up {} on exit".format(self.dir))
            cleanup_tree_on_exit = True
        else:
            self.log.warning("Not cleaning up dir {}".format(self.dir))
            cleanup_tree_on_exit = False

        if self.success == TestStatus.PASSED:
            self.log.info("Tests successful")
            exit_code = TEST_EXIT_PASSED
        elif self.success == TestStatus.SKIPPED:
            self.log.info("Test skipped")
            exit_code = TEST_EXIT_SKIPPED
        else:
            self.log.error("Test failed. Test logging available at %s/test_framework.log", self.dir)
            exit_code = TEST_EXIT_FAILED

        if cleanup_tree_on_exit:
            shutil.rmtree(self.dir)

        self.nodes.clear()

        return exit_code

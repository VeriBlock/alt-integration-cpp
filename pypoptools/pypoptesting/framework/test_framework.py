import datetime
import logging
import pathlib
import subprocess
import sys
import tempfile
import time
from abc import abstractmethod
from enum import Enum
from typing import Callable, List

from pypoptesting.framework.node import Node
from pypoptesting.framework.test_util import TEST_EXIT_PASSED, TEST_EXIT_SKIPPED, TEST_EXIT_FAILED, CreateNodeFunction
from pypoptesting.framework.util import sync_blocks, sync_pop_mempools, sync_pop_tips, sync_all, \
    wait_for_rpc_availability


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
    set_test_params(), run_test() and setup_network() methods.
    Individual tests can also override setup_nodes() to customize the node setup.
    The __init__() and main() methods should not be overridden.
    This class also contains various public and private helper methods."""

    def __init__(self):
        """Sets test framework defaults. Do not override this method. Instead, override the set_test_params() method"""
        self.success: bool = False
        self.nodes: List[Node] = []
        self.num_nodes: int = 0
        self.dir: pathlib.Path = pathlib.Path()
        self.set_test_params()

    def name(self) -> str:
        return type(self).__name__

    def main(self, create_node: CreateNodeFunction, tmpdir):
        """Main function. This should not be overridden by the subclass test scripts."""

        assert hasattr(self, "num_nodes"), "Test must set self.num_nodes in set_test_params()"

        try:
            self.setup(tmpdir)
            self._create_nodes_(create_node)
            self.setup_network()
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
            exit_code = self.shutdown()
            sys.exit(exit_code)

    def setup(self, parent):
        """Call this method to start up the test framework object with options set."""
        # Set up temp directory and start logging
        timestamp = datetime.datetime.now().strftime("%y%m%d%H%M%S")
        self.dir = tempfile.mkdtemp(dir=parent, prefix="{}_{}_".format(timestamp, self.name()))
        self._start_logging()
        self.skip_test_if_missing_module()
        self.success = TestStatus.PASSED

    def sync_blocks(self, nodes=None, **kwargs):
        sync_blocks(nodes or self.nodes, **kwargs)

    def sync_pop_mempools(self, nodes=None, **kwargs):
        sync_pop_mempools(nodes or self.nodes, **kwargs)

    def sync_pop_tips(self, nodes=None, **kwargs):
        sync_pop_tips(nodes or self.nodes, **kwargs)

    def sync_all(self, nodes=None, **kwargs):
        sync_all(nodes or self.nodes, **kwargs)

    def setup_nodes(self):
        """"Override this method to customize the node setup"""
        pass

    def skip_test_if_missing_module(self):
        """Override this method to skip a test if a module is not compiled"""
        pass

    @abstractmethod
    def set_test_params(self):
        """Tests must this method to change default values for number of nodes, topology, etc"""
        raise NotImplementedError

    def setup_network(self):
        """
        Default implementation of setup_network starts `num_nodes` and waits for their RPC availability.
        """
        for i in self.nodes:
            i.start()

        for i in self.nodes:
            wait_for_rpc_availability(i)

    @abstractmethod
    def run_test(self):
        """Tests must override this method to define test logic"""
        raise NotImplementedError

    def shutdown(self) -> int:
        """Call this method to shut down the test framework object."""

        if self.success == TestStatus.PASSED:
            self.log.info("Tests successful")
            exit_code = TEST_EXIT_PASSED
        elif self.success == TestStatus.SKIPPED:
            self.log.info("Test skipped")
            exit_code = TEST_EXIT_SKIPPED
        else:
            self.log.error("Test failed. Test logging available at %s/test_framework.log", self.dir)
            exit_code = TEST_EXIT_FAILED

        return exit_code

    def stop_nodes(self):
        """Stop multiple test nodes"""
        for node in self.nodes:
            # Issue RPC to stop nodes
            node.stop()

    def _create_nodes_(self, factory_lambda: CreateNodeFunction):
        for i in range(self.num_nodes):
            datadir = pathlib.Path(self.dir, "node" + str(i))
            datadir.mkdir()
            self.nodes.append(factory_lambda(i, datadir))

    def _start_logging(self):
        # Add logger and logging handlers
        self.log = logging.getLogger('TestFramework')
        self.log.setLevel(logging.ERROR)
        # Create file handler to log all messages
        fh = logging.FileHandler(pathlib.Path(self.dir, 'test_framework.log'), encoding='utf-8')
        fh.setLevel(logging.INFO)
        # Create console handler to log messages to stderr.
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.ERROR)

        formatter = logging.Formatter(fmt='%(asctime)s.%(msecs)03d000Z %(name)s (%(levelname)s): %(message)s',
                                      datefmt='%Y-%m-%dT%H:%M:%S')
        formatter.converter = time.gmtime
        fh.setFormatter(formatter)
        ch.setFormatter(formatter)
        # add the handlers to the logger
        self.log.addHandler(fh)
        self.log.addHandler(ch)

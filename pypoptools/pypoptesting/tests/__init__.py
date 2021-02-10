from .node import (
    NodeRestartTest,
    NodeStartTest,
    NodeStopTest
)
from .rpc import (
    RpcConnectTest,
    RpcGenerateTest,
    RpcPreflightTest
)
from .pop import (
    PopActivateTest,
    PopE2ETest,
    PopForkResolutionTest,
    PopInitTest,
    PopMempoolGetpopTest,
    PopMempoolReorgTest,
    PopMempoolSyncTest,
    PopParamsTest,
    PopSyncTest
)

node_tests = [
    NodeStartTest(),
    NodeStopTest(),
    NodeRestartTest()
]

rpc_tests = [
    RpcPreflightTest(),
    RpcConnectTest(),
    RpcGenerateTest()
]

pop_tests = [
    PopForkResolutionTest(),
    PopMempoolGetpopTest(),
    PopInitTest(),
    PopActivateTest(),
    PopE2ETest(),
    PopMempoolSyncTest(),
    PopSyncTest(),
    PopMempoolReorgTest(),
    PopParamsTest()
]

all_tests = [
    *node_tests,
    *rpc_tests,
    *pop_tests
]

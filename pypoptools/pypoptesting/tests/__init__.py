from .node import (
    NodeRestartTest,
    NodeStartTest,
    NodeStopTest,
    NodeBasicSyncTest, 
    NodeConnectTest, 
    NodeGenerateTest,
    NodePreflightTest
)
from .rpc import (
    RpcPopRpcFunctionsTest
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
    NodeRestartTest(),
    NodeBasicSyncTest(),
    NodeConnectTest(), 
    NodeGenerateTest(),
    NodePreflightTest()

]

rpc_tests = [
    RpcPopRpcFunctionsTest()
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

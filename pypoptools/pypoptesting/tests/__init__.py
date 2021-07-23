from .node import (
    NodeRestartTest,
    NodeStartTest,
    NodeStopTest,
    NodeBasicSyncTest, 
    NodeConnectTest, 
    NodeGenerateTest,
    NodePreflightTest
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
    PopSyncTest,
    PopStressTest
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
    PopParamsTest(),
    PopStressTest()
]

all_tests = [
    *node_tests,
    *rpc_tests,
    *pop_tests
]

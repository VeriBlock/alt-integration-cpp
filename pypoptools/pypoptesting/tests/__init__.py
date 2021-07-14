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
    PopFinalizationTest,
    PopForkResolutionTest,
    PopInitTest,
    PopMempoolGetpopTest,
    PopMempoolReorgTest,
    PopMempoolSyncTest,
    PopParamsTest,
    PopSyncTest
)

from .rpc import (
    RpcFunctionsSignatureTest,
)

node_tests = [
    #NodeStartTest(),
    #NodeStopTest(),
    #NodeRestartTest(),
    #NodeBasicSyncTest(),
    #NodeConnectTest(), 
    #NodeGenerateTest(),
    #NodePreflightTest()
]

rpc_tests = [
    #RpcFunctionsSignatureTest(),
]

pop_tests = [
    PopFinalizationTest(),
    #PopForkResolutionTest(),
    #PopMempoolGetpopTest(),
    #PopInitTest(),
    #PopActivateTest(),
    #PopE2ETest(),
    #PopMempoolSyncTest(),
    #PopSyncTest(),
    #PopMempoolReorgTest(),
    #PopParamsTest()
]

all_tests = [
    *node_tests,
    *rpc_tests,
    *pop_tests
]

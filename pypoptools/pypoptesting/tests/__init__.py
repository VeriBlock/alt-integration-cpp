from .connect_and_sync import ConnectAndSync
from .pop_activate import PopActivate
from .pop_e2e import PopE2E
from .pop_fork_resolution import PopForkResolution
from .pop_init import PopInit
from .pop_mempool_getpop import PopMempoolGetpop
from .pop_mempool_reorg import PopMempoolReorg
from .pop_mempool_sync import PopMempoolSync
from .pop_params import PopParams
from .pop_sync import PopSync
from .popfr_shorter_chain_wins import PopForkResolutionShorterChainWins
from .preflight_checks import PreflightChecks
from .restart_and_sync import RestartAndSync

all_tests = [
    PopForkResolution(),
    PopMempoolGetpop(),
    PopInit(),
    PopActivate(),
    PreflightChecks(),
    PopE2E(),
    PopMempoolSync(),
    PopSync(),
    PopMempoolReorg(),
    RestartAndSync(),
    ConnectAndSync(),
    PopForkResolutionShorterChainWins(),
    PopParams(),
]

from .preflight_checks import PreflightChecks
from .popfr_shorter_chain_wins import PopForkResolutionShorterChainWins

ALL_TESTS = [
    PreflightChecks(),
    PopForkResolutionShorterChainWins()
]

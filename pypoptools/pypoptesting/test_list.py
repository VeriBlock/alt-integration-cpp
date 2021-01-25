from pypoptesting.tests.popfr_shorter_chain_wins import PopForkResolutionShorterChainWins
from pypoptesting.tests.preflight_checks import PreflightChecks

ALL_TESTS = [
    PreflightChecks(),
    PopForkResolutionShorterChainWins()
]

#include <veriblock/pop/blockchain/pop/fork_resolution.hpp>

namespace altintegration {

const char* popFrOutcomeToString(PopFrOutcome value) {
  switch (value) {
    case PopFrOutcome::CANDIDATE_IS_TIP:
      return "comparing tip to itself";
    case PopFrOutcome::CANDIDATE_INVALID_CHAIN:
      return "candidate has unknown validity, but added to an invalid chain";
    case PopFrOutcome::CANDIDATE_INVALID_PAYLOADS:
      return "candidate has invalid PoP data";
    case PopFrOutcome::CANDIDATE_PART_OF_ACTIVE_CHAIN:
      return "candidate is part of active chain";
    case PopFrOutcome::CANDIDATE_IS_TIP_SUCCESSOR:
      return "candidate is valid and after current tip";
    case PopFrOutcome::TIP_IS_FINAL:
      return "tip can not be reorganized because it's final";
    case PopFrOutcome::BOTH_DONT_CROSS_KEYSTONE_BOUNDARY:
      return "both chains do not cross keystone boundary";
    case PopFrOutcome::CANDIDATE_INVALID_INDEPENDENTLY:
      return "candidate is invalid when applied independently of active chain";
    case PopFrOutcome::HIGHER_POP_SCORE:
      return "higher PoP score";
    default:
      return "unknown";
  }
}

}
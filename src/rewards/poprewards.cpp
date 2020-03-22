#include <cassert>
#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/rewards/poprewards_calculator.hpp>
#include <veriblock/rewards/poprewards.hpp>

namespace altintegration {

static int getBestPublicationHeight(
    const std::vector<VbkEndorsement>& endorsements,
    const std::shared_ptr<VbkBlockTree>& vbk_tree) {
  int bestPublication = -1;
  for (const VbkEndorsement& e : endorsements) {
    auto* block = vbk_tree->getBlockIndex(e.blockOfProof);
    if (block == nullptr) continue;

    if (block->height < bestPublication || bestPublication < 0)
      bestPublication = block->height;
  }
  return bestPublication;
}

static PopRewardsBigDecimal getScoreMultiplierFromRelativeBlock(
    int relativeBlock, const PopRewardsParams& rewardParams) {
  if (relativeBlock < 0 ||
      relativeBlock >= rewardParams.relativeScoreLookupTable().size()) {
    return 0.0;
  }

  return rewardParams.relativeScoreLookupTable()[relativeBlock];
}

PopRewardsBigDecimal PopRewards::scoreFromEndorsements(
    const AltBlock& endorsedBlock) {
  PopRewardsBigDecimal totalScore = PopRewardsBigDecimal(0.0);

  ///TODO: in Java we were limiting the search in containingBlocks list.
  ///      Maybe we should do the same here as well.
  auto endorsements = erepo_->get(endorsedBlock.getHash());

  // we simply find the lowest VBK height in the endorsements
  int bestPublication = getBestPublicationHeight(endorsements, vbk_tree_);
  if (bestPublication < 0) return totalScore;

  for (const VbkEndorsement& e : endorsements) {
    auto* block = vbk_tree_->getBlockIndex(e.blockOfProof);
    if (block == nullptr) continue;
    int relativeHeight = block->height - bestPublication;
    totalScore += PopRewardsBigDecimal(
        getScoreMultiplierFromRelativeBlock(relativeHeight, rewardParams_));
  }
  return totalScore;
}

}  // namespace altintegration

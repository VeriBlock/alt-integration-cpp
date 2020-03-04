#include <veriblock/blockchain/pop/vbk_block_tree.hpp>

namespace VeriBlock {

std::vector<KeystoneContext> VbkBlockTree::getKeystoneContext(
    const std::vector<ProtoKeystoneContext>& chain) {
  std::vector<KeystoneContext> ret;

  std::transform(
      chain.begin(),
      chain.end(),
      std::back_inserter(ret),
      [this](const ProtoKeystoneContext& pkc) {
        int earliestEndorsementIndex = std::numeric_limits<int32_t>::max();
        for (const auto* btcIndex : pkc.referencedByBtcBlocks) {
          if (btcIndex == nullptr) {
            continue;
          }
          auto endorsementIndex = btcIndex->height;
          if (endorsementIndex >= earliestEndorsementIndex) {
            continue;
          }

          if (pkc.timestampOfEndorsedBlock < btcIndex->getBlockTime()) {
            earliestEndorsementIndex = endorsementIndex;
            continue;
          }

          // look at the future BTC blocks and set the
          // earliestEndorsementIndex to a future Bitcoin block
          auto btcBest = btc_.getBestChain();
          for (int adjustedEndorsementIndex = endorsementIndex + 1;
               adjustedEndorsementIndex <= btcBest.chainHeight();
               adjustedEndorsementIndex++) {
            // Ensure that the keystone's block time isn't later than the
            // block time of the Bitcoin block it's endorsed in
            auto* index = btcBest[adjustedEndorsementIndex];
            if (index == nullptr) {
              throw std::logic_error("unexpected nullptr in btc best chain");
            }
            if (pkc.timestampOfEndorsedBlock < index->getBlockTime()) {
              // Timestamp of VeriBlock block is lower than Bitcoin block,
              // set this as the adjusted index if another lower index has
              // not already been set
              if (adjustedEndorsementIndex < earliestEndorsementIndex) {
                earliestEndorsementIndex = adjustedEndorsementIndex;
              }

              // Always break; we found a valid Bitcoin index and any
              // future adjustedEndorsementIndex is going to be higher
              break;
            }  // end if
          }    // end for
        }      // end for

        return KeystoneContext{pkc.vbkBlockHeight, earliestEndorsementIndex};
      });

  return ret;
}

std::vector<ProtoKeystoneContext> VbkBlockTree::getProtoKeystoneContext(
    const Chain<VbkBlock>& chain) {
  std::vector<ProtoKeystoneContext> ret;
  auto* tip = chain.tip();
  if (tip == nullptr) {
    throw std::logic_error("unexpected nullptr - no tip in best chain");
  }

  auto highestPossibleEndorsedBlockHeaderHeight = tip->height;
  auto lastKeystone =
      highestKeystoneAtOrBefore(tip->height, VBK_KEYSTONE_INTERVAL);
  auto firstKeystone =
      firstKeystoneAfter(chain.first()->height, VBK_KEYSTONE_INTERVAL);

  // For each keystone, find the endorsements of itself and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto keystoneToConsider = firstKeystone;
       keystoneToConsider <= lastKeystone;
       keystoneToConsider =
           firstKeystoneAfter(keystoneToConsider, VBK_KEYSTONE_INTERVAL)) {
    ProtoKeystoneContext pkc(keystoneToConsider,
                             chain[keystoneToConsider]->height);

    auto highestConnectingBlock = highestBlockWhichConnectsKeystoneToPrevious(
        keystoneToConsider, VBK_KEYSTONE_INTERVAL);
    for (auto relevantEndorsedBlock = keystoneToConsider;
         relevantEndorsedBlock <= highestConnectingBlock &&
         relevantEndorsedBlock <= highestPossibleEndorsedBlockHeaderHeight;
         relevantEndorsedBlock++) {
      auto* index = chain[relevantEndorsedBlock];

      // chain must contain relevantEndorsedBlock
      assert(index != nullptr);

      // get all endorsements of this block
      auto endorsements = erepo_->get(index->getHash());
      // TODO: remove endorsements that do not belong to this chain
      //          ,
      //          [this, &chain](const VbkBlock::hash_t& hash) -> bool {
      //            auto* index = this->getBlockIndex(hash);
      //            return index != nullptr && chain.contains(index);
      //          }

      for (const auto& e : endorsements) {
        auto* ind = this->btc_.getBlockIndex(e.blockOfProof);
        pkc.referencedByBtcBlocks.insert(ind);
      }
    }

    ret.push_back(std::move(pkc));
  }

  return ret;
}

void VbkBlockTree::determineBestChain(Chain<block_t>& currentBest,
                                      BlockTree::index_t& indexNew) {
  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    return;
  }

  auto* forkIndex = currentBest.findFork(&indexNew);
  // this should never happen. if it is nullptr, it means that we passed
  // `indexNew` index which has no known prev block, which is possible only
  // after logic error, OR error in 'findFork'
  assert(forkIndex != nullptr);

  // last common keystone of two forks
  auto* forkKeystone = forkIndex->getAncestor(
      highestKeystoneAtOrBefore(forkIndex->height, VBK_KEYSTONE_INTERVAL));

  int result = 0;
  if (isCrossedKeystoneBoundary(
          forkKeystone->height, indexNew.height, VBK_KEYSTONE_INTERVAL) &&
      isCrossedKeystoneBoundary(forkKeystone->height,
                                currentBest.tip()->height,
                                VBK_KEYSTONE_INTERVAL)) {
    // [vbk fork point ... current tip]
    Chain<block_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());
    auto pkcCurrent = getProtoKeystoneContext(vbkCurrentSubchain);
    auto kcCurrent = getKeystoneContext(pkcCurrent);

    // [vbk fork point ... new block]
    Chain<block_t> vbkOther(forkKeystone->height, &indexNew);
    auto pkcOther = getProtoKeystoneContext(vbkOther);
    auto kcOther = getKeystoneContext(pkcOther);

    result = compare_(kcCurrent, kcOther);
  }

  if (result > 0) {
    // other chain won!
    auto prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    return addForkCandidate(prevTip, &indexNew);
  } else if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    return VbkTree::determineBestChain(currentBest, indexNew);
  } else {
    // existing chain is still the best
    addForkCandidate(&indexNew, indexNew.pprev);
    return;
  }
}
}  // namespace VeriBlock

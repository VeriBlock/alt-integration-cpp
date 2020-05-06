// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/keystone_util.hpp>

namespace altintegration {

namespace internal {

template <typename ProtectingBlockT>
struct ProtoKeystoneContext {
  int blockHeight;
  uint32_t timestampOfEndorsedBlock;

  ProtoKeystoneContext(int height, int time)
      : blockHeight(height), timestampOfEndorsedBlock(time) {}

  // A map of the Endorsement blocks which reference (endorse a block header
  // which is the represented block or references the represented block) to the
  // index of the earliest
  std::set<BlockIndex<ProtectingBlockT>*> referencedByBlocks;
};

struct KeystoneContext {
  int blockHeight;
  int firstBlockPublicationHeight;
};

template <typename ConfigType>
bool publicationViolatesFinality(int pubToCheck,
                                 int base,
                                 const ConfigType& config) {
  int diff = pubToCheck - base;
  return (int32_t)(diff - config.getFinalityDelay()) > 0;
}

template <typename ConfigType>
int getConsensusScoreFromRelativeBlockStartingAtZero(int64_t relativeBlock,
                                                     const ConfigType& config) {
  if (relativeBlock < 0 ||
      (size_t)relativeBlock >= config.getForkResolutionLookUpTable().size()) {
    return 0;
  }

  return config.getForkResolutionLookUpTable()[relativeBlock];
}

struct KeystoneContextList {
  std::vector<KeystoneContext> ctx;
  const int keystoneInterval;

  KeystoneContextList(const std::vector<KeystoneContext>& c,
                      int keystoneInt,
                      int finalityDelay)
      : keystoneInterval(keystoneInt) {
    size_t chop_index = c.size();

    if (c.size() > 1) {
      for (size_t i = 1; i < c.size(); ++i) {
        if ((c[i].firstBlockPublicationHeight -
             c[i - 1].firstBlockPublicationHeight) > finalityDelay) {
          chop_index = i;
          break;
        }
      }
    }

    ctx.insert(ctx.begin(), c.begin(), c.begin() + chop_index);
  }

  bool empty() const { return ctx.empty(); }

  int firstKeystone() const { return ctx[0].blockHeight; }

  int lastKeystone() const { return ctx[ctx.size() - 1].blockHeight; }

  const KeystoneContext* getKeystone(int blockNumber) const {
    if (!isKeystone(blockNumber, keystoneInterval)) {
      throw std::invalid_argument(
          "getKeystone can not be called with a non-keystone block number");
    }

    if (blockNumber < this->firstKeystone()) {
      return nullptr;
    }

    if (blockNumber > this->lastKeystone()) {
      return nullptr;
    }

    auto i = (blockNumber - firstKeystone()) / keystoneInterval;
    return &ctx.at(i);
  }
};

template <typename ProtectingBlockT, typename ProtectingChainParams>
std::vector<KeystoneContext> getKeystoneContext(
    const std::vector<ProtoKeystoneContext<ProtectingBlockT>>& chain,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& tree) {
  std::vector<KeystoneContext> ret;
  ret.reserve(chain.size());

  std::transform(
      chain.begin(),
      chain.end(),
      std::back_inserter(ret),
      [&tree](const ProtoKeystoneContext<ProtectingBlockT>& pkc) {
        int earliestEndorsementIndex = std::numeric_limits<int32_t>::max();
        for (const auto* btcIndex : pkc.referencedByBlocks) {
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

          // look at the future BTC blocks and set the--0
          // earliestEndorsementIndex to a future Bitcoin block
          auto best = tree.getBestChain();
          for (int adjustedEndorsementIndex = endorsementIndex + 1;
               adjustedEndorsementIndex <= best.chainHeight();
               adjustedEndorsementIndex++) {
            // Ensure that the keystone's block time isn't later than the
            // block time of the Bitcoin block it's endorsed in
            auto* index = best[adjustedEndorsementIndex];
            assert(index != nullptr);
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

        return KeystoneContext{pkc.blockHeight, earliestEndorsementIndex};
      });

  return ret;
}

template <typename ProtectedBlockT,
          typename ProtectingBlockT,
          typename ProtectingChainParams,
          typename ProtectedChainParams>
std::vector<ProtoKeystoneContext<ProtectingBlockT>> getProtoKeystoneContext(
    const Chain<BlockIndex<ProtectedBlockT>>& chain,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& tree,
    const ProtectedChainParams& config) {
  std::vector<ProtoKeystoneContext<ProtectingBlockT>> ret;

  auto ki = config.getKeystoneInterval();
  auto* tip = chain.tip();
  assert(tip != nullptr && "tip must not be nullptr");

  auto highestPossibleEndorsedBlockHeaderHeight = tip->height;
  auto lastKeystone = highestKeystoneAtOrBefore(tip->height, ki);
  auto firstKeystone = firstKeystoneAfter(chain.first()->height, ki);
  const auto allHashesInChain = chain.getAllHashesInChain();

  // For each keystone, find the endorsements of itself and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto keystoneToConsider = firstKeystone;
       keystoneToConsider <= lastKeystone;
       keystoneToConsider = firstKeystoneAfter(keystoneToConsider, ki)) {
    ProtoKeystoneContext<ProtectingBlockT> pkc(
        keystoneToConsider, chain[keystoneToConsider]->height);

    auto highestConnectingBlock =
        highestBlockWhichConnectsKeystoneToPrevious(keystoneToConsider, ki);
    for (auto relevantEndorsedBlock = keystoneToConsider;
         relevantEndorsedBlock <= highestConnectingBlock &&
         relevantEndorsedBlock <= highestPossibleEndorsedBlockHeaderHeight;
         relevantEndorsedBlock++) {
      auto* index = chain[relevantEndorsedBlock];

      // chain must contain relevantEndorsedBlock
      assert(index != nullptr);

      for (const auto* e : index->endorsedBy) {
        if (!allHashesInChain.count(e->containingHash)) {
          // do not count endorsement whose containingHash is not on the same
          // chain as 'endorsedHash'
          continue;
        }

        auto* ind = tree.getBlockIndex(e->blockOfProof);
        if (!tree.getBestChain().contains(ind)) {
          continue;
        }

        // include only endorsements that are on best chain of protecting chain,
        // and whose 'containingHash' is on the same chain as 'endorsedHash'
        pkc.referencedByBlocks.insert(ind);

      }  // end for
    }    // end for

    ret.push_back(std::move(pkc));
  }  // end for

  return ret;
}

template <typename ProtectedChainConfig>
int comparePopScoreImpl(const std::vector<KeystoneContext>& chainA,
                        const std::vector<KeystoneContext>& chainB,
                        const ProtectedChainConfig& config) {
  assert(config.getKeystoneInterval() > 0);
  auto ki = config.getKeystoneInterval();
  KeystoneContextList a(chainA, ki, config.getFinalityDelay());
  KeystoneContextList b(chainB, ki, config.getFinalityDelay());

  if (a.empty() && b.empty()) {
    return 0;
  }

  if (a.empty()) {
    // a empty, b is not
    return -1;
  }

  if (b.empty()) {
    // b is empty, a is not
    return 1;
  }

  int earliestKeystone = a.firstKeystone();
  if (earliestKeystone != b.firstKeystone()) {
    throw std::invalid_argument(
        "comparePopScore can not be called on two keystone lists that don't "
        "start at the same keystone index");
  }

  int latestKeystone = std::max(a.lastKeystone(), b.lastKeystone());

  bool aOutsideFinality = false;
  bool bOutsideFinality = false;
  int chainAscore = 0;
  int chainBscore = 0;
  for (int keystoneToCompare = earliestKeystone;
       keystoneToCompare <= latestKeystone;
       keystoneToCompare += ki) {
    auto* actx = a.getKeystone(keystoneToCompare);
    auto* bctx = b.getKeystone(keystoneToCompare);

    if (aOutsideFinality) {
      actx = nullptr;
    }

    if (bOutsideFinality) {
      bctx = nullptr;
    }

    if (actx == nullptr && bctx == nullptr) {
      break;
    }

    if (actx == nullptr) {
      chainBscore += config.getForkResolutionLookUpTable()[0];
      // Nothing added to chainA; it doesn't have an endorsed keystone at
      // this height (or any additional height) Optimization note: if chainB
      // score is greater than A here, we can exit early as A will not have
      // any additional points

      continue;
    }

    if (bctx == nullptr) {
      chainAscore += config.getForkResolutionLookUpTable()[0];
      // Optimization note: if chainA score is greater than B here, we can
      // exit early as B will not have any additional points
      continue;
    }

    int earliestPublicationA = actx->firstBlockPublicationHeight;
    int earliestPublicationB = bctx->firstBlockPublicationHeight;

    int earliestPublicationOfEither =
        std::min(earliestPublicationA, earliestPublicationB);

    chainAscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationA - earliestPublicationOfEither, config);
    chainBscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationB - earliestPublicationOfEither, config);

    if (publicationViolatesFinality(
            earliestPublicationA, earliestPublicationB, config)) {
      aOutsideFinality = true;
    }

    if (publicationViolatesFinality(
            earliestPublicationB, earliestPublicationA, config)) {
      bOutsideFinality = true;
    }

    if (aOutsideFinality && bOutsideFinality) {
      break;
    }
  }

  return chainAscore - chainBscore;
}

}  // namespace internal

template <typename ProtectedBlock,
          typename ProtectedParams,
          typename ProtectingBlockTree,
          typename ProtectedBlockTree>
struct PopAwareForkResolutionComparator {
  using protected_block_t = ProtectedBlock;
  using protected_block_hash_t = typename ProtectedBlock::hash_t;
  using protected_params_t = ProtectedParams;
  using protecting_params_t = typename ProtectingBlockTree::params_t;
  using protected_index_t = BlockIndex<protected_block_t>;
  using protecting_index_t = typename ProtectingBlockTree::index_t;
  using protecting_block_t = typename protecting_index_t::block_t;
  using context_t = typename protected_index_t::context_t;
  using endorsement_t = typename protected_block_t::endorsement_t;
  using protected_payloads_t = typename protected_index_t::payloads_t;
  using sm_t = PopStateMachine<ProtectingBlockTree,
                               ProtectedBlockTree,
                               BlockIndex<protected_block_t>,
                               protected_params_t>;

  PopAwareForkResolutionComparator(ProtectingBlockTree tree,
                                   const protecting_params_t& protectingParams,
                                   const protected_params_t& protectedParams)
      : ing_(std::move(tree)),
        protectedParams_(&protectedParams),
        protectingParams_(&protectingParams) {
    assert(protectedParams.getKeystoneInterval() > 0);
  }

  ProtectingBlockTree& getProtectingBlockTree() { return ing_; }
  const ProtectingBlockTree& getProtectingBlockTree() const { return ing_; }

  //! finds a path between current ed's best chain and 'to', and applies all
  //! commands in between
  bool setState(ProtectedBlockTree& ed,
                protected_index_t& to,
                ValidationState& state) {
    auto* currentActive = ed.getBestChain().tip();
    assert(currentActive && "should be bootstrapped");

    sm_t sm(ed, ing_);
    if (to.pprev && to.pprev == currentActive) {
      return sm.apply(*currentActive, to, state);
    }

    Chain<protected_index_t> chain(0, currentActive);
    auto* forkBlock = chain.findFork(&to);
    if (!forkBlock) {
      // we can't find 'to' in fork. maybe it is one of 'next' blocks?
      if (to.getAncestor(currentActive->height) != currentActive) {
        // can not find path to that block...
        return false;
      }

      return sm.apply(*currentActive, to, state);
    }

    sm.unapply(*currentActive, *forkBlock);
    return sm.apply(*forkBlock, to, state);
  }

  int comparePopScore(ProtectedBlockTree& ed,
                      protected_index_t& indexNew,
                      ValidationState& state) {
    assert(indexNew.isValid());

    auto currentBest = ed.getBestChain();
    auto bestTip = currentBest.tip();
    assert(bestTip);

    // this function is ALWAYS called on FORKS
    assert(indexNew.getAncestor(bestTip->height) != bestTip);

    auto ki = ed.getParams().getKeystoneInterval();
    const auto* forkKeystone =
        currentBest.findHighestKeystoneAtOrBeforeFork(&indexNew, ki);
    if (!forkKeystone) {
      // no fork keystone found. this can happen during bootstrap
      return 0;
    }

    bool AcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(forkKeystone->height, indexNew.height, ki);
    bool BcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(forkKeystone->height, bestTip->height, ki);
    if (!AcrossedKeystoneBoundary || !BcrossedKeystoneBoundary) {
      // chans are equal in terms of POP
      return 0;
    }

    // [vbk fork point keystone ... current tip]
    Chain<protected_index_t> chainA(forkKeystone->height, currentBest.tip());
    // [vbk fork point keystone... new block]
    Chain<protected_index_t> chainB(forkKeystone->height, &indexNew);

    // chains are not empty and chains start at the same block
    assert(chainA.first() != nullptr && chainA.first() == chainB.first());
    // first block is a keystone
    assert(isKeystone(chainA.first()->height,
                      protectedParams_->getKeystoneInterval()));

    // we ALWAYS compare currently applied chain (chainA) and other chain
    // (chainB)
    assert(chainA.tip() == bestTip);

    sm_t sm(ed, ing_, chainA.first()->height);

    // we are at chainA.
    // apply all payloads from chain B (both chains have same first block - fork
    // point at keystone, so exclude it during 'apply')
    if (!sm.apply(*chainB.first(), *chainB.tip(), state)) {
      // chain A has valid payloads, and chain B has invalid payloads
      // chain A is better

      // chain B has been unapplied already
      // invalid block in chain B has been invalidated already
      return 1;
    }

    // now tree contains payloads from both chains

    // rename
    const auto& filter1 = internal::getProtoKeystoneContext<protected_block_t,
                                                            protecting_block_t,
                                                            protecting_params_t,
                                                            protected_params_t>;
    const auto& filter2 =
        internal::getKeystoneContext<protecting_block_t, protecting_params_t>;

    // filter chainA
    auto pkcChain1 = filter1(chainA, ing_, *protectedParams_);
    auto kcChain1 = filter2(pkcChain1, ing_);

    // filter chainB
    auto pkcChain2 = filter1(chainB, ing_, *protectedParams_);
    auto kcChain2 = filter2(pkcChain2, ing_);

    // current tree contains both chains.
    int result = internal::comparePopScoreImpl<protected_params_t>(
        kcChain1, kcChain2, *protectedParams_);
    if (result >= 0) {
      // chain A remains best. unapply B. A remains applied
      sm.unapply(*chainB.tip(), *chainB.first());
    } else {
      // chain B is better. unapply A. B remains applied
      sm.unapply(*chainA.tip(), *chainA.first());
    }

    return result;
  }

  bool operator==(const PopAwareForkResolutionComparator& o) const {
    return ing_ == o.ing_;
  }

  std::string toPrettyString(size_t level = 0) const {
    std::ostringstream ss;
    std::string pad(level, ' ');
    ss << pad << "Comparator{\n";
    ss << pad << "{tree=\n";
    ss << ing_.toPrettyString(level + 2);
    ss << "}";
    return ss.str();
  }

 private:
  ProtectingBlockTree ing_;

  const protected_params_t* protectedParams_;
  const protecting_params_t* protectingParams_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

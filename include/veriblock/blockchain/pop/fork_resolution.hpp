// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/storage/payloads_index.hpp>

namespace altintegration {

namespace internal {

//! @private
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

//! @private
struct KeystoneContext {
  int blockHeight;
  int firstBlockPublicationHeight;
};

template <typename ConfigType>
bool publicationViolatesFinality(int pubToCheck,
                                 int base,
                                 const ConfigType& config) {
  int64_t diff = pubToCheck - base;
  return diff > config.getFinalityDelay();
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

//! @private
struct KeystoneContextList {
  std::vector<KeystoneContext> ctx;
  const int keystoneInterval;

  KeystoneContextList(const std::vector<KeystoneContext>& c, int keystoneInt)
      : ctx(c), keystoneInterval(keystoneInt) {
    VBK_ASSERT(
        std::is_sorted(ctx.begin(),
                       ctx.end(),
                       [](const KeystoneContext& a, const KeystoneContext& b) {
                         return a.blockHeight < b.blockHeight;
                       }));
  }

  size_t size() const { return ctx.size(); }

  bool empty() const { return ctx.empty(); }

  int firstKeystone() const { return ctx.front().blockHeight; }

  int lastKeystone() const { return ctx.back().blockHeight; }

  void chopAtKeystone(int keystoneToChop) {
    VBK_ASSERT_MSG(
        keystoneToChop >= firstKeystone(),
        "Cannot chop a keystone context to %d when its first keystone is %d",
        keystoneToChop,
        firstKeystone());
    VBK_ASSERT(isKeystone(keystoneToChop, keystoneInterval));

    if (ctx.empty()) {
      return;
    }

    if (keystoneToChop > lastKeystone()) {
      return;
    }

    // erase all keystone contexts >= 'keystoneToChop'
    auto newend = std::remove_if(
        ctx.begin(), ctx.end(), [keystoneToChop](const KeystoneContext& kc) {
          return kc.blockHeight >= keystoneToChop;
        });

    ctx.erase(newend, ctx.end());
  }

  const KeystoneContext* getKeystone(int blockNumber) const {
    VBK_ASSERT_MSG(
        isKeystone(blockNumber, keystoneInterval),
        "getKeystone can not be called with a non-keystone block number");

    if (blockNumber < this->firstKeystone()) {
      return nullptr;
    }

    if (blockNumber > this->lastKeystone()) {
      return nullptr;
    }

    auto it = std::find_if(
        ctx.begin(), ctx.end(), [blockNumber](const KeystoneContext& kc) {
          return kc.blockHeight == blockNumber;
        });

    if (it == ctx.end()) {
      return nullptr;
    }

    return &*it;
  }
};

template <typename ProtectingBlockT, typename ProtectingChainParams>
std::vector<KeystoneContext> getKeystoneContext(
    const std::vector<ProtoKeystoneContext<ProtectingBlockT>>& chain,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& tree) {
  std::vector<KeystoneContext> ret;
  ret.reserve(chain.size());
  for (const auto& pkc : chain) {
    int earliestEndorsementIndex = (std::numeric_limits<int32_t>::max)();
    for (const auto* btcIndex : pkc.referencedByBlocks) {
      if (btcIndex == nullptr) {
        continue;
      }

      auto endorsementIndex = btcIndex->getHeight();
      if (endorsementIndex >= earliestEndorsementIndex) {
        continue;
      }

      bool EnableTimeAdjustment = tree.getParams().EnableTimeAdjustment();
      if (!EnableTimeAdjustment ||
          pkc.timestampOfEndorsedBlock < btcIndex->getBlockTime()) {
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
        VBK_ASSERT(index != nullptr);
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

    ret.push_back(KeystoneContext{pkc.blockHeight, earliestEndorsementIndex});
  }

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
  VBK_ASSERT(tip != nullptr && "tip must not be nullptr");

  auto highestPossibleEndorsedBlockHeaderHeight = tip->getHeight();
  auto lastKeystone = highestKeystoneAtOrBefore(tip->getHeight(), ki);
  auto firstKeystone = firstKeystoneAfter(chain.first()->getHeight(), ki);
  const auto allHashesInChain = chain.getAllHashesInChain();

  // For each keystone, find the endorsements of itself and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto keystoneToConsider = firstKeystone;
       keystoneToConsider <= lastKeystone;
       keystoneToConsider = firstKeystoneAfter(keystoneToConsider, ki)) {
    ProtoKeystoneContext<ProtectingBlockT> pkc(
        keystoneToConsider, chain[keystoneToConsider]->getHeight());

    auto highestConnectingBlock =
        highestBlockWhichConnectsKeystoneToPrevious(keystoneToConsider, ki);
    for (auto relevantEndorsedBlock = keystoneToConsider;
         relevantEndorsedBlock <= highestConnectingBlock &&
         relevantEndorsedBlock <= highestPossibleEndorsedBlockHeaderHeight;
         relevantEndorsedBlock++) {
      auto* index = chain[relevantEndorsedBlock];

      // chain must contain relevantEndorsedBlock
      VBK_ASSERT(index != nullptr);

      for (const auto* e : index->endorsedBy) {
        if (!allHashesInChain.count(e->containingHash)) {
          // do not count endorsement whose containingHash is not on the same
          // chain as 'endorsedHash'
          continue;
        }

        auto* ind = tree.getBlockIndex(e->blockOfProof);
        VBK_ASSERT(ind != nullptr &&
                   "state corruption: could not find the block of proof of "
                   "an applied endorsement");
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
  VBK_ASSERT(config.getKeystoneInterval() > 0);
  auto ki = config.getKeystoneInterval();
  KeystoneContextList a(chainA, ki);
  KeystoneContextList b(chainB, ki);

  if (a.empty() && b.empty()) {
    return 0;
  }

  if (a.empty()) {
    // a is empty, b is not
    return -1;
  }

  if (b.empty()) {
    // b is empty, a is not
    return 1;
  }

  VBK_ASSERT(!chainA.empty());
  VBK_ASSERT(!chainB.empty());
  VBK_LOG_DEBUG(
      "Comparing POP scores of chains A(first=%d, tip=%d) "
      "and B(first=%d, tip=%d)",
      a.firstKeystone(),
      a.lastKeystone(),
      b.firstKeystone(),
      b.lastKeystone());

  int earliestKeystone = a.firstKeystone();
  VBK_ASSERT(earliestKeystone == b.firstKeystone());
  int latestKeystone = (std::max)(a.lastKeystone(), b.lastKeystone());

  // clang-format off
  // If either chain has a keystone the other chain is missing, chop the other chain
  //
  // Example:
  // Chain A has keystones VBK20:BTC100, VBK40:BTC101, VBK60:BTC103, VBK100:BTC104
  // Chain B has keystones VBK20:BTC100, VBK40:BTC101, VBK100:BTC102
  //
  // Chain A has keystone 60 but chain B does not, so Chain B is
  // chopped to VBK20:BTC100, VBK40:BTC101. Also chop a chain which contains a
  // keystone which violates the Bitcoin finality delay based on the previous
  // keystone in the chain.
  // clang-format on
  for (int keystoneToCompare = earliestKeystone;
       keystoneToCompare <= latestKeystone;
       keystoneToCompare += ki) {
    auto* A = a.getKeystone(keystoneToCompare);
    auto* B = b.getKeystone(keystoneToCompare);

    if (A == nullptr && B == nullptr) {
      // both chains are missing keystone at this height, ignore...
      continue;
    } else if (B == nullptr) {
      // chain B is missing the keystone but A isn't, chop B after this point
      b.chopAtKeystone(keystoneToCompare);
    } else if (A == nullptr) {
      // chain A is missing the keystone but B isn't, chop A after this point
      a.chopAtKeystone(keystoneToCompare);
    } else {
      // both chains have a keystone at this height
      // can't check first keystone for violating Bitcoin finality delay
      auto* Aprev = a.getKeystone(keystoneToCompare - ki);
      auto* Bprev = b.getKeystone(keystoneToCompare - ki);

      // Aprev could be null if keystoneToCompare is right after an allowed gap,
      // and Bitcoin finality delay violations cannot (and should not) be
      // checked
      if (Aprev != nullptr &&
          publicationViolatesFinality(A->firstBlockPublicationHeight,
                                      Aprev->firstBlockPublicationHeight,
                                      config)) {
        a.chopAtKeystone(keystoneToCompare);
      }

      // Bprev could be null if keystoneToCompare is right after an allowed gap,
      // and Bitcoin finality delay violations cannot (and should not) be
      // checked
      if (Bprev != nullptr &&
          publicationViolatesFinality(B->firstBlockPublicationHeight,
                                      Bprev->firstBlockPublicationHeight,
                                      config)) {
        b.chopAtKeystone(keystoneToCompare);
      }
    }
  }

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
      continue;
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
        (std::min)(earliestPublicationA, earliestPublicationB);

    chainAscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationA - earliestPublicationOfEither, config);
    chainBscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationB - earliestPublicationOfEither, config);

    if (publicationViolatesFinality(
            earliestPublicationA, earliestPublicationB, config)) {
      VBK_LOG_DEBUG("Chain A is outside finality");
      aOutsideFinality = true;
    }

    if (publicationViolatesFinality(
            earliestPublicationB, earliestPublicationA, config)) {
      VBK_LOG_DEBUG("Chain B is outside finality");
      bOutsideFinality = true;
    }

    if (aOutsideFinality && bOutsideFinality) {
      break;
    }
  }

  VBK_LOG_INFO("ChainA score=%d, ChainB score=%d", chainAscore, chainBscore);
  return chainAscore - chainBscore;
}

}  // namespace internal

//! @private
template <typename ProtectedBlock,
          typename ProtectedParams,
          typename ProtectingBlockTree,
          typename ProtectedBlockTree>
struct PopAwareForkResolutionComparator {
  using protected_block_t = ProtectedBlock;
  using protected_params_t = ProtectedParams;
  using protecting_params_t = typename ProtectingBlockTree::params_t;
  using protected_index_t = BlockIndex<protected_block_t>;
  using protecting_index_t = typename ProtectingBlockTree::index_t;
  using protecting_block_t = typename protecting_index_t::block_t;
  using endorsement_t = typename protected_index_t::endorsement_t;
  using sm_t = PopStateMachine<ProtectingBlockTree,
                               ProtectedBlockTree,
                               BlockIndex<protected_block_t>,
                               protected_params_t>;

  PopAwareForkResolutionComparator(std::shared_ptr<ProtectingBlockTree> tree,
                                   const protected_params_t& protectedParams,
                                   PayloadsProvider& payloadsProvider,
                                   PayloadsIndex& payloadsIndex)
      : ing_(std::move(tree)),
        protectedParams_(&protectedParams),
        payloadsProvider_(payloadsProvider),
        payloadsIndex_(payloadsIndex) {
    VBK_ASSERT(protectedParams.getKeystoneInterval() > 0);
  }

  ProtectingBlockTree& getProtectingBlockTree() { return *ing_; }
  const ProtectingBlockTree& getProtectingBlockTree() const { return *ing_; }

  //! finds a path between current ed's best chain and 'to', and applies all
  //! commands in between
  // atomic: either changes the state to 'to' or leaves it unchanged
  bool setState(ProtectedBlockTree& ed,
                protected_index_t& to,
                ValidationState& state,
                // if true, setState will be in "validation mode"
                // if invalid payloads found, mark it as invalid and continue
                bool continueOnInvalid = false) {
    auto* currentActive = ed.getBestChain().tip();
    VBK_ASSERT(currentActive && "should be bootstrapped");

    VBK_ASSERT_MSG(currentActive->getHeight() + 1 ==
                       ed.getRoot().getHeight() + ed.appliedBlockCount,
                   "the tree must have the best chain applied");

    if (currentActive == &to) {
      // already at this state
      return true;
    }

    auto guard = ing_->deferForkResolutionGuard();
    auto originalTip = ing_->getBestChain().tip();

    sm_t sm(ed, *ing_, payloadsProvider_, payloadsIndex_, continueOnInvalid);
    if (sm.setState(*currentActive, to, state)) {
      return true;
    }

    guard.overrideDeferredForkResolution(originalTip);
    return false;
  }

  /**
   * Compare the currently applied(best) and candidate chains
   * @return 0 if the chains are equal,
   *         positive if the current chain is better
   *         negative if the candidate chain is better
   */
  int comparePopScore(ProtectedBlockTree& ed,
                      protected_index_t& candidate,
                      ValidationState& state) {
    if (!candidate.isValid()) {
      // if the new block is known to be invalid, we always return "A is better"
      VBK_LOG_INFO("Candidate %s is invalid, the current chain wins",
                   candidate.toShortPrettyString());
      return 1;
    }
    auto currentBest = ed.getBestChain();
    auto bestTip = currentBest.tip();
    VBK_ASSERT(bestTip && "must be bootstrapped");

    if (bestTip == &candidate) {
      // we are comparing the best chain to itself
      return 1;
    }

    if (currentBest.contains(&candidate)) {
      VBK_LOG_INFO(
          "Candidate %s is part of the active chain, the current chain wins",
          candidate.toShortPrettyString());
      return 1;
    }

    auto originalProtectingTip = ing_->getBestChain().tip();

    // candidate is on top of our best tip
    if (candidate.getAncestor(bestTip->getHeight()) == bestTip) {
      VBK_LOG_DEBUG("Candidate is %d blocks ahead",
                    candidate.getHeight() - bestTip->getHeight());

      auto guard = ing_->deferForkResolutionGuard();

      sm_t sm(ed, *ing_, payloadsProvider_, payloadsIndex_);
      if (!sm.apply(*bestTip, candidate, state)) {
        // new chain is invalid. our current chain is definitely better.
        VBK_LOG_INFO("Candidate contains INVALID command(s): %s",
                     state.toString());
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return 1;
      }

      VBK_LOG_DEBUG("Candidate contains VALID commands, chain B wins");
      return -1;
    }

    VBK_LOG_INFO("Doing %s POP fork resolution. Best=%s, Candidate=%s",
                 protected_block_t::name(),
                 bestTip->toShortPrettyString(),
                 candidate.toShortPrettyString());

    auto ki = ed.getParams().getKeystoneInterval();
    const auto* fork = currentBest.findFork(&candidate);
    VBK_ASSERT_MSG(
        fork != nullptr,
        "state corruption: all blocks in a blocktree must form a tree, "
        "thus all pairs of chains must have a fork point: chainA=%s, chainB=%s",
        bestTip->toPrettyString(),
        candidate.toPrettyString());

    bool AcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(fork->getHeight(), bestTip->getHeight(), ki);
    bool BcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(fork->getHeight(), candidate.getHeight(), ki);
    if (!AcrossedKeystoneBoundary && !BcrossedKeystoneBoundary) {
      // chains are equal in terms of POP
      VBK_LOG_INFO(
          "Neither chain crossed a keystone boundary: chains are equal");
      return 0;
    }

    // [vbk fork point ... current tip]
    Chain<protected_index_t> chainA(fork->getHeight(), currentBest.tip());
    // [vbk fork point ... new block]
    Chain<protected_index_t> chainB(fork->getHeight(), &candidate);

    // chains are not empty and chains start at the same block
    VBK_ASSERT(chainA.first() != nullptr && chainA.first() == chainB.first());

    // we ALWAYS compare the currently applied chain (chainA) and the candidate
    // (chainB)
    VBK_ASSERT(chainA.tip() == bestTip);

    sm_t sm(ed, *ing_, payloadsProvider_, payloadsIndex_);

    // we are at chainA.
    // apply all payloads from chain B (both chains have same first block - the
    // fork point, so exclude it during 'apply')
    {
      auto guard = ing_->deferForkResolutionGuard();

      if (!sm.apply(chainB, state)) {
        // chain B has been unapplied and invalidated already
        VBK_LOG_INFO("Chain B contains INVALID payloads, Chain A wins (%s)",
                     state.toString());
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return 1;
      }
    }

    // now the tree contains payloads from both chains

    // rename
    const auto& filter1 = internal::getProtoKeystoneContext<protected_block_t,
                                                            protecting_block_t,
                                                            protecting_params_t,
                                                            protected_params_t>;
    const auto& filter2 =
        internal::getKeystoneContext<protecting_block_t, protecting_params_t>;

    // filter chainA
    auto pkcChain1 = filter1(chainA, *ing_, *protectedParams_);
    auto kcChain1 = filter2(pkcChain1, *ing_);

    // filter chainB
    auto pkcChain2 = filter1(chainB, *ing_, *protectedParams_);
    auto kcChain2 = filter2(pkcChain2, *ing_);

    // current tree contains both chains.
    int result = internal::comparePopScoreImpl<protected_params_t>(
        kcChain1, kcChain2, *protectedParams_);
    if (result >= 0) {
      // chain A remains the best one. unapply B and leave A applied
      auto guard = ing_->deferForkResolutionGuard();
      sm.unapply(chainB);
      guard.overrideDeferredForkResolution(originalProtectingTip);
      VBK_LOG_INFO("Chain A remains the best chain");
    } else {
      // chain B is better. unapply A and leave B applied
      auto guard = ing_->deferForkResolutionGuard();

      // if part of chainB has uncertain validity(never been applied before with
      // setState()), we have to unapply this part before unapplying chainA and
      // try to apply the unvalidated part of chainB without any payloads from
      // chainA to make sure it is fully valid
      auto& chainBValidFrom =
          sm.unapplyWhile(chainB, [](protected_index_t& index) -> bool {
            return !index.isValid(BLOCK_CAN_BE_APPLIED);
          });

      // unapply losing chain
      sm.unapply(chainA);

      // validate the unvalidated part of chainB
      if (!sm.apply(chainBValidFrom, *chainB.tip(), state)) {
        sm.unapply(chainBValidFrom, *chainB.first());
        bool success = sm.apply(*chainA.first(), *chainA.tip(), state);
        VBK_ASSERT_MSG(
            success,
            "state corruption: chainA as a former best chain should be valid");
        VBK_LOG_INFO("Chain B is invalid when applied alone. Chain A wins");
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return 1;
      }

      VBK_LOG_INFO("Chain B wins");
    }

    return result;
  }

  std::string toPrettyString(size_t level = 0) const {
    std::string pad(level, ' ');
    return fmt::sprintf("%sComparator{\n%s{tree=\n%s}}",
                        pad,
                        pad,
                        ing_->toPrettyString(level + 2));
  }

 private:
  std::shared_ptr<ProtectingBlockTree> ing_;

  const protected_params_t* protectedParams_;
  PayloadsProvider& payloadsProvider_;
  PayloadsIndex& payloadsIndex_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

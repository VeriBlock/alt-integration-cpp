// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/blockchain/blocktree.hpp>
#include <veriblock/pop/blockchain/chain_slice.hpp>
#include <veriblock/pop/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/pop/finalizer.hpp>
#include <veriblock/pop/keystone_util.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/storage/payloads_index.hpp>
#include <veriblock/pop/trace.hpp>

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
  std::set<const BlockIndex<ProtectingBlockT>*> referencedByBlocks;
};

//! @private
// FIXME: we don't use block height
// FIXME: this is really asking to be converted to an optional(but it isn't
// available in C++14) or a pseudo-optional where firstBlockPublicationHeight ==
// NO_ENDORSEMENT is the check for the missing value
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

constexpr int NO_ENDORSEMENT = (std::numeric_limits<int32_t>::max)();

template <typename ProtectingBlockT, typename ProtectingChainParams>
KeystoneContext getKeystoneContext(
    const ProtoKeystoneContext<ProtectingBlockT>& pkc,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& tree) {
  VBK_TRACE_ZONE_SCOPED;

  int earliestEndorsementIndex = NO_ENDORSEMENT;
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
        pkc.timestampOfEndorsedBlock < btcIndex->getTimestamp()) {
      earliestEndorsementIndex = endorsementIndex;
      continue;
    }

    // look at the future BTC blocks and set the
    // earliestEndorsementIndex to a future Bitcoin block
    auto best = tree.getBestChain();
    for (int adjustedEndorsementIndex = endorsementIndex + 1;
         adjustedEndorsementIndex <= best.chainHeight();
         adjustedEndorsementIndex++) {
      // Ensure that the keystone's block time isn't later than the
      // block time of the Bitcoin block it's endorsed in
      auto* index = best[adjustedEndorsementIndex];
      VBK_ASSERT(index != nullptr);
      if (pkc.timestampOfEndorsedBlock < index->getTimestamp()) {
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

  return {pkc.blockHeight, earliestEndorsementIndex};
}

template <typename ProtectedBlockT,
          typename ProtectingBlockT,
          typename ProtectingChainParams,
          typename ProtectedChainParams,
          typename ProtectedBlockTree>
ProtoKeystoneContext<ProtectingBlockT> getProtoKeystoneContext(
    int keystoneToConsider,
    const ChainSlice<BlockIndex<ProtectedBlockT>>& chain,
    const ProtectedBlockTree& ed,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& ing,
    const ProtectedChainParams& config) {
  VBK_TRACE_ZONE_SCOPED;
  VBK_LOG_DEBUG("Entered method with keystoneToConsider={}",
                keystoneToConsider);

  auto ki = config.getKeystoneInterval();
  auto* tip = chain.tip();
  VBK_ASSERT(tip != nullptr && "chain must not be empty");

  auto highestConnectingBlock =
      highestBlockWhichConnectsKeystoneToPrevious(keystoneToConsider, ki);
  auto highestPossibleEndorsedBlockHeaderHeight = tip->getHeight();
  auto highestEndorsedBlock = std::min(
      highestConnectingBlock, highestPossibleEndorsedBlockHeaderHeight);

  ProtoKeystoneContext<ProtectingBlockT> pkc(
      keystoneToConsider, chain[keystoneToConsider]->getTimestamp());

  // Find the endorsements of the keystone block and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto relevantEndorsedBlock = keystoneToConsider;
       relevantEndorsedBlock <= highestEndorsedBlock;
       relevantEndorsedBlock++) {
    auto* index = chain[relevantEndorsedBlock];

    // chain must contain relevantEndorsedBlock
    VBK_ASSERT(index != nullptr);

    for (const auto* e : index->getEndorsedBy()) {
      VBK_ASSERT(e != nullptr);
      auto* containingBlock = ed.getBlockIndex(e->containingHash);
      VBK_ASSERT_MSG(containingBlock != nullptr,
                     "state corruption: could not find the containing block of "
                     "an applied endorsement");

      if (!chain.contains(containingBlock)) {
        // do not count endorsement whose containingHash is not on the same
        // chain as 'endorsedHash'
        continue;
      }

      auto* ind = ing.getBlockIndex(e->blockOfProof);
      VBK_ASSERT(ind != nullptr &&
                 "state corruption: could not find the block of proof of "
                 "an applied endorsement");
      if (!ing.getBestChain().contains(ind)) {
        continue;
      }

      // include only endorsements that are on best chain of protecting chain,
      // and whose 'containingHash' is on the same chain as 'endorsedHash'
      pkc.referencedByBlocks.insert(ind);

    }  // end for
  }    // end for

  return pkc;
}

//! @private
// AKA KeystoneContextList
template <typename ProtectedBlockT,
          typename ProtectingBlockT,
          typename ProtectingChainParams,
          typename ProtectedChainParams,
          typename ProtectedBlockTree>
struct ReducedPublicationView {
  using protecting_block_t = ProtectingBlockT;
  using protected_block_t = ProtectedBlockT;
  using protecting_chain_params_t = ProtectingChainParams;
  using protected_chain_params_t = ProtectedChainParams;
  using protected_chain_t = ChainSlice<BlockIndex<protected_block_t>>;
  using protected_tree_t = ProtectedBlockTree;
  using protecting_tree_t =
      BlockTree<protecting_block_t, protecting_chain_params_t>;

  const protected_chain_params_t& config;
  const int keystoneInterval;
  const protected_chain_t chain;

  const protected_tree_t& protectedTree;
  const protecting_tree_t& protectingTree;

  const int firstKeystoneHeight;
  const int lastKeystoneHeight;

  KeystoneContext currentKeystoneContext;  // FIXME: UGLY

  ReducedPublicationView(const protected_chain_t _chain,
                         const protected_chain_params_t& _config,
                         const protected_tree_t& _ed,
                         const protecting_tree_t& _ing)
      : config(_config),
        keystoneInterval(config.getKeystoneInterval()),
        chain(_chain),
        protectedTree(_ed),
        protectingTree(_ing),
        firstKeystoneHeight(firstKeystoneAfterBlockIndex(chain.first())),
        lastKeystoneHeight(highestKeystoneAtOrBeforeBlockIndex(chain.tip())) {
    VBK_ASSERT(keystoneInterval > 0);
  }

  const protected_chain_params_t& getConfig() const { return config; }

  size_t size() const {
    return blockHeightToKeystoneNumber(lastKeystone(), keystoneInterval) -
           blockHeightToKeystoneNumber(firstKeystone(), keystoneInterval) + 1;
  }

  bool empty() const { return size() == 0; }

  int firstKeystone() const { return firstKeystoneHeight; }

  int lastKeystone() const { return lastKeystoneHeight; }

  int nextKeystoneAfter(int keystoneHeight) const {
    VBK_ASSERT(isKeystone(keystoneHeight, keystoneInterval));
    return keystoneHeight + keystoneInterval;
  }

  const KeystoneContext* operator[](int blockHeight) {
    return getKeystone(blockHeight);
  }

  const KeystoneContext* getKeystone(int blockHeight) {
    VBK_ASSERT_MSG(
        isKeystone(blockHeight, keystoneInterval),
        "getKeystone can not be called with a non-keystone block height");

    if (blockHeight < firstKeystone() || blockHeight > lastKeystone()) {
      return nullptr;
    }

    // FIXME: there's no need to store the intermediate list of blocks and thus
    // no need for ProtoKeystoneContext entity
    auto pkc = getProtoKeystoneContext(
        blockHeight, chain, protectedTree, protectingTree, config);

    currentKeystoneContext = getKeystoneContext(pkc, protectingTree);
    return &currentKeystoneContext;
  }

  int32_t firstKeystoneAfterBlockIndex(typename protected_chain_t::index_t* blockIndex) {
    VBK_ASSERT_MSG(blockIndex, "Protected tree should be bootstrapped");
    return firstKeystoneAfter(blockIndex->getHeight(), keystoneInterval);
  }

  int32_t highestKeystoneAtOrBeforeBlockIndex(
      typename protected_chain_t::index_t* blockIndex) {
    VBK_ASSERT_MSG(blockIndex, "Protected tree should be bootstrapped");
    return highestKeystoneAtOrBefore(blockIndex->getHeight(),
                                     keystoneInterval);
  }
};

template <typename PublicationView>
int comparePopScoreImpl(PublicationView& a, PublicationView& b) {
  VBK_TRACE_ZONE_SCOPED;

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

  auto& config = a.getConfig();
  VBK_ASSERT(&config == &b.getConfig());  // FIXME: comparing pointers is UGLY
  // clang-format off
  // If either chain has a keystone the other chain is missing, the other chain
  // goes outside finality.
  //
  // Example:
  // Chain A has keystones VBK20:BTC100, VBK40:BTC101, VBK60:BTC103, VBK100:BTC104
  // Chain B has keystones VBK20:BTC100, VBK40:BTC101, VBK100:BTC102
  //
  // Chain A has keystone 60 but chain B does not, so Chain B is
  // chopped to VBK20:BTC100, VBK40:BTC101.

  // Also, the chain goes out of finality if it contains a keystone which violates
  // the Bitcoin finality delay based on the previous keystone in the chain.
  // clang-format on

  bool aOutsideFinality = false;
  bool bOutsideFinality = false;
  int chainAscore = 0;
  int chainBscore = 0;
  int previousPublicationA = NO_ENDORSEMENT;
  int previousPublicationB = NO_ENDORSEMENT;

  for (int keystoneToCompare = earliestKeystone;
       keystoneToCompare <= latestKeystone;
       keystoneToCompare = a.nextKeystoneAfter(keystoneToCompare)) {
    auto* actx = aOutsideFinality ? nullptr : a.getKeystone(keystoneToCompare);
    auto* bctx = bOutsideFinality ? nullptr : b.getKeystone(keystoneToCompare);

    int earliestPublicationA =
        actx == nullptr ? NO_ENDORSEMENT : actx->firstBlockPublicationHeight;
    int earliestPublicationB =
        bctx == nullptr ? NO_ENDORSEMENT : bctx->firstBlockPublicationHeight;

    // if keystoneToCompare is right after a gap, publicationViolatesFinality
    // will return false, thus we only compare the finality delay violations
    // between endorsements that exist
    if (actx != nullptr &&
        publicationViolatesFinality(
            earliestPublicationA, previousPublicationA, config)) {
      aOutsideFinality = true;
      actx = nullptr;
      VBK_LOG_DEBUG(
          "Chain A is outside finality: the gap between adjacent keystone "
          "endorsements is too large");
    }
    previousPublicationA = earliestPublicationA;

    if (bctx != nullptr &&
        publicationViolatesFinality(
            earliestPublicationB, previousPublicationB, config)) {
      bOutsideFinality = true;
      bctx = nullptr;
      VBK_LOG_DEBUG(
          "Chain B is outside finality: the gap between adjacent keystone "
          "endorsements is too large");
    }
    previousPublicationB = earliestPublicationB;

    // if both chains are missing keystone at this height, ignore
    // if both are outside finality, break
    if (actx == nullptr && bctx == nullptr) {
      if (aOutsideFinality && bOutsideFinality) {
        break;
      }
      continue;
    }

    if (actx == nullptr) {
      chainBscore += config.getForkResolutionLookUpTable()[0];
      // Nothing added to chainA; it doesn't have an endorsed keystone at
      // this height (or any additional height)

      // chain A is missing the keystone that chain B isn't
      aOutsideFinality = true;

      if (chainBscore > chainAscore) {
        // exit early as the score of A will stay the same
        break;
      }

      continue;
    }

    if (bctx == nullptr) {
      chainAscore += config.getForkResolutionLookUpTable()[0];

      // chain B is missing the keystone that chain A isn't
      bOutsideFinality = true;

      if (chainAscore > chainBscore) {
        // exit early as the score of chain B will stay the same
        break;
      }

      continue;
    }

    // both chains have a keystone at this height

    int earliestPublicationOfEither =
        (std::min)(earliestPublicationA, earliestPublicationB);

    chainAscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationA - earliestPublicationOfEither, config);
    chainBscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationB - earliestPublicationOfEither, config);

    if (publicationViolatesFinality(
            earliestPublicationA, earliestPublicationB, config)) {
      VBK_LOG_DEBUG("Chain A is outside finality: way behind chain B");
      aOutsideFinality = true;
    }

    if (publicationViolatesFinality(
            earliestPublicationB, earliestPublicationA, config)) {
      VBK_LOG_DEBUG("Chain B is outside finality: way behind chain A");
      bOutsideFinality = true;
    }
  }

  VBK_LOG_DEBUG("ChainA score=%d, ChainB score=%d", chainAscore, chainBscore);
  return chainAscore - chainBscore;
}

}  // namespace internal

enum class PopFrOutcome {
  UNKNOWN,
  CANDIDATE_IS_TIP,
  CANDIDATE_INVALID_CHAIN,
  CANDIDATE_INVALID_PAYLOADS,
  CANDIDATE_PART_OF_ACTIVE_CHAIN,
  CANDIDATE_IS_TIP_SUCCESSOR,
  TIP_IS_FINAL,
  BOTH_DONT_CROSS_KEYSTONE_BOUNDARY,
  CANDIDATE_INVALID_INDEPENDENTLY,
  HIGHER_POP_SCORE,
};

const char* popFrOutcomeToString(PopFrOutcome value);

//! @private
template <typename ProtectedBlock,
          typename ProtectedParams,
          typename ProtectingBlockTree,
          typename ProtectedBlockTree>
struct PopAwareForkResolutionComparator {
  using protected_block_t = ProtectedBlock;
  using protected_params_t = ProtectedParams;
  using protected_block_tree_t = ProtectedBlockTree;
  using protecting_params_t = typename ProtectingBlockTree::params_t;
  using protected_index_t = BlockIndex<protected_block_t>;
  using protecting_index_t = typename ProtectingBlockTree::index_t;
  using protecting_block_t = typename protecting_index_t::block_t;
  using endorsement_t = typename protected_index_t::endorsement_t;
  using sm_t = PopStateMachine<ProtectingBlockTree,
                               ProtectedBlockTree,
                               BlockIndex<protected_block_t>,
                               protected_params_t>;

  using reduced_publication_view_t =
      internal::ReducedPublicationView<protected_block_t,
                                       protecting_block_t,
                                       protecting_params_t,
                                       protected_params_t,
                                       protected_block_tree_t>;

  PopAwareForkResolutionComparator(ProtectedBlockTree& ed,
                                   std::shared_ptr<ProtectingBlockTree> ing,
                                   const protected_params_t& protectedParams,
                                   PayloadsStorage& payloadsProvider,
                                   PayloadsIndex& payloadsIndex)
      : ed_(ed),
        ing_(std::move(ing)),
        protectedParams_(protectedParams),
        payloadsProvider_(payloadsProvider),
        payloadsIndex_(payloadsIndex),
        sm_(ed_, *ing_, payloadsIndex_) {
    VBK_ASSERT(protectedParams.getKeystoneInterval() > 0);
  }

  ProtectingBlockTree& getProtectingBlockTree() { return *ing_; }
  const ProtectingBlockTree& getProtectingBlockTree() const { return *ing_; }

  //! finds a path between ed_'s best chain and 'to', and applies all
  //! commands in between
  //! @invariant atomic: either changes the state to 'to' or leaves it unchanged
  //! @return true if the state change was successful, false otherwise
  bool setState(protected_index_t& to, ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("Entered method");

    auto* currentActive = ed_.getBestChain().tip();
    VBK_ASSERT(currentActive != nullptr && "should be bootstrapped");

    VBK_ASSERT_MSG(
        currentActive->getHeight() + 1 ==
            ed_.getRoot().getHeight() +
                (typename protected_block_t::height_t)ed_.appliedBlockCount,
        "the tree must have the best chain applied");

    if (currentActive == &to) {
      // already at this state
      return true;
    }

    auto guard = ing_->deferForkResolutionGuard();
    auto originalTip = ing_->getBestChain().tip();

    if (sm_.setState(*currentActive, to, state)) {
      return true;
    }

    guard.overrideDeferredForkResolution(originalTip);
    return false;
  }

  /**
   * Compare the currently applied(best) and candidate chains
   * @return First: 0 if the chains are equal,
   *                positive if the current chain is better
   *                negative if the candidate chain is better
   *         Second: reason explaining result.
   */
  std::pair<int, PopFrOutcome> comparePopScore(protected_index_t& candidate,
                                               ValidationState& state) {
    VBK_TRACE_ZONE_SCOPED;
    VBK_LOG_DEBUG("Entered method");

    if (!candidate.isValid()) {
      // if the new block is known to be invalid, we always return "A is better"
      VBK_LOG_DEBUG("Candidate %s is invalid, the current chain wins",
                    candidate.toShortPrettyString());
      return {1, PopFrOutcome::CANDIDATE_INVALID_CHAIN};
    }
    auto currentBest = ed_.getBestChain();
    auto bestTip = currentBest.tip();
    VBK_ASSERT(bestTip && "must be bootstrapped");

    if (bestTip == &candidate) {
      // we are comparing the best chain to itself
      return {1, PopFrOutcome::CANDIDATE_IS_TIP};
    }

    if (bestTip->finalized && candidate.getHeight() <= bestTip->getHeight()) {
      // finalized blocks can not be reorganized
      return {1, PopFrOutcome::TIP_IS_FINAL};
    }

    if (currentBest.contains(&candidate)) {
      VBK_LOG_DEBUG(
          "Candidate %s is part of the active chain, the current chain wins",
          candidate.toShortPrettyString());
      return {1, PopFrOutcome::CANDIDATE_PART_OF_ACTIVE_CHAIN};
    }

    auto originalProtectingTip = ing_->getBestChain().tip();

    // candidate is on top of our best tip
    if (candidate.getAncestor(bestTip->getHeight()) == bestTip) {
      VBK_LOG_DEBUG("Candidate is %d blocks ahead",
                    candidate.getHeight() - bestTip->getHeight());

      auto guard = ing_->deferForkResolutionGuard();

      if (!sm_.apply(*bestTip, candidate, state)) {
        // new chain is invalid. our current chain is definitely better.
        VBK_LOG_DEBUG("Candidate contains INVALID command(s): %s",
                      state.toString());
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return {1, PopFrOutcome::CANDIDATE_INVALID_PAYLOADS};
      }

      VBK_LOG_DEBUG("Candidate contains VALID commands, chain B wins");
      return {-1, PopFrOutcome::CANDIDATE_IS_TIP_SUCCESSOR};
    }

    VBK_LOG_DEBUG("Doing %s POP fork resolution. Best=%s, Candidate=%s",
                  protected_block_t::name(),
                  bestTip->toShortPrettyString(),
                  candidate.toShortPrettyString());

    auto ki = ed_.getParams().getKeystoneInterval();
    const auto* fork = findFork(currentBest, &candidate);
    VBK_ASSERT_MSG(
        fork != nullptr,
        "state corruption: all blocks in a blocktree must form a tree, "
        "thus all pairs of chains must have a fork point: chainA=%s, chainB=%s",
        bestTip->toPrettyString(),
        candidate.toPrettyString());

    // if block next to fork is finalized, we don't do FR, since it can not be
    // reorganized.
    auto* nextToFork = currentBest[fork->getHeight() + 1];
    if (nextToFork != nullptr && nextToFork->finalized) {
      // block `fork+1` is on active chain, and ancestor of currentBest.
      // which means that `fork+1` can not be reorganized and candidate will
      // never win.
      return {1, PopFrOutcome::TIP_IS_FINAL};
    }

    // perform multi-keystone PoP fork resolution
    bool AcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(fork->getHeight(), bestTip->getHeight(), ki);
    bool BcrossedKeystoneBoundary =
        isCrossedKeystoneBoundary(fork->getHeight(), candidate.getHeight(), ki);
    if (!AcrossedKeystoneBoundary && !BcrossedKeystoneBoundary) {
      // chains are equal in terms of POP
      VBK_LOG_DEBUG(
          "Neither chain crossed a keystone boundary: chains are equal");
      return {0, PopFrOutcome::BOTH_DONT_CROSS_KEYSTONE_BOUNDARY};
    }

    // [vbk fork point ... current tip]
    ChainSlice<protected_index_t> chainA(currentBest, fork->getHeight());
    // [vbk fork point ... new block]
    Chain<protected_index_t> chainB(fork->getHeight(), &candidate);

    // chains are not empty and chains start at the same block
    VBK_ASSERT(chainA.first() != nullptr && chainA.first() == chainB.first());

    // we ALWAYS compare the currently applied chain (chainA) and the candidate
    // (chainB)
    VBK_ASSERT(chainA.tip() == bestTip);

    // we are at chainA.
    // apply all payloads from chain B (both chains have same first block - the
    // fork point, so exclude it during 'apply')
    {
      auto guard = ing_->deferForkResolutionGuard();

      if (!sm_.apply(chainB, state)) {
        // chain B has been unapplied and invalidated already
        VBK_LOG_DEBUG("Chain B contains INVALID payloads, Chain A wins (%s)",
                      state.toString());
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return {1, PopFrOutcome::CANDIDATE_INVALID_PAYLOADS};
      }
    }

    // now the tree contains payloads from both chains
    auto reducedPublicationViewA =
        reduced_publication_view_t(chainA, protectedParams_, ed_, *ing_);
    auto reducedPublicationViewB =
        reduced_publication_view_t(chainB, protectedParams_, ed_, *ing_);

    int result = internal::comparePopScoreImpl(reducedPublicationViewA,
                                               reducedPublicationViewB);
    if (result >= 0) {
      // chain A remains the best one. unapply B and leave A applied
      auto guard = ing_->deferForkResolutionGuard();
      sm_.unapply(chainB);
      guard.overrideDeferredForkResolution(originalProtectingTip);
      VBK_LOG_DEBUG("Chain A remains the best chain");
    } else {
      // chain B is better. unapply A and leave B applied
      auto guard = ing_->deferForkResolutionGuard();

      // if part of chainB has uncertain validity(never been applied before with
      // setState()), we have to unapply this part before unapplying chainA and
      // try to apply the unvalidated part of chainB without any payloads from
      // chainA to make sure it is fully valid
      auto& chainBValidFrom =
          sm_.unapplyWhile(chainB, [](protected_index_t& index) -> bool {
            return !index.isValid(BLOCK_CAN_BE_APPLIED);
          });

      // unapply losing chain
      sm_.unapply(chainA);

      // validate the unvalidated part of chainB
      if (!sm_.apply(chainBValidFrom, *chainB.tip(), state)) {
        sm_.unapply(chainBValidFrom, *chainB.first());
        bool success = sm_.apply(*chainA.first(), *chainA.tip(), state);
        VBK_ASSERT_MSG(
            success,
            "state corruption: chainA as a former best chain should be valid");
        VBK_LOG_DEBUG("Chain B is invalid when applied alone. Chain A wins");
        guard.overrideDeferredForkResolution(originalProtectingTip);
        return {1, PopFrOutcome::CANDIDATE_INVALID_INDEPENDENTLY};
      }

      VBK_LOG_DEBUG("Chain B wins");
    }

    return {result, PopFrOutcome::HIGHER_POP_SCORE};
  }

  std::string toPrettyString(size_t level = 0) const {
    std::string pad(level, ' ');
    return format("{}Comparator{{\n{}{{tree=\n{}}}}}",
                  pad,
                  pad,
                  ing_->toPrettyString(level + 2));
  }

 private:
  ProtectedBlockTree& ed_;
  std::shared_ptr<ProtectingBlockTree> ing_;

  const protected_params_t& protectedParams_;
  PayloadsStorage& payloadsProvider_;
  PayloadsIndex& payloadsIndex_;
  sm_t sm_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <functional>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/storage/endorsement_repository.hpp>
#include <veriblock/storage/payloads_repository.hpp>

namespace altintegration {

template <typename EndorsementBlockType>
struct ProtoKeystoneContext {
  int blockHeight;
  uint32_t timestampOfEndorsedBlock;

  ProtoKeystoneContext(int height, int time)
      : blockHeight(height), timestampOfEndorsedBlock(time) {}

  // A map of the Endorsement blocks which reference (endorse a block header
  // which is the represented block or references the represented block) to the
  // index of the earliest
  std::set<BlockIndex<EndorsementBlockType>*> referencedByBlocks;
};

struct KeystoneContext {
  int vbkBlockHeight;
  int firstBtcBlockPublicationHeight;
};

namespace {

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
  const std::vector<KeystoneContext>& ctx;
  const int keystoneInterval;

  KeystoneContextList(const std::vector<KeystoneContext>& c, int keystoneInt)
      : ctx{c}, keystoneInterval(keystoneInt) {}

  bool empty() const { return ctx.empty(); }

  int firstKeystone() const { return ctx[0].vbkBlockHeight; }

  int lastKeystone() const { return ctx[ctx.size() - 1].vbkBlockHeight; }

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

template <typename ProtectedChainConfig>
int comparePopScore(const std::vector<KeystoneContext>& chainA,
                    const std::vector<KeystoneContext>& chainB,
                    const ProtectedChainConfig& config) {
  assert(config.getKeystoneInterval() > 0);
  KeystoneContextList a(chainA, config.getKeystoneInterval());
  KeystoneContextList b(chainB, config.getKeystoneInterval());

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
       keystoneToCompare += config.getKeystoneInterval()) {
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

    int earliestPublicationA = actx->firstBtcBlockPublicationHeight;
    int earliestPublicationB = bctx->firstBtcBlockPublicationHeight;

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

/// @invariant atomic: if we have encounter invalid payloads, p and pState
/// will be in original state after function exits
template <typename ProtectingBlockTree, typename ProtectedIndex>
void unapply(
    ProtectingBlockTree& p,
    ProtectedIndex** pState,
    const ProtectedIndex& to,
    const std::function<std::vector<Payloads>(const ProtectedIndex& index)>&
        getPayloadsForBlock) {
  if (&to == *pState) {
    // already at this state
    return;
  }

  Chain<ProtectedIndex> chain(0, *pState);
  auto* forkPoint = chain.findFork(&to);
  auto* current = chain.tip();
  while (current && current != forkPoint) {
    // unapply payloads
    for (const auto& payloads : getPayloadsForBlock(current)) {
      removePayloads(p, payloads);
    }
    *pState = current;
    current = current->pprev;
  }
}

/// @invariant atomic: if we have encounter invalid payloads, p and pState
/// will be in original state after function exits
template <typename ProtectingBlockTree, typename ProtectedIndex>
bool apply(
    ProtectingBlockTree& p,
    ProtectedIndex** pState,
    const ProtectedIndex& to,
    ValidationState& state,
    const std::function<std::vector<Payloads>(const ProtectedIndex& index)>&
        getPayloadsForBlock) {
  if (&to == *pState) {
    // already at this state
    return true;
  }

  Chain<ProtectedIndex> fork(0, &to);

  auto* originalPstate = *pState;
  auto* current = *pState;
  // move forward from forkPoint to "to" and apply payloads in between

  // exclude fork point itself
  current = fork.next(current);

  while (current) {
    for (const auto& payloads : getPayloadsForBlock(current)) {
      if (addPayloads(p, payloads, state)) {
        // TODO: rollback added payloads
        *pState = originalPstate;
        return state.addStackFunction("PopAwareForkResolution::apply");
      }

      *pState = current;
    }

    if (current != &to) {
      current = fork.next(current);
    } else {
      break;
    }
  }

  assert(*pState == &to);

  // TODO: commit added payloads

  return true;
}

/// @invariant atomic: if we have encounter invalid payloads, p and pState
/// will be in original state after function exits
template <typename ProtectingBlockTree, typename ProtectedIndex>
bool unapplyAndApply(
    ProtectingBlockTree& p,
    ProtectedIndex** pState,
    const ProtectedIndex& to,
    ValidationState& state,
    const std::function<std::vector<Payloads>(const ProtectedIndex& index)>&
        getPayloadsForBlock) {
  if (&to == *pState) {
    // already at this state
    return true;
  }
  unapply(p, pState, to, getPayloadsForBlock);

  Chain<ProtectedIndex> chain(0, &to);
  if (chain.contains(*pState)) {
    // do not apply payloads as "to" is in current chain and no new payloads
    // will be added
    return true;
  }

  return apply(p, pState, to, state, getPayloadsForBlock);
}

}  // namespace

template <typename ProtectedBlock,
          typename ProtectedParams,
          typename ProtectingBlockTree,
          typename EndorsementT>
struct PopAwareForkResolutionComparator {
  using protected_params_t = ProtectedParams;
  using protecting_params_t = typename ProtectingBlockTree::params_t;
  using protected_index_t = BlockIndex<ProtectedBlock>;
  using protecting_index_t = typename ProtectingBlockTree::index_t;

  PopAwareForkResolutionComparator(
      const EndorsementRepository<EndorsementT>& e,
      const PayloadsRepository<ProtectedBlock, Payloads>& p,
      const protected_params_t& protectedParams,
      const protecting_params_t& protectingParams)
      : tree_(protectingParams), e_(e), p_(p), config_(protectedParams) {
    assert(protectedParams.getKeystoneInterval() > 0);
  }

  ProtectingBlockTree& getProtectingBlockTree() { return tree_; }
  const ProtectingBlockTree& getProtectingBlockTree() const { return tree_; }

  bool setState(const protected_index_t& index, ValidationState& state) {
    return unapplyAndApply(tree_,
                           treeState_,
                           index,
                           state,
                           [this](const protected_index_t& block) {
                             return this->p_.get(block.getHash());
                           });
  }

  int operator()(const Chain<protected_index_t>& chainA,
                 const Chain<protected_index_t>& chainB) {
    // chains are not empty and chains start at the same block
    assert(chainA.first() != nullptr && chainA.first() == chainB.first());
    // first block is a keystone
    assert(isKeystone(chainA.first().height, config_.getKeystoneInterval()));

    auto getPayloads = [this](const BlockIndex<ProtectedBlock>& index) {
      return this->p_.get(index.header.getHash());
    };

    ValidationState state;
    ProtectingBlockTree temp = getProtectingBlockTree();
    auto* tempState = treeState_;
    // try set current state to chain A
    if (!unapplyAndApply(temp, &tempState, chainA.tip(), state, getPayloads)) {
      // failed - try set state to chain B
      if (!unapplyAndApply(
              temp, &tempState, chainB.tip(), state, getPayloads)) {
        // failed - this should never happen.
        // during fork resolution we should always compare one valid chain to
        // another potentially valid (may be invalid, we'll find out this during
        // fork resolution).
        throw std::logic_error(
            "PopAwareForkResolution::comparePopScore both chains have invalid "
            "payloads");
      }

      // chain B is better, because chain A contains bad payloads, and chain B
      // doesn't
      return -1;
    }

    // 'temp' now corresponds to chain A.
    // apply all payloads from chain B (both chains have same first block - fork
    // point at keystone)
    if (!apply(temp, &tempState, *chainB.tip(), state, getPayloads)) {
      // chain A has valid payloads, and chain B has invalid payloads
      // chain A is better
      return 1;
    }

    // now 'temp' contains payloads from both chains

    /// filter chainA
    auto pkcChain1 = getProtoKeystoneContext(
        chainA, tree_, config_, getValidEndorsements(chainA));
    auto kcChain1 = getKeystoneContext(pkcChain1, tree_);

    /// filter chainB
    auto pkcChain2 = getProtoKeystoneContext(
        chainB, tree_, config_, getValidEndorsements(chainB));
    auto kcChain2 = getKeystoneContext(pkcChain2, tree_);

    return comparePopScore(chainA, chainB, config_);
  }

 private:
  std::function<std::vector<EndorsementT>(const protected_index_t&)>
  getValidEndorsements(const Chain<protected_index_t>& chain) {
    return [&](const protected_index_t& index) -> std::vector<EndorsementT> {
      // all endorsements of block 'index'
      auto allEndorsements = e_.get(index->getHash());

      // efficiently remove endorsements that do not belong to active
      // protect(ing/ed) chains
      allEndorsements.erase(std::remove_if(
          allEndorsements.begin(),
          allEndorsements.end(),
          [&](const EndorsementT& e) {
            // 1. check that containing block is on best chain.
            auto* edindex = tree_.getBlockIndex(e.containing);

            // 2. check that blockOfProof is still on best chain.
            auto* ingindex = tree_.getBlockIndex(e.blockOfProof);

            // endorsed block is guaranteed to be ancestor of containing block
            // therefore, do not do check here
            return chain.contains(edindex) &&
                   tree_.getBestChain().contains(ingindex);
          },
          allEndorsements.end()));

      return allEndorsements;
    };
  }

 private:
  ProtectingBlockTree tree_;
  protected_index_t* treeState_;
  const EndorsementRepository<EndorsementT>& e_;
  const PayloadsRepository<ProtectedBlock, Payloads>& p_;
  const protected_params_t& config_;
};

template <typename ProtectingBlockT, typename ProtectingChainParams>
std::vector<KeystoneContext> getKeystoneContext(
    const std::vector<ProtoKeystoneContext<ProtectingBlockT>>& chain,
    const BlockTree<ProtectingBlockT, ProtectingChainParams>& tree) {
  std::vector<KeystoneContext> ret;

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

        return KeystoneContext{pkc.blockHeight, earliestEndorsementIndex};
      });

  return ret;
}

template <typename EndorsedBlockType,
          typename EndorsementBlockType,
          typename EndorsementChainParams,
          typename EndorsementType,
          typename ConfigType>
std::vector<ProtoKeystoneContext<EndorsementBlockType>> getProtoKeystoneContext(
    const Chain<BlockIndex<EndorsedBlockType>>& chain,
    const BlockTree<EndorsementBlockType, EndorsementChainParams>& tree,
    const ConfigType& config,
    const std::function<std::vector<EndorsementType>(
        const BlockIndex<EndorsedBlockType>&)>& getValidEndorsements) {
  std::vector<ProtoKeystoneContext<EndorsementBlockType>> ret;

  auto ki = config.getKeystoneInterval();
  auto* tip = chain.tip();
  assert(tip != nullptr && "tip must not be nullptr");

  auto highestPossibleEndorsedBlockHeaderHeight = tip->height;
  auto lastKeystone = highestKeystoneAtOrBefore(tip->height, ki);
  auto firstKeystone = firstKeystoneAfter(chain.first()->height, ki);

  // For each keystone, find the endorsements of itself and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto keystoneToConsider = firstKeystone;
       keystoneToConsider <= lastKeystone;
       keystoneToConsider = firstKeystoneAfter(keystoneToConsider, ki)) {
    ProtoKeystoneContext<EndorsementBlockType> pkc(
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

      // get all endorsements of this block that are on the same chain as that
      // block
      for (const auto& e : getValidEndorsements(*index)) {
        auto* ind = tree.getBlockIndex(e.blockOfProof);
        pkc.referencedByBlocks.insert(ind);
      }
    }

    ret.push_back(std::move(pkc));
  }

  return ret;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

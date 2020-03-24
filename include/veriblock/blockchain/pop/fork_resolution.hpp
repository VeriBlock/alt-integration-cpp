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

}  // namespace

template <typename ProtectedBlockTree,
          typename ProtectingBlockTree,
          typename EndorsementT>
struct ComparePopScore {
  using protected_params_t = typename ProtectedBlockTree::params_t;
  using protected_block_t = typename ProtectedBlockTree::block_t;
  using protecting_params_t = typename ProtectingBlockTree::params_t;
  using protected_index_t = typename ProtectedBlockTree::index_t;
  using protecting_index_t = typename ProtectingBlockTree::index_t;

  explicit ComparePopScore(const EndorsementRepository<EndorsementT>& e,
                           const protected_params_t& config)
      : e_(e), config_(config) {
    assert(config.getKeystoneInterval() > 0);
  }

  int operator()(const ProtectingBlockTree& ing,
                 const Chain<protected_index_t>& chainA,
                 const Chain<protected_index_t>& chainB) {
    assert(chainA.first() != nullptr && chainA.first() == chainB.first());

    auto getValidEndorsements =
        [&](const protected_index_t& index) -> std::vector<EndorsementT> {
      // all endorsements of block 'index'
      auto allEndorsements = e_.get(index->getHash());

      // efficiently remove endorsements that do not belong to active
      // protect(ing/ed) chains
      allEndorsements.erase(std::remove_if(
          allEndorsements.begin(),
          allEndorsements.end(),
          [&](const EndorsementT& e) {
            // 1. check that containing block is on best chain.
            auto* edindex = ing.getBlockIndex(e.containing);

            // 2. check that blockOfProof is still on best chain.
            auto* ingindex = ing.getBlockIndex(e.blockOfProof);

            // endorsed block is guaranteed to be ancestor of containing block
            // therefore, do not do check here
            return (chainA.contains(edindex) || chainB.contains(edindex)) &&
                   ing.getBestChain().contains(ingindex);
          },
          allEndorsements.end()));
    };

    /// filter chainA
    auto pkcChain1 =
        getProtoKeystoneContext(chainA, ing, config_, getValidEndorsements);
    auto kcChain1 = getKeystoneContext(pkcChain1, ing);

    /// filter chainB
    auto pkcChain2 =
        getProtoKeystoneContext(chainB, ing, config_, getValidEndorsements);
    auto kcChain2 = getKeystoneContext(pkcChain2, ing);

    return comparePopScore(chainA, chainB, config_);
  }

 private:
  const EndorsementRepository<EndorsementT>& e_;
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
    std::function<std::vector<EndorsementType>(
        const BlockIndex<EndorsedBlockType>&)> getEndorsements) {
  std::vector<ProtoKeystoneContext<EndorsementBlockType>> ret;
  auto* tip = chain.tip();
  assert(tip != nullptr && "tip must not be nullptr");

  auto highestPossibleEndorsedBlockHeaderHeight = tip->height;
  auto lastKeystone =
      highestKeystoneAtOrBefore(tip->height, config.getKeystoneInterval());
  auto firstKeystone =
      firstKeystoneAfter(chain.first()->height, config.getKeystoneInterval());

  // For each keystone, find the endorsements of itself and other blocks which
  // reference it, and look at the earliest Bitcoin block that any of those
  // endorsements are contained within.
  for (auto keystoneToConsider = firstKeystone;
       keystoneToConsider <= lastKeystone;
       keystoneToConsider = firstKeystoneAfter(keystoneToConsider,
                                               config.getKeystoneInterval())) {
    ProtoKeystoneContext<EndorsementBlockType> pkc(
        keystoneToConsider, chain[keystoneToConsider]->height);

    auto highestConnectingBlock = highestBlockWhichConnectsKeystoneToPrevious(
        keystoneToConsider, config.getKeystoneInterval());
    for (auto relevantEndorsedBlock = keystoneToConsider;
         relevantEndorsedBlock <= highestConnectingBlock &&
         relevantEndorsedBlock <= highestPossibleEndorsedBlockHeaderHeight;
         relevantEndorsedBlock++) {
      auto* index = chain[relevantEndorsedBlock];

      // chain must contain relevantEndorsedBlock
      assert(index != nullptr);

      // get all endorsements of this block that are on the same chain as that
      // block
      for (const auto& e : getEndorsements(*index)) {
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

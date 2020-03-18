#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <memory>
#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace AltIntegrationLib {

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

template <typename ConfigType>
int comparePopScore(const std::vector<KeystoneContext>& chainA,
                    const std::vector<KeystoneContext>& chainB,
                    const ConfigType& config) {
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

template <typename ConfigType>
struct ComparePopScore {
  explicit ComparePopScore(const ConfigType& config) : config_(config) {
    assert(config.getKeystoneInterval() > 0);
  }

  int operator()(const std::vector<KeystoneContext>& chainA,
                 const std::vector<KeystoneContext>& chainB) {
    return comparePopScore(chainA, chainB, config_);
  }

 private:
  const ConfigType& config_;
};

template <typename EndorsementBlockType, typename EndorsementChainParams>
std::vector<KeystoneContext> getKeystoneContext(
    const std::vector<ProtoKeystoneContext<EndorsementBlockType>>& chain,
    const BlockTree<EndorsementBlockType, EndorsementChainParams>&
        endorsementBlockTree) {
  std::vector<KeystoneContext> ret;

  std::transform(
      chain.begin(),
      chain.end(),
      std::back_inserter(ret),
      [&endorsementBlockTree](
          const ProtoKeystoneContext<EndorsementBlockType>& pkc) {
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
          auto btcBest = endorsementBlockTree.getBestChain();
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
    const Chain<EndorsedBlockType>& chain,
    const BlockTree<EndorsementBlockType, EndorsementChainParams>&
        endorsementBlockTree,
    std::shared_ptr<EndorsementRepository<EndorsementType>> endorsementRepo,
    const ConfigType& config) {
  std::vector<ProtoKeystoneContext<EndorsementBlockType>> ret;
  auto* tip = chain.tip();
  if (tip == nullptr) {
    throw std::logic_error("unexpected nullptr - no tip in best chain");
  }

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

      // get all endorsements of this block
      auto endorsements = endorsementRepo->get(index->getHash());
      // TODO: remove endorsements that do not belong to this chain
      //          ,
      //          [this, &chain](const VbkBlock::hash_t& hash) -> bool {
      //            auto* index = this->getBlockIndex(hash);
      //            return index != nullptr && chain.contains(index);
      //          }

      for (const auto& e : endorsements) {
        auto* ind = endorsementBlockTree.getBlockIndex(e.blockOfProof);
        pkc.referencedByBlocks.insert(ind);
      }
    }

    ret.push_back(std::move(pkc));
  }

  return ret;
}

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

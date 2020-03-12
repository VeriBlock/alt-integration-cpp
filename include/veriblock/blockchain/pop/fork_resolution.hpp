#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <memory>
#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/keystone_util.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace VeriBlock {

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

struct ComparePopScore {
  explicit ComparePopScore(uint32_t keystoneInt)
      : keystoneInterval(keystoneInt) {
    assert(keystoneInterval > 0);
  }

  int operator()(const std::vector<KeystoneContext>& chainA,
                 const std::vector<KeystoneContext>& chainB);

 private:
  int keystoneInterval;
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
      [&endorsementBlockTree](const ProtoKeystoneContext<BtcBlock>& pkc) {
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
          typename EndorsementType>
std::vector<ProtoKeystoneContext<EndorsementBlockType>> getProtoKeystoneContext(
    const Chain<EndorsedBlockType>& chain,
    const BlockTree<EndorsementBlockType, EndorsementChainParams>&
        endorsementBlockTree,
    std::shared_ptr<EndorsementRepository<EndorsementType>> endorsementRepo) {
  std::vector<ProtoKeystoneContext<EndorsementBlockType>> ret;
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
    ProtoKeystoneContext<EndorsementBlockType> pkc(
        keystoneToConsider, chain[keystoneToConsider]->height);

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

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

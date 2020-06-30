// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

template <typename Block>
void determineBestChain(Chain<Block>& currentBest, BlockIndex<Block>& indexNew);

template <typename Block, typename ChainParams>
uint32_t getNextWorkRequired(const BlockIndex<Block>& prevBlock,
                             const Block& block,
                             const ChainParams& params);

template <typename Block>
ArithUint256 getBlockProof(const Block& block);

template <typename Block>
int64_t getMedianTimePast(const BlockIndex<Block>& prev);

template <typename Block>
bool checkBlockTime(const BlockIndex<Block>& prev,
                    const Block& block,
                    ValidationState& state);

template <typename Block, typename ChainParams>
bool contextuallyCheckBlock(const BlockIndex<Block>& prev,
                            const Block& block,
                            ValidationState& state,
                            const ChainParams& params);

template <typename EdTree>
struct CheckEndorsementResult {
  // whether this endorsement is valid or not
  bool valid = true;
  // a pointer to containing block
  typename EdTree::index_t* containing = nullptr;
  // a pointer to endorsed block
  typename EdTree::index_t* endorsed = nullptr;
};

template <typename EndorsementT, typename EdTree, typename IngTree>
CheckEndorsementResult<EdTree> contextuallyCheckEndorsement(
    const EndorsementT& e_,
    const EdTree& ed_,
    const IngTree& ing_,
    ValidationState& state) {
  using protected_block_t = typename EdTree::block_t;
  using protected_index_t = typename EdTree::index_t;
  using endorsement_t = EndorsementT;
  using result_t = CheckEndorsementResult<EdTree>;

  // return this triplet
  result_t result;

  auto* containing = ed_.getBlockIndex(e_.containingHash);
  if (!containing) {
    result.valid = state.Invalid(
        protected_block_t::name() + "-no-containing",
        fmt::sprintf("Can not find containing block in endorsement=%s",
                     e_.toPrettyString()));
    return result;
  }

  // endorsement validity window
  auto window = ed_.getParams().getEndorsementSettlementInterval();
  auto minHeight = (std::max)(containing->height - window, 0);
  Chain<protected_index_t> chain(minHeight, containing);

  auto endorsedHeight = e_.endorsedHeight;
  if (containing->height - endorsedHeight > window) {
    result.valid = state.Invalid(protected_block_t::name() + "-expired",
                                 "Endorsement expired");
    return result;
  }

  auto* endorsed = chain[endorsedHeight];
  if (!endorsed) {
    result.valid =
        state.Invalid(protected_block_t::name() + "-no-endorsed-block",
                      "No block found on endorsed block height");
    return result;
  }

  if (endorsed->getHash() != e_.endorsedHash) {
    result.valid = state.Invalid(
        protected_block_t::name() + "-block-differs",
        fmt::sprintf(
            "Endorsed block is on a different chain. Expected: %s, got %s",
            endorsed->toShortPrettyString(),
            HexStr(e_.endorsedHash)));
    return result;
  }

  auto& id = e_.id;
  auto endorsed_it =
      std::find_if(endorsed->endorsedBy.rbegin(),
                   endorsed->endorsedBy.rend(),
                   [&id](endorsement_t* p) { return p->id == id; });
  if (endorsed_it != endorsed->endorsedBy.rend()) {
    // found duplicate
    result.valid = state.Invalid(
        protected_block_t ::name() + "-duplicate",
        fmt::sprintf("Can not add endorsement=%s to block=%s, because we "
                     "found block endorsed by it in %s",
                     e_.toPrettyString(),
                     containing->toShortPrettyString(),
                     endorsed->toShortPrettyString()));
    return result;
  }

  auto* blockOfProof = ing_.getBlockIndex(e_.blockOfProof);
  if (!blockOfProof) {
    result.valid = state.Invalid(
        protected_block_t::name() + "-block-of-proof-not-found",
        fmt::sprintf("Can not find block of proof in SP Chain (%s)",
                     HexStr(e_.blockOfProof)));
    return result;
  }

  auto* duplicate =
      findBlockContainingEndorsement(chain, containing, e_, window);
  if (duplicate) {
    // found duplicate
    result.valid = state.Invalid(
        protected_block_t ::name() + "-duplicate",
        fmt::sprintf("Can not add endorsement=%s to block=%s, because we "
                     "found its duplicate in block %s",
                     e_.toPrettyString(),
                     containing->toShortPrettyString(),
                     duplicate->toShortPrettyString()));
    return result;
  }

  result.valid = true;
  result.containing = containing;
  result.endorsed = endorsed;

  return result;
}

template <typename Block>
void removePayloadsFromIndex(BlockIndex<Block>& index, const CommandGroup& cg);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

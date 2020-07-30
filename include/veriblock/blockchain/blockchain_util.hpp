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

template <typename Block, typename Storage>
void removePayloadsFromIndex(Storage& storage,
                             BlockIndex<Block>& index,
                             const CommandGroup& cg);

template <typename ProtectedBlockTree>
bool recoverEndorsedBy(ProtectedBlockTree& ed_,
                       Chain<typename ProtectedBlockTree::index_t>& chain,
                       typename ProtectedBlockTree::index_t& toRecover,
                       ValidationState& state) {
  std::vector<std::function<void()>> actions;
  auto& containingEndorsements = toRecover.getContainingEndorsements();
  actions.reserve(containingEndorsements.size());

  for (const auto& p : containingEndorsements) {
    auto& id = p.first;
    auto& e = *p.second;

    if (id != e.id) {
      return state.Invalid(
          "bad-id", fmt::format("Key={}, Id={}", HexStr(id), HexStr(e.id)));
    }

    auto* endorsed = ed_.getBlockIndex(e.endorsedHash);
    if (!endorsed) {
      return state.Invalid(
          "no-endorsed",
          fmt::format("Can not find endorsed block in {}", e.toPrettyString()));
    }

    if (chain[endorsed->getHeight()] == nullptr ||
        chain[endorsed->getHeight()]->getHash() != e.endorsedHash ||
        endorsed->getHash() != e.endorsedHash) {
      return state.Invalid(
          "bad-endorsed",
          fmt::format("Endorsed block does not match {}", e.toPrettyString()));
    }

    if (e.containingHash != toRecover.getHash()) {
      return state.Invalid("bad-containing",
                           fmt::format("Containing block does not match {}",
                                       e.toPrettyString()));
    }

    // make sure it is accessible in lambda
    auto& endorsedPtr = *endorsed;

    // delay execution. this ensures atomic changes - if any of endorsemens fail
    // validation, no 'action' is actually executed.
    actions.push_back([&] {
      auto& by = endorsedPtr.endorsedBy;
      VBK_ASSERT(std::find(by.rbegin(), by.rend(), &e) == by.rend() &&
                 "same endorsement is added to endorsedBy second time");
      by.push_back(&e);
    });
  }

  // all actions have been validated
  // commit changes
  for (auto& f : actions) {
    f();
  }

  return true;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

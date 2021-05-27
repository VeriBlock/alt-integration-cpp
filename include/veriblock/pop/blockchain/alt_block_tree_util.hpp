// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP

#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/publication_data.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "alt_block_tree.hpp"
#include "block_index.hpp"
#include "blockchain_util.hpp"
#include "command_group.hpp"

namespace altintegration {

/** Find stateless and stateful duplicates
 * Reorder the container items, moving all found duplicates to the back
 * @return an iterator to the first duplicate
 */
template <typename Element, typename Container>
/*iterator*/ auto findDuplicates(BlockIndex<AltBlock>& index,
                                 Container& payloads,
                                 AltBlockTree& tree)
    -> decltype(payloads.end()) {
  const auto startHeight = tree.getParams().getBootstrapBlock().height;
  // don't look for duplicates in index itself
  Chain<BlockIndex<AltBlock>> chain(startHeight, index.pprev);
  std::unordered_set<std::vector<uint8_t>> ids;

  const auto& storage = tree.getPayloadsIndex();
  auto duplicates =
      std::remove_if(payloads.begin(), payloads.end(), [&](const Element& p) {
        const auto id = getIdVector(p);
        // ensure existing blocks do not contain this id
        for (const auto& hash : storage.getContainingAltBlocks(id)) {
          const auto* candidate = tree.getBlockIndex(hash);
          if (chain.contains(candidate)) {
            // duplicate in 'candidate'
            return true;
          }
        }

        // ensure ids are unique within `pop`
        bool inserted = ids.insert(id).second;
        return !inserted;
      });

  return duplicates;
}

//! @private
template <>
bool checkBlockTime(const BlockIndex<AltBlock>& prev,
                    const AltBlock& block,
                    ValidationState& state,
                    const AltChainParams& params);

//! @private
template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    AltBlockTree& tree,
    const PopData& pop,
    const AltBlock::hash_t& containinghash);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP

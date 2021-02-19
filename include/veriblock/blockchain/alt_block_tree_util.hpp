// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP

#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/publication_data.hpp>
#include <veriblock/validation_state.hpp>

#include "block_index.hpp"
#include "blockchain_util.hpp"
#include "command_group.hpp"

namespace altintegration {

struct VbkBlockTree;
struct AltChainParams;
struct AltBlockTree;

template <>
bool checkBlockTime(const BlockIndex<AltBlock>& prev,
                    const AltBlock& block,
                    ValidationState& state,
                    const AltChainParams& params);

template <>
std::vector<CommandGroup> payloadsToCommandGroups(
    AltBlockTree& tree,
    const PopData& pop,
    const AltBlock::hash_t& containinghash);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_UTIL_HPP

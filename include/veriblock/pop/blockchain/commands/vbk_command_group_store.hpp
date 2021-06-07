// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_COMMAND_GROUP_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_VBK_COMMAND_GROUP_PROVIDER_HPP

#include <veriblock/pop/blockchain/blockchain_util.hpp>
#include <veriblock/pop/blockchain/command_group.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/storage/payloads_provider.hpp>

namespace altintegration {

struct VbkBlockTree;

/**
 * A wrapper for the payload store that constructs command objects.
 */

struct VbkCommandGroupStore {
  using command_groups_t = std::vector<std::unique_ptr<CommandGroup>>;

  VbkCommandGroupStore(VbkBlockTree& tree, PayloadsStorage& payloadStore)
      : _tree(tree), _payloadStore(payloadStore) {}

  /**
   * Get the command group for the given payload in the given block.
   * @param[in] block load from this block
   * @param[in] id the id of the payload
   * @param[out] state will be set to Error if the payload fails to validate
   * @return a reference to the requested command group that is valid until the
   * next getCommand(s) call or null if the payload fails to validate
   */
  std::unique_ptr<CommandGroup> getCommand(const BlockIndex<VbkBlock>& block,
                                           const VTB::id_t& id,
                                           ValidationState& state);

  /**
   * Get command groups for a particular block.
   * @param[in] block load from this block
   * @param[out] state will be set to Error if one of the payloads fails to
   * validate
   * @return a vector of references to commands that are valid until the next
   * getCommand(s) call or null if one or more payloads fails to validate
   */
  std::unique_ptr<command_groups_t> getCommands(
      const BlockIndex<VbkBlock>& block, ValidationState& state);

 private:
  VbkBlockTree& _tree;
  PayloadsStorage& _payloadStore;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_COMMAND_GROUP_PROVIDER_HPP

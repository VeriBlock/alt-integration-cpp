// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALTINTEGRATION_HPP
#define ALTINTEGRATION_ALTINTEGRATION_HPP

#include "veriblock/alt-util.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/storage/pop_storage.hpp"
#include "veriblock/config.hpp"

namespace altintegration {

struct Altintegration {
  static std::shared_ptr<AltTree> create(const Config& config, PayloadsStorage& storage) {
    config.validate();
    auto tree = std::make_shared<altintegration::AltTree>(
        *config.alt, *config.vbk.params, *config.btc.params, storage);
    ValidationState state;

    // first, bootstrap BTC
    if (config.btc.blocks.empty()) {
      tree->btc().bootstrapWithGenesis(state);
      VBK_ASSERT(state.IsValid());
    } else {
      tree->btc().bootstrapWithChain(
          config.btc.startHeight, config.btc.blocks, state);
      VBK_ASSERT(state.IsValid());
    }

    // then, bootstrap VBK
    if (config.vbk.blocks.empty()) {
      tree->vbk().bootstrapWithGenesis(state);
      VBK_ASSERT(state.IsValid());
    } else {
      tree->vbk().bootstrapWithChain(
          config.vbk.startHeight, config.vbk.blocks, state);
      VBK_ASSERT(state.IsValid());
    }

    tree->bootstrap(state);
    return tree;
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALTINTEGRATION_HPP

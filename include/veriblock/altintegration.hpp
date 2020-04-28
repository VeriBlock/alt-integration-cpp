// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALTINTEGRATION_HPP
#define ALTINTEGRATION_ALTINTEGRATION_HPP

#include "veriblock/alt-util.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/config.hpp"

namespace altintegration {

struct Altintegration {
  static AltTree create(const Config& config) {
    config.validate();
    AltTree tree(*config.alt, *config.vbk.params, *config.btc.params);
    ValidationState state;

    // first, bootstrap BTC
    if (config.btc.blocks.empty()) {
      tree.btc().bootstrapWithGenesis(state);
      assert(state.IsValid());
    } else {
      tree.btc().bootstrapWithChain(
          config.btc.startHeight, config.btc.blocks, state);
      assert(state.IsValid());
    }

    // then, bootstrap VBK
    if (config.vbk.blocks.empty()) {
      tree.vbk().bootstrapWithGenesis(state);
      assert(state.IsValid());
    } else {
      tree.vbk().bootstrapWithChain(
          config.vbk.startHeight, config.vbk.blocks, state);
      assert(state.IsValid());
    }

    tree.bootstrap(state);

    return tree;
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALTINTEGRATION_HPP

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_CONFIG_HPP___
#define ALTINTEGRATION_CONFIG_HPP___

#include <utility>
#include <vector>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/stateless_validation.hpp>

namespace altintegration {

struct Config {
  template <typename Block, typename ChainParams>
  struct Bootstrap {
    //! height of the first bootstrap block
    int32_t startHeight = 0;

    //! a contiguous list of blocks.
    //! if empty, bootstrapWithGenesis will be used
    //! has to have at least `params->numBlocksForBootstrap()` blocks
    std::vector<Block> blocks;

    std::shared_ptr<ChainParams> params;

    static Bootstrap create(int32_t start,
                            const std::vector<std::string>& hexblocks,
                            std::shared_ptr<ChainParams> params) {
      Bootstrap b;
      b.startHeight = start;
      b.params = std::move(params);

      b.blocks.reserve(hexblocks.size());
      std::transform(
          hexblocks.begin(),
          hexblocks.end(),
          std::back_inserter(b.blocks),
          [](const std::string& hexblock) { return Block::fromHex(hexblock); });

      return b;
    }
  };

  std::shared_ptr<AltChainParams> alt;
  Bootstrap<BtcBlock, BtcChainParams> btc;
  Bootstrap<VbkBlock, VbkChainParams> vbk;

  //! helper, which converts array of hexstrings (blocks) into "Bootstrap" type
  void setBTC(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<BtcChainParams> params) {
    this->btc = decltype(btc)::create(start, hexblocks, std::move(params));
  }

  //! helper, which converts array of hexstrings (blocks) into "Bootstrap" type
  void setVBK(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<VbkChainParams> params) {
    this->vbk = decltype(vbk)::create(start, hexblocks, std::move(params));
  }

  //! small helper to check whether config is valid
  //! @throws std::invalid_argument with a message, if something is wrong.
  void validate() const {
    if (!alt) {
      throw std::invalid_argument("Config: altchain == nullptr");
    }

    if (!btc.params) {
      throw std::invalid_argument("Config: btc params == nullptr");
    }

    if (!vbk.params) {
      throw std::invalid_argument("Config: vbk params == nullptr");
    }

    ValidationState state;
    if (!vbk.blocks.empty()) {
      if (vbk.blocks.size() < vbk.params->numBlocksForBootstrap()) {
        throw std::invalid_argument(
            "Config: you have to specify at least " +
            std::to_string(vbk.params->numBlocksForBootstrap()) +
            " VBK blocks to bootstrap.");
      }

      if (!checkVbkBlocks(vbk.blocks, state, *vbk.params)) {
        throw std::invalid_argument(
            "Config: VBK blocks are invalid: " + state.GetPath() + ", " +
            state.GetDebugMessage());
      }

      if (vbk.startHeight != vbk.blocks[0].height) {
        throw std::invalid_argument(
            "Config: vbk startHeight does not match height of first VBK "
            "bootstrap block");
      }
    }

    if (!btc.blocks.empty()) {
      if (btc.blocks.size() < btc.params->numBlocksForBootstrap()) {
        throw std::invalid_argument(
            "Config: you have to specify at least " +
            std::to_string(btc.params->numBlocksForBootstrap()) +
            " BTC blocks to bootstrap.");
      }

      if (!checkBtcBlocks(btc.blocks, state, *btc.params)) {
        throw std::invalid_argument(
            "Config: BTC blocks are invalid: " + state.GetPath() + ", " +
            state.GetDebugMessage());
      }
    }
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONFIG_HPP___
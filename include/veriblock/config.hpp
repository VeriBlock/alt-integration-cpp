// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_CONFIG_HPP___
#define ALTINTEGRATION_CONFIG_HPP___

#include <utility>
#include <vector>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/stateless_validation.hpp>

#include "veriblock/serde.hpp"

namespace altintegration {

struct Config {
  template <typename Block, typename ChainParams>
  struct Bootstrap {
    int32_t startHeight = 0;
    std::vector<Block> blocks;
    std::shared_ptr<ChainParams> params;
    Bootstrap static create(int32_t start,
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

  void setBTC(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<BtcChainParams> params);

  void setVBK(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<VbkChainParams> params);

  void validate() const;
};

}  // namespace altintegration
#endif
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

/**
 * @file config.hpp
 *
 * Altchains must configure AltTree prior any usage by providing bootstrapping
 * Config.
 *
 * Config MUST specify Bitcoin and Veriblock blockchains configs.
 * Namely, network (mainnet/testnet/regtest), contiguous
 * list of blocks named Bootstrap blocks (must be at least 2016 blocks,
 * at least single retargeting period for Bitcoin and 100 blocks for Veriblock),
 * and height of first bootstrap block.
 *
 */

namespace altintegration {

/**
 * A container for Bitcoin and Veriblock configuration data.
 */
struct Config {
  //! per-chain bootstrap config
  template <typename Block, typename ChainParams>
  struct Bootstrap {
    //! height of the first bootstrap block
    int32_t startHeight = 0;

    //! a contiguous list of blocks.
    //! if empty, bootstrapWithGenesis() will be used
    //! has to have at least `params->numBlocksForBootstrap()` blocks
    std::vector<Block> blocks;

    //! network parameters - Mainnet/Testnet/Regtest...
    std::shared_ptr<ChainParams> params;

    //! factory method to create this config from vector of hexencoded blocks
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
              std::shared_ptr<BtcChainParams> params);

  //! helper, which converts array of hexstrings (blocks) into "Bootstrap" type
  void setVBK(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<VbkChainParams> params);

  //! Validates config instance. Throws on errors.
  //! @throws std::invalid_argument with a message, if something is wrong.
  void validate() const;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONFIG_HPP___

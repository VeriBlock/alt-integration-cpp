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

    static Bootstrap<Block, ChainParams> fromRaw(ReadStream& stream) {
      Bootstrap<Block, ChainParams> bootstrap;
      bootstrap.startHeight = stream.readBE<int32_t>();
      bootstrap.blocks =
          readArrayOf<Block>(stream,
                             0,
                             MAX_CONTEXT_COUNT,
                             (Block(*)(ReadStream&))Block::fromVbkEncoding);
      return bootstrap;
    }

    static Bootstrap<Block, ChainParams> fromRaw(
        const std::vector<uint8_t>& bytes) {
      ReadStream stream(bytes);
      return fromRaw(stream);
    }

    std::vector<uint8_t> toRaw() const {
      WriteStream stream;
      toRaw(stream);
      return stream.data();
    }

    void toRaw(WriteStream& stream) const {
      stream.writeBE<int32_t>(startHeight);
      writeSingleBEValue(stream, blocks.size());
      for (const auto& b : blocks) {
        b.toVbkEncoding(stream);
      }

      this->params->toRaw(stream);
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

  //! small helper to check whether config is valid
  //! @throws std::invalid_argument with a message, if something is wrong.
  void validate() const;

  static Config fromRaw(ReadStream& stream);

  static Config fromRaw(const std::vector<uint8_t>& bytes);

  std::vector<uint8_t> toRaw() const;

  void toRaw(WriteStream& stream) const;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONFIG_HPP___

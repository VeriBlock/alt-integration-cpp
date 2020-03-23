#ifndef ALTINTEGRATION_CONFIG_HPP___
#define ALTINTEGRATION_CONFIG_HPP___

#include <utility>
#include <vector>
#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/params.hpp>

namespace altintegration {

struct Config {
  template <typename Block, typename ChainParams>
  struct Bootstrap {
    //! height of the first bootstrap block
    int32_t startHeight = 0;

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

  std::shared_ptr<AltChainParams> altParams;
  Bootstrap<BtcBlock, BtcChainParams> btc;
  Bootstrap<VbkBlock, VbkChainParams> vbk;
  std::string dbName = "altintegration-db";

  void setBTC(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<BtcChainParams> params) {
    this->btc = decltype(btc)::create(start, hexblocks, std::move(params));
  }

  void setVBK(int32_t start,
              const std::vector<std::string>& hexblocks,
              std::shared_ptr<VbkChainParams> params) {
    this->vbk = decltype(vbk)::create(start, hexblocks, std::move(params));
  }

  void validate() const {
    if (!altParams) {
      throw std::invalid_argument("Config: altchain == nullptr");
    }

    if (!btc.params) {
      throw std::invalid_argument("Config: btc params == nullptr");
    }

    if (!vbk.params) {
      throw std::invalid_argument("Config: vbk params == nullptr");
    }

    if (btc.blocks.empty()) {
      throw std::invalid_argument("Config: empty BTC bootstrap context");
    }

    if (vbk.blocks.empty()) {
      throw std::invalid_argument("Config: empty VBK bootstrap context");
    }

    if (vbk.startHeight != vbk.blocks[0].height) {
      throw std::invalid_argument(
          "Config: vbk startHeight does not match height of first VBK "
          "bootstrap block");
    }

    if (dbName.empty()) {
      throw std::invalid_argument("Config: empty DB name");
    }
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONFIG_HPP___

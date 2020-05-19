// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/config.hpp>

namespace altintegration {

void Config::setBTC(int32_t start,
                    const std::vector<std::string>& hexblocks,
                    std::shared_ptr<BtcChainParams> params) {
  btc = decltype(btc)::create(start, hexblocks, std::move(params));
}

void Config::setVBK(int32_t start,
                    const std::vector<std::string>& hexblocks,
                    std::shared_ptr<VbkChainParams> params) {
  vbk = decltype(vbk)::create(start, hexblocks, std::move(params));
}

void Config::validate() const {
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

Config Config::fromRaw(ReadStream& stream) {}

Config Config::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

void Config::toRaw(WriteStream& stream) const {
  btc.toRaw(stream);
  vbk.toRaw(stream);
  alt->toRaw(stream);
}

std::vector<uint8_t> Config::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

}  // namespace altintegration

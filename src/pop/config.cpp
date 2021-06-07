// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <utility>
#include <veriblock/pop/config.hpp>

namespace altintegration {

static std::shared_ptr<altintegration::VbkChainParams> ParseVbkNetwork(
    std::string net) {
  using namespace altintegration;
  static const std::vector<std::shared_ptr<VbkChainParams>> all{
      std::make_shared<VbkChainParamsMain>(),
      std::make_shared<VbkChainParamsTest>(),
      std::make_shared<VbkChainParamsRegTest>()};

  auto it = std::find_if(
      all.begin(), all.end(), [&](const std::shared_ptr<VbkChainParams>& p) {
        return p->networkName() == net;
      });

  if (it == all.end()) {
    return nullptr;
  }

  return *it;
}

static std::shared_ptr<altintegration::BtcChainParams> ParseBtcNetwork(
    std::string net) {
  using namespace altintegration;
  static const std::vector<std::shared_ptr<BtcChainParams>> all{
      std::make_shared<BtcChainParamsMain>(),
      std::make_shared<BtcChainParamsTest>(),
      std::make_shared<BtcChainParamsRegTest>(),
  };

  auto it = std::find_if(
      all.begin(), all.end(), [&](const std::shared_ptr<BtcChainParams>& p) {
        return p->networkName() == net;
      });

  if (it == all.end()) {
    return nullptr;
  }

  return *it;
}

void Config::SelectBtcParams(std::string net,
                             int startHeight,
                             const std::vector<std::string>& blocks) {
  auto param = ParseBtcNetwork(std::move(net));
  VBK_ASSERT_MSG(param, "BTC network can be either main/test/regtest");
  setBTC(startHeight, blocks, param);
}

void Config::SelectVbkParams(std::string net,
                             int startHeight,
                             const std::vector<std::string>& blocks) {
  auto param = ParseVbkNetwork(std::move(net));
  VBK_ASSERT_MSG(
      param,
      "VBK network can be either main/test/regtest/alpha, current: %s",
      net);
  setVBK(startHeight, blocks, param);
}

void Config::SelectAltParams(std::shared_ptr<AltChainParams> param) {
  alt = std::move(param);
}

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
          " VBK blocks to bootstrap. Current size: " +
          std::to_string(vbk.blocks.size()));
    }

    if (!checkVbkBlocks(vbk.blocks, state, *vbk.params)) {
      throw std::invalid_argument(
          "Config: VBK blocks are invalid: " + state.GetPath() + ", " +
          state.GetDebugMessage());
    }

    if (vbk.startHeight != vbk.blocks[0].getHeight()) {
      throw std::invalid_argument(
          "Config: vbk startHeight does not match height of first VBK "
          "bootstrap block. StartHeight height: " +
          std::to_string(vbk.startHeight) +
          ", current heigth: " + std::to_string(vbk.blocks[0].getHeight()));
    }
  }

  if (!btc.blocks.empty()) {
    if (btc.blocks.size() < btc.params->numBlocksForBootstrap()) {
      throw std::invalid_argument(
          "Config: you have to specify at least " +
          std::to_string(btc.params->numBlocksForBootstrap()) +
          " BTC blocks to bootstrap. Current size: " +
          std::to_string(btc.blocks.size()));
    }

    if (!checkBtcBlocks(btc.blocks, state, *btc.params)) {
      throw std::invalid_argument(
          "Config: BTC blocks are invalid: " + state.GetPath() + ", " +
          state.GetDebugMessage());
    }
  }
}

const BtcChainParams& Config::getBtcParams() const { return *btc.params; }

const VbkChainParams& Config::getVbkParams() const { return *vbk.params; }

const AltChainParams& Config::getAltParams() const { return *alt; }

}  // namespace altintegration

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_SERIALIZABLE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_SERIALIZABLE_HPP_

#include <veriblock/blockchain/btc_chain_params.hpp>

namespace altintegration {

struct BtcChainParamsSerializable : public BtcChainParams {
  ~BtcChainParamsSerializable() override = default;

  uint256 getPowLimit() const override { return powLimit_; }
  uint32_t getPowTargetTimespan() const noexcept override {
    return powTargetTimespan_;
  }
  uint32_t getPowTargetSpacing() const noexcept override {
    return powTargetSpacing_;
  }
  bool getAllowMinDifficultyBlocks() const noexcept override {
    return allowMinDifficultyBlocks_;
  }
  bool getPowNoRetargeting() const noexcept override {
    return powNoRetargeting_;
  }
  BtcBlock getGenesisBlock() const noexcept override { return genesisBlock_; }
  uint32_t numBlocksForBootstrap() const noexcept override {
    return numBlocksForBootstrap_;
  }
  std::string networkName() const noexcept override { return networkName_; }

  static BtcChainParamsSerializable fromRaw(ReadStream& stream);

  static BtcChainParamsSerializable fromRaw(const std::vector<uint8_t>& bytes);

 private:
  uint256 powLimit_;
  uint32_t powTargetTimespan_;
  uint32_t powTargetSpacing_;
  bool allowMinDifficultyBlocks_;
  bool powNoRetargeting_;
  BtcBlock genesisBlock_;
  uint32_t numBlocksForBootstrap_;
  std::string networkName_;
};

}  // namespace altintegration

#endif

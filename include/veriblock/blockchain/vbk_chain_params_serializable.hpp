// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_SERIALIZABLE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_SERIALIZABLE_HPP_

#include <veriblock/blockchain/vbk_chain_params.hpp>

namespace altintegration {

struct VbkChainParamsSerializable : public VbkChainParams {
  ~VbkChainParamsSerializable() override = default;

  bool EnableTimeAdjustment() const noexcept override {
    return EnableTimeAdjustment_;
  }

  std::string networkName() const override { return networkName_; }
  uint256 getMinimumDifficulty() const override { return minimumDifficulty_; }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return transactionMagicByte_;
  }
  bool getPowNoRetargeting() const noexcept override {
    return powNoRetargeting_;
  }
  VbkBlock getGenesisBlock() const noexcept override { return genesisBlock_; }
  uint32_t getRetargetPeriod() const noexcept override {
    return retargetPeriod_;
  }
  uint32_t getTargetBlockTime() const noexcept override {
    return targetBlockTime_;
  }
  uint32_t numBlocksForBootstrap() const noexcept override {
    return numBlocksForBootstrap_;
  }

  uint32_t getKeystoneInterval() const noexcept override {
    return keystoneInterval_;
  }

  uint32_t getFinalityDelay() const noexcept override { return finalityDelay_; }

  const std::vector<uint32_t>& getForkResolutionLookUpTable()
      const noexcept override {
    return forkResolutionLookUpTable_;
  }

  int32_t getEndorsementSettlementInterval() const noexcept override {
    return endorsementSettlementInterval_;
  }

  static VbkChainParamsSerializable fromRaw(ReadStream& stream);

  static VbkChainParamsSerializable fromRaw(const std::vector<uint8_t>& bytes);

 private:
  std::string networkName_;
  bool EnableTimeAdjustment_;
  uint256 minimumDifficulty_;
  VbkNetworkType transactionMagicByte_{false, 0};
  bool powNoRetargeting_;
  VbkBlock genesisBlock_;
  uint32_t retargetPeriod_;
  uint32_t targetBlockTime_;
  uint32_t numBlocksForBootstrap_;
  uint32_t keystoneInterval_;
  uint32_t finalityDelay_;
  std::vector<uint32_t> forkResolutionLookUpTable_;
  int32_t endorsementSettlementInterval_;
};

}  // namespace altintegration

#endif

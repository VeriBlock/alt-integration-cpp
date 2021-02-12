// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

#include <memory>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/serde.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

/**
 * VeriBlock chain parameters.
 */
struct VbkChainParams {
  virtual ~VbkChainParams() = default;
  virtual int getProgPowForkHeight() const = 0;
  virtual uint32_t getProgPowStartTimeEpoch() const noexcept = 0;
  virtual bool isProgPowStartTimeEpochEnabled() const noexcept { return true; }
  virtual std::string networkName() const = 0;
  virtual uint256 getMinimumDifficulty() const = 0;
  virtual VbkNetworkType getTransactionMagicByte() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  virtual uint32_t getRetargetPeriod() const noexcept = 0;
  virtual uint32_t getTargetBlockTime() const noexcept = 0;
  virtual uint32_t numBlocksForBootstrap() const noexcept = 0;
  virtual uint32_t maxFutureBlockTime() const noexcept {
    return mMaxFutureBlockTime;
  }

  //! In miner it is hard to simulate correct timestamps, so this flag disables
  //! Time Adjustment Algorithm in POP Fork Resolution. Set it to TRUE in
  //! production, and to FALSE in tests.
  virtual bool EnableTimeAdjustment() const noexcept = 0;

  virtual int32_t getMaxReorgBlocks() const noexcept { return 2000; }
  virtual uint32_t getKeystoneInterval() const noexcept { return 20; }

  virtual uint32_t getFinalityDelay() const noexcept { return 11; }

  /// addPayloads in VBK tree will reject all payloads that are added this
  /// number of blocks behind of current tip in active chain
  virtual int32_t getHistoryOverwriteLimit() const noexcept {
    /* roughly 100h worth of VBK block production */
    return 12000;
  }

  virtual const std::vector<uint32_t>& getForkResolutionLookUpTable()
      const noexcept {
    return forkResolutionLookUpTable_;
  }

  virtual int32_t getEndorsementSettlementInterval() const noexcept {
    return 400;
  }

  //! when finalizeBlock is called, this many blocks behind final block will be
  //! preserved in RAM.
  //! In VBK this number depends on
  uint32_t preserveAncestorsBehindFinal() const noexcept {
    return getEndorsementSettlementInterval();
  }

 protected:
  uint32_t mMaxFutureBlockTime = 5 * 60;  // 5 min
  std::vector<uint32_t> forkResolutionLookUpTable_{
      100, 100, 95, 89, 80, 69, 56, 40, 21};
};

/**
 * MainNet VBK parameters.
 *
 * @ingroup config
 */
struct VbkChainParamsMain : public VbkChainParams {
  ~VbkChainParamsMain() override = default;

  int getProgPowForkHeight() const override { return 1512000; }
  virtual uint32_t getProgPowStartTimeEpoch() const noexcept override {
    return 1600716052U;
  }

  std::string networkName() const override { return "main"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(900000000000) = d18c2e2800
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("d18c2e2800");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    VbkNetworkType r;
    r.hasValue = false;
    r.value = 0;
    return r;
  }
  bool getPowNoRetargeting() const noexcept override { return false; }

  bool EnableTimeAdjustment() const noexcept override { return true; }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

/**
 * TestNet VBK config.
 *
 * @ingroup config
 */
struct VbkChainParamsTest : public VbkChainParams {
  ~VbkChainParamsTest() override = default;

  int getProgPowForkHeight() const override { return 872000; }
  uint32_t getProgPowStartTimeEpoch() const noexcept override {
    return 1600444017U;
  }

  std::string networkName() const override { return "test"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(100000000) = 5f5e100
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("05F5E100");
  }

  VbkNetworkType getTransactionMagicByte() const noexcept override {
    VbkNetworkType r;
    r.hasValue = true;
    r.value = 0xAA;
    return r;
  }
  bool getPowNoRetargeting() const noexcept override { return false; }

  bool EnableTimeAdjustment() const noexcept override { return true; }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

/**
 * RegTest VBK config.
 *
 * @note technically, there's no RegTest in VeriBlock, we created it only for
 * testing purposes specifically in veriblock-pop-cpp.
 *
 * @ingroup config
 */
struct VbkChainParamsRegTest : public VbkChainParams {
  ~VbkChainParamsRegTest() override = default;

  int getProgPowForkHeight() const override { return 0; }
  uint32_t getProgPowStartTimeEpoch() const noexcept override {
    // disabled (see 'false' below)
    return 0;
  }
  bool isProgPowStartTimeEpochEnabled() const noexcept override {
    return false;
  }

  std::string networkName() const override { return "regtest"; }

  uint32_t numBlocksForBootstrap() const noexcept override { return 0; }

  // hex(1) = 1
  uint256 getMinimumDifficulty() const override {
    return uint256::fromHex("1");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    VbkNetworkType r;
    r.hasValue = true;
    r.value = 0xBB;
    return r;
  }
  bool EnableTimeAdjustment() const noexcept override { return false; }

  bool getPowNoRetargeting() const noexcept override { return true; }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

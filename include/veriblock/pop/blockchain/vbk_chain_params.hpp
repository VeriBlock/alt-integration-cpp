// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

#include <memory>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

/**
 * VeriBlock chain parameters.
 */
struct VbkChainParams {
  virtual ~VbkChainParams() = default;
  virtual int getProgPowForkHeight() const = 0;
  virtual uint32_t getProgPowStartTimeEpoch() const noexcept = 0;
  virtual bool isProgPowStartTimeEpochEnabled() const noexcept { return true; }
  virtual const char* networkName() const = 0;
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

  virtual int32_t getMaxReorgBlocks() const noexcept { return mMaxReorgBlocks; }
  virtual uint32_t getKeystoneInterval() const noexcept { return 20; }

  virtual uint32_t getFinalityDelay() const noexcept { return 11; }

  //! all blocks further than this number of blocks are considered "old"
  virtual int32_t getOldBlocksWindow() const noexcept {
    return mOldBlocksWindow;
  }

  virtual const std::vector<uint32_t>& getForkResolutionLookUpTable()
      const noexcept {
    return forkResolutionLookUpTable_;
  }

  virtual int32_t getEndorsementSettlementInterval() const noexcept {
    return mEndorsementSettlementInterval;
  }

  //! when finalizeBlockImpl is called, this many blocks behind final block will
  //! be preserved in RAM. In VBK we should preserve at least last
  //! `endorsementSettlementInterval` blocks before finalized (not including
  //! finalized).
  uint32_t preserveBlocksBehindFinal() const noexcept {
    VBK_ASSERT(mPreserveBlocksBehindFinal >= mEndorsementSettlementInterval);
    return mPreserveBlocksBehindFinal;
  }

  /* roughly 100h worth of VBK block production */
  // equal to 1.5*8000 (1.5 progpow epochs)
  uint32_t mOldBlocksWindow = 12000;

  uint32_t mEndorsementSettlementInterval = 400;
  uint32_t mPreserveBlocksBehindFinal = mEndorsementSettlementInterval;
  int32_t mMaxReorgBlocks = 20000;

 protected:
  uint32_t mMaxFutureBlockTime = 5 * 60;  // 5 min
  std::vector<uint32_t> forkResolutionLookUpTable_{
      100, 100, 95, 89, 80, 69, 56, 40, 21};
};

/**
 * MainNet VBK parameters.
 *
 */
struct VbkChainParamsMain : public VbkChainParams {
  ~VbkChainParamsMain() override = default;

  int getProgPowForkHeight() const override { return 1512000; }
  virtual uint32_t getProgPowStartTimeEpoch() const noexcept override {
    return 1600716052U;
  }

  const char* networkName() const override { return "main"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(90 000 000 000) = 14f46b0400
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("14f46b0400");
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
 */
struct VbkChainParamsTest : public VbkChainParams {
  ~VbkChainParamsTest() override = default;

  int getProgPowForkHeight() const override { return 872000; }
  uint32_t getProgPowStartTimeEpoch() const noexcept override {
    return 1600444017U;
  }

  const char* networkName() const override { return "test"; }

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

  const char* networkName() const override { return "regtest"; }

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

// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/strutil.hpp>
#include <veriblock/pop/uint.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

//! base class for BTC params
struct BtcChainParams {
  virtual ~BtcChainParams() = default;
  virtual uint256 getPowLimit() const = 0;
  virtual uint32_t getPowTargetTimespan() const noexcept = 0;
  virtual uint32_t getPowTargetSpacing() const noexcept = 0;
  virtual bool getAllowMinDifficultyBlocks() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  //  virtual BtcBlock getGenesisBlock() const noexcept = 0;
  virtual bool EnableTimeAdjustment() const noexcept = 0;
  uint32_t getDifficultyAdjustmentInterval() const noexcept {
    return getPowTargetTimespan() / getPowTargetSpacing();
  }
  //! minimum number of BTC blocks needed to bootstrap chain
  virtual uint32_t numBlocksForBootstrap() const noexcept = 0;
  virtual const char* networkName() const noexcept = 0;
  virtual uint32_t maxFutureBlockTime() const noexcept {
    return mMaxFutureBlockTime;
  }

  //! by default we store this many last BTC blocks in RAM
  int32_t getMaxReorgBlocks() const noexcept {
    VBK_ASSERT(static_cast<uint32_t>(mMaxReorgBlocks) >=
               getDifficultyAdjustmentInterval());
    return mMaxReorgBlocks;
  }

  //! when finalizeBlockImpl is called, this many blocks behind final block will
  //! be preserved in RAM. In BTC we can deallocate all blocks past final block.
  uint32_t preserveBlocksBehindFinal() const noexcept { return 0; }

  //! all blocks further than this number of blocks are considered "old"
  int32_t getOldBlocksWindow() const noexcept { return mOldBlocksWindow; }

  uint32_t mOldBlocksWindow = 1000;
  int32_t mMaxReorgBlocks = BTC_MAX_REORG_BLOCKS_MIN_VALUE;

 protected:
  uint32_t mMaxFutureBlockTime = 2 * 60 * 60;  // 2 hours
};

/**
 * @struct BtcChainParamsMain
 *
 * mainnet network params in Bitcoin chain.
 *
 */
struct BtcChainParamsMain : public BtcChainParams {
  ~BtcChainParamsMain() override = default;

  bool EnableTimeAdjustment() const noexcept override { return true; }

  const char* networkName() const noexcept override { return "main"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getDifficultyAdjustmentInterval();
  };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff00000000");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;  // two weeks
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return false; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1231006505;
  //    block.nonce = 2083236893;
  //    block.bits = 0x1d00ffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("000000000019d6689c085ae165831e934ff763ae46a2a6c172"
  //                         "b3f1b60a8ce26f"));
  //
  //    return block;
  //  }
};

/**
 * @struct BtcChainParamsTest
 *
 * testnet3 network params in Bitcoin chain.
 *
 */
struct BtcChainParamsTest : public BtcChainParams {
  ~BtcChainParamsTest() override = default;

  bool EnableTimeAdjustment() const noexcept override { return true; }

  const char* networkName() const noexcept override { return "test"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getDifficultyAdjustmentInterval();
  };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff00000000");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1296688602;
  //    block.nonce = 414098458;
  //    block.bits = 0x1d00ffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("000000000933ea01ad0ee984209779baaec3ced90fa3f40871"
  //                         "9526f8d77f4943"));
  //
  //    return block;
  //  }
};

/**
 * @struct BtcChainParamsRegTest
 *
 * regtest network params in Bitcoin chain.
 *
 */
struct BtcChainParamsRegTest : public BtcChainParams {
  ~BtcChainParamsRegTest() override = default;

  //! time adjustment is disabled in regtest mode
  bool EnableTimeAdjustment() const noexcept override { return false; }

  const char* networkName() const noexcept override { return "regtest"; }

  uint32_t numBlocksForBootstrap() const noexcept override { return 1; };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return true; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1296688602;
  //    block.nonce = 2;
  //    block.bits = 0x207fffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca59"
  //                         "0b1a11466e2206"));
  //
  //    return block;
  //  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

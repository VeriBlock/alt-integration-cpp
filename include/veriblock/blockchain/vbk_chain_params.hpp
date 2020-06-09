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

//! works as optional<uint8_t>
struct VbkNetworkType {
  VbkNetworkType(bool _hasValue, uint8_t _value)
      : hasValue(_hasValue), value(_value) {}

  //! if hasValue == false, it is mainnet
  bool hasValue = false;
  //! otherwise, use value for network ID
  uint8_t value = 0;

  std::vector<uint8_t> toRaw() const;

  void toRaw(WriteStream& stream) const;

  static VbkNetworkType fromRaw(const std::vector<uint8_t>& bytes);

  static VbkNetworkType fromRaw(ReadStream& stream);
};

/**
 * VeriBlock chain parameters.
 */
struct VbkChainParams {
  virtual ~VbkChainParams() = default;
  virtual std::string networkName() const = 0;
  virtual uint256 getMinimumDifficulty() const = 0;
  virtual VbkNetworkType getTransactionMagicByte() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  virtual VbkBlock getGenesisBlock() const noexcept = 0;
  virtual uint32_t getRetargetPeriod() const noexcept = 0;
  virtual uint32_t getTargetBlockTime() const noexcept = 0;
  virtual uint32_t numBlocksForBootstrap() const noexcept = 0;

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

  std::vector<uint8_t> toRaw() const;

  void toRaw(WriteStream& stream) const;

 private:
  std::vector<uint32_t> forkResolutionLookUpTable_{
      100, 100, 95, 89, 80, 69, 56, 40, 21};
};

/**
 * MainNet.
 */
struct VbkChainParamsMain : public VbkChainParams {
  ~VbkChainParamsMain() override = default;

  std::string networkName() const override { return "main"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(900000000000) = d18c2e2800
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("d18c2e2800");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return VbkNetworkType(false, 0);
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    //{
    //  "height": 0,
    //  "version": 2,
    //  "previousBlock": "000000000000000000000000",
    //  "previousKeystone": "000000000000000000",
    //  "secondPreviousKeystone": "000000000000000000",
    //  "merkleRoot": "a7e5f2b7ec94291767b4d67b4a33682d",
    //  "timestamp": 1553497611,
    //  "difficulty": 100722900,
    //  "nonce": 289244493
    //}
    VbkBlock block;
    block.height = 0;
    block.version = 2;
    block.merkleRoot = uint128::fromHex("a7e5f2b7ec94291767b4d67b4a33682d");
    block.timestamp = 1553497611;
    block.difficulty = 100722900;
    block.nonce = 289244493;

    VBK_ASSERT(block.getHash().toHex() ==
               "0000000000f4fd66b91f0649bb3fcb137823c5ce317c105c");

    return block;
  }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

/**
 * TestNet
 */

struct VbkChainParamsTest : public VbkChainParams {
  ~VbkChainParamsTest() override = default;

  std::string networkName() const override { return "test"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(100000000) = 5f5e100
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("05F5E100");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return VbkNetworkType(true, 0xAA);
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    //{
    //  "height": 0,
    //  "version": 2,
    //  "previousBlock": "000000000000000000000000",
    //  "previousKeystone": "000000000000000000",
    //  "secondPreviousKeystone": "000000000000000000",
    //  "merkleRoot": "a2ea7c29ef7915db412ebd4012a9c617",
    //  "timestamp": 1570649416,
    //  "difficulty": 67499489,
    //  "nonce": 14304633
    //}
    VbkBlock block;
    block.height = 0;
    block.version = 2;
    block.merkleRoot = uint128::fromHex("a2ea7c29ef7915db412ebd4012a9c617");
    block.timestamp = 1570649416;
    block.difficulty = 67499489;
    block.nonce = 14304633;

    VBK_ASSERT(block.getHash().toHex() ==
               "00000017eb579ec7d0cdd63379a0615dc3d68032ce248823");

    return block;
  }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

/**
 * RegTest
 */
struct VbkChainParamsRegTest : public VbkChainParams {
  ~VbkChainParamsRegTest() override = default;

  std::string networkName() const override { return "regtest"; }

  uint32_t numBlocksForBootstrap() const noexcept override { return 0; }

  // hex(1) = 1
  uint256 getMinimumDifficulty() const override {
    return uint256::fromHex("1");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return VbkNetworkType(true, 0xBB);
  }
  bool getPowNoRetargeting() const noexcept override { return true; }
  VbkBlock getGenesisBlock() const noexcept override {
    //{
    //  "height": 0,
    //  "version": 2,
    //  "previousBlock": "000000000000000000000000",
    //  "previousKeystone": "000000000000000000",
    //  "secondPreviousKeystone": "000000000000000000",
    //  "merkleRoot": "a2ea7c29ef7915db412ebd4012a9c617",
    //  "timestamp": 1553699987,
    //  "difficulty": 117586646,
    //  "nonce": 1924857207
    //}

    VbkBlock block;
    block.height = 0;
    block.version = 2;
    block.merkleRoot = uint128::fromHex("a2ea7c29ef7915db412ebd4012a9c617");
    block.timestamp = 1553699987;
    block.difficulty = 16842752;
    block.nonce = 0;

    VBK_ASSERT(block.getHash().toHex() ==
               "5113a60099c9f24260476a546ad38f8a5995053b4b04d16c");

    return block;
  }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

/**
 * AlphaNet
 */
struct VbkChainParamsAlpha : public VbkChainParams {
  ~VbkChainParamsAlpha() override = default;

  std::string networkName() const override { return "alpha"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getRetargetPeriod();
  }

  // hex(9999872) = 989600
  uint256 getMinimumDifficulty() const override {
    return ArithUint256::fromHex("989600");
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return VbkNetworkType(true, 0xAA);
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    //{
    //  "height": 0,
    //  "version": 2,
    //  "previousBlock": "000000000000000000000000",
    //  "previousKeystone": "000000000000000000",
    //  "secondPreviousKeystone": "000000000000000000",
    //  "merkleRoot": "b34a487a6b3a386689f59d8d2e586363",
    //  "timestamp": 1555416021,
    //  "difficulty": 67147926,
    //  "nonce": 45543957
    //}
    VbkBlock block;
    block.height = 0;
    block.version = 2;
    block.merkleRoot = uint128::fromHex("b34a487a6b3a386689f59d8d2e586363");
    block.timestamp = 1555416021;
    block.difficulty = 67147926;
    block.nonce = 45543957;

    VBK_ASSERT(block.getHash().toHex() ==
               "000000701198864f3c19fb552ef9c3c10620ba8128dace8e");
    return block;
  }

  uint32_t getRetargetPeriod() const noexcept override { return 100; }

  uint32_t getTargetBlockTime() const noexcept override { return 30; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

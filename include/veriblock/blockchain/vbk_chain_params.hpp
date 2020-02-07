#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

#include <memory>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/uint.hpp>

namespace VeriBlock {

//! works as optional<uint8_t>
struct VbkNetworkType {
  //! if hasValue == false, it is mainnet
  bool hasValue = false;
  //! otherwise, use value for network ID
  uint8_t value = 0;
};

/**
 * VeriBlock chain parameters.
 */
struct VbkChainParams {
  virtual ~VbkChainParams() = default;
  virtual uint256 getMinimumDifficulty() const = 0;
  virtual VbkNetworkType getTransactionMagicByte() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  virtual VbkBlock getGenesisBlock() const noexcept = 0;
};

/**
 * MainNet.
 */
struct VbkChainParamsMain : public VbkChainParams {
  ~VbkChainParamsMain() override = default;
  // hex(900000000000) = d18c2e2800
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("d18c2e2800"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {false, 0};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    VbkBlock block;
    // TODO: set valid block
    return block;
  }
};

/**
 * TestNet
 */
struct VbkChainParamsTest : public VbkChainParams {
  ~VbkChainParamsTest() override = default;
  // hex(100000000) = 5f5e100
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("5f5e100"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {true, 0xAA};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    VbkBlock block;
    // TODO: set valid block
    return block;
  }
};

/**
 * RegTest
 */
struct VbkChainParamsRegTest : public VbkChainParams {
  ~VbkChainParamsRegTest() override = default;
  // hex(1) = 1
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("1"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return VbkNetworkType{true, 0xBB};
  }
  bool getPowNoRetargeting() const noexcept override { return true; }
  VbkBlock getGenesisBlock() const noexcept override {
    VbkBlock block;
    block.height = 0;
    block.version = 2;
    block.timestamp = 1577367966;
    // minumum possible difficulty
    block.difficulty = 0;  // TODO
    return block;
  }
};

/**
 * AlphaNet
 */
struct VbkChainParamsAlpha : public VbkChainParams {
  ~VbkChainParamsAlpha() override = default;
  // hex(9999872) = 989600
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("989600"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {true, 0xAA};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
  VbkBlock getGenesisBlock() const noexcept override {
    VbkBlock block;
    // TODO: set valid block
    return block;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

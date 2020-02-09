#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

#include <cassert>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/strutil.hpp>
#include <veriblock/uint.hpp>

namespace VeriBlock {

struct BtcChainParams {
  virtual ~BtcChainParams() = default;
  virtual uint256 getPowLimit() const = 0;
  virtual int getPowTargetTimespan() const noexcept = 0;
  virtual int getPowTargetSpacing() const noexcept = 0;
  virtual bool getAllowMinDifficultyBlocks() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  virtual BtcBlock getGenesisBlock() const noexcept = 0;
  int64_t getDifficultyAdjustmentInterval() const noexcept {
    return getPowTargetTimespan() / getPowTargetSpacing();
  }
};

struct BtcChainParamsMain : public BtcChainParams {
  ~BtcChainParamsMain() override = default;

  uint256 getPowLimit() const override {
    return uint256(ParseHex(
        "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;  // two weeks
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return false; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  BtcBlock getGenesisBlock() const noexcept override {
    BtcBlock block;
    block.version = 1;
    block.timestamp = 1231006505;
    block.nonce = 2083236893;
    block.bits = 0x1d00ffff;
    block.merkleRoot = ParseHex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");

    ValidationState state;
    assert(checkBlock(block, state));
    assert(block.getHash().toHex() ==
           "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");

    return block;
  }
};

struct BtcChainParamsTest : public BtcChainParams {
  ~BtcChainParamsTest() override = default;

  uint256 getPowLimit() const override {
    return uint256(ParseHex(
        "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  BtcBlock getGenesisBlock() const noexcept override {
    BtcBlock block;
    block.version = 1;
    block.timestamp = 1296688602;
    block.nonce = 414098458;
    block.bits = 0x1d00ffff;
    block.merkleRoot = ParseHex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");

    ValidationState state;
    assert(checkBlock(block, state));
    assert(block.getHash().reverse().toHex() ==
           "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943");

    return block;
  }
};

struct BtcChainParamsRegTest : public BtcChainParams {
  ~BtcChainParamsRegTest() override = default;

  uint256 getPowLimit() const override {
    return ParseHex(
        "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return true; }
  BtcBlock getGenesisBlock() const noexcept override {
    BtcBlock block;
    block.version = 1;
    block.timestamp = 1296688602;
    block.nonce = 2;
    block.bits = 0x207fffff;
    block.merkleRoot = uint256(ParseHex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

    ValidationState state;
    assert(checkBlock(block, state));
    assert(block.getHash().reverse().toHex() ==
           "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206");

    return block;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

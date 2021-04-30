// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "config2.hpp"
#include "entities/altblock.hpp"
#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"
#include "veriblock/pop/bootstraps.hpp"
#include "veriblock/pop/c/extern2.h"
#include "veriblock/pop/config.hpp"

struct AltChainParamsImpl : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept override {
    return POP_EXTERN_FUNCTION_NAME(get_altchain_id)();
  }

  //! first ALT block used in AltBlockTree. This is first block that can be
  //! endorsed.
  altintegration::AltBlock getBootstrapBlock() const noexcept override {
    auto* res = POP_EXTERN_FUNCTION_NAME(get_bootstrap_block)();
    return res->ref;
  }

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    VBK_ASSERT(bytes.size() != 0);

    POP_ARRAY_NAME(u8) input;
    input.size = bytes.size();
    input.data = const_cast<uint8_t*>(bytes.data());

    auto res = POP_EXTERN_FUNCTION_NAME(get_block_header_hash)(input);

    VBK_ASSERT(res.size <= altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA);
    return std::vector<uint8_t>(res.data, res.data + res.size);
  }

  bool checkBlockHeader(
      const std::vector<uint8_t>& header,
      const std::vector<uint8_t>& root,
      altintegration::ValidationState& state) const noexcept override {
    VBK_ASSERT(header.size() != 0);
    VBK_ASSERT(root.size() != 0);

    POP_ARRAY_NAME(u8) header_input;
    header_input.size = header.size();
    header_input.data = const_cast<uint8_t*>(header.data());

    POP_ARRAY_NAME(u8) root_input;
    root_input.size = root.size();
    root_input.data = const_cast<uint8_t*>(root.data());

    if (!POP_EXTERN_FUNCTION_NAME(check_block_header)(header_input,
                                                      root_input)) {
      return state.Invalid("invalid altchain block header");
    }

    return true;
  }
};

POP_ENTITY_FREE_SIGNATURE(config) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(config) {
  auto* res = new POP_ENTITY_NAME(config);
  res->ref = std::make_shared<altintegration::Config>();
  res->ref->alt = std::make_shared<AltChainParamsImpl>();
  return res;
}

static std::vector<std::string> ParseBlocks(const std::string& blocks) {
  std::vector<std::string> ret;
  std::istringstream ss(blocks);
  std::string substr;
  while (std::getline(ss, substr, ',')) {
    ret.push_back(substr);
  }
  return ret;
}

POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_vbk_params,
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           POP_ARRAY_NAME(string) blocks) {
  VBK_ASSERT(self);
  VBK_ASSERT(net.data);
  VBK_ASSERT(blocks.data);

  if (blocks.size == 0) {
    self->ref->SelectVbkParams(
        std::string(net.data, net.data + net.size),
        start_height,
        {SerializeToRawHex(altintegration::GetRegTestVbkBlock())});
    return;
  }

  auto b = ParseBlocks(std::string(blocks.data, blocks.data + blocks.size));
  VBK_ASSERT_MSG(
      !b.empty(),
      "VBK 'blocks' does not contain valid comma-separated hexstrings");

  self->ref->SelectVbkParams(
      std::string(net.data, net.data + net.size), start_height, b);
}

POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_btc_params,
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           POP_ARRAY_NAME(string) blocks) {
  VBK_ASSERT(self);
  VBK_ASSERT(net.data);
  VBK_ASSERT(blocks.data);

  if (blocks.size == 0) {
    self->ref->SelectBtcParams(
        std::string(net.data, net.data + net.size),
        start_height,
        {SerializeToRawHex(altintegration::GetRegTestBtcBlock())});
    return;
  }

  auto b = ParseBlocks(std::string(blocks.data, blocks.data + blocks.size));
  VBK_ASSERT_MSG(
      !b.empty(),
      "VBK 'blocks' does not contain valid comma-separated hexstrings");

  self->ref->SelectBtcParams(
      std::string(net.data, net.data + net.size), start_height, b);
}
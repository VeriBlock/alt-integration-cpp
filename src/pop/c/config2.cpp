// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "config2.hpp"
#include "veriblock/pop/c/extern.h"
#include "veriblock/pop/config.hpp"

struct AltChainParamsImpl : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept override {
    // TODO: add extern function VBK_getAltchainId into C interface v2
    return VBK_getAltchainId();
  }

  //! first ALT block used in AltBlockTree. This is first block that can be
  //! endorsed.
  altintegration::AltBlock getBootstrapBlock() const noexcept override {
    return altintegration::AssertDeserializeFromRawHex<
        altintegration::AltBlock>(VBK_getBootstrapBlock());
  }

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    VBK_ASSERT(bytes.size() != 0);
    std::vector<uint8_t> hash(altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA,
                              0);
    int size = 0;
    VBK_getBlockHeaderHash(bytes.data(), (int)bytes.size(), hash.data(), &size);
    VBK_ASSERT(size <= altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA);
    hash.resize(size);
    return hash;
  }

  bool checkBlockHeader(
      const std::vector<uint8_t>& bytes,
      const std::vector<uint8_t>& root,
      altintegration::ValidationState& state) const noexcept override {
    (void)bytes;
    (void)root;
    (void)state;
    // TODO: add ectern function VBK_checkBlockHeader into C interface v2
    // VbkValidationState c_state;
    // if (!VBK_checkBlockHeader(bytes.data(),
    //                           (int)bytes.size(),
    //                           root.data(),
    //                           (int)root.size(),
    //                           &c_state)) {
    //   return state.Invalid(c_state.getState().GetPath());
    // }
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

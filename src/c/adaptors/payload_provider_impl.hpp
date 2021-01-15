// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS

#include "veriblock/c/extern.h"
#include "veriblock/storage/payloads_provider.hpp"

namespace adaptors {

struct PayloadsProviderImpl : public altintegration::PayloadsProvider {
  ~PayloadsProviderImpl() override = default;

  PayloadsProviderImpl(size_t maxSize) { buffer.resize(maxSize); }

  bool getContainingAltPayloads(
      const altintegration::BlockIndex<altintegration::AltBlock>& block,
      altintegration::PopData& out,
      altintegration::ValidationState& state) override {
    (void)block;
    (void)out;
    (void)state;
    // TODO implement
    return true;
  }

  bool getContainingVbkPayloads(
      const altintegration::BlockIndex<altintegration::VbkBlock>& block,
      std::vector<altintegration::VTB>& out,
      altintegration::ValidationState& state) override {
    (void)block;
    (void)out;
    (void)state;
    // TODO implement
    return true;
  }

  bool getATV(const typename altintegration::ATV::id_t& id,
              altintegration::ATV& out,
              altintegration::ValidationState& state) override {
    (void)id;
    (void)out;
    (void)state;
    // TODO implement
    return true;
  }

 private:
  std::vector<uint8_t> buffer;
};
}  // namespace adaptors

#endif
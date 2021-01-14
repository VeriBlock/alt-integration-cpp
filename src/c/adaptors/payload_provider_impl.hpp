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

  bool getATVs(const std::vector<altintegration::ATV::id_t>& ids,
               std::vector<altintegration::ATV>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      if (!VBK_getATV(id.data(), (int)id.size(), buffer.data(), &size)) {
        return state.Invalid("get-atv", "atv has been not found");
      }
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::ATV atv_out;
      if (!altintegration::DeserializeFromVbkEncoding(stream, atv_out, state)) {
        return state.Invalid("get-atv", "cannot deserialize atv");
      }
      out.push_back(atv_out);
    }

    return true;
  }

  bool getVTBs(const std::vector<altintegration::VTB::id_t>& ids,
               std::vector<altintegration::VTB>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      if (!VBK_getVTB(id.data(), (int)id.size(), buffer.data(), &size)) {
        return state.Invalid("get-vtb", "vtb has been not found");
      }
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::VTB vtb_out;
      if (!altintegration::DeserializeFromVbkEncoding(stream, vtb_out, state)) {
        return state.Invalid("get-vtb", "cannot deserialize vtb");
      }
      out.push_back(vtb_out);
    }

    return true;
  }

  bool getVBKs(const std::vector<altintegration::VbkBlock::id_t>& ids,
               std::vector<altintegration::VbkBlock>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      if (!VBK_getVBK(id.data(), (int)id.size(), buffer.data(), &size)) {
        return state.Invalid("get-vbk", "vbk has been not found");
      }
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::VbkBlock vbk_out;
      if (!altintegration::DeserializeFromVbkEncoding(stream, vbk_out, state)) {
        return state.Invalid("get-vbk", "cannot deserialize vbk");
      }
      out.push_back(vbk_out);
    }

    return true;
  }

 private:
  std::vector<uint8_t> buffer;
};
}  // namespace adaptors

#endif
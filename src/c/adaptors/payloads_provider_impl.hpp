// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS

#include "storage_interface.hpp"
#include "veriblock/c/extern.h"
#include "veriblock/serde.hpp"
#include "veriblock/storage/payloads_provider.hpp"

namespace adaptors {

const char DB_VBK_PREFIX = '^';
const char DB_VTB_PREFIX = '<';
const char DB_ATV_PREFIX = '>';

template <typename pop_t>
std::vector<uint8_t> payloads_key(const typename pop_t::id_t& id);

template <>
inline std::vector<uint8_t> payloads_key<altintegration::ATV>(
    const altintegration::ATV::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_ATV_PREFIX);
  return res;
}

template <>
inline std::vector<uint8_t> payloads_key<altintegration::VTB>(
    const altintegration::VTB::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_VTB_PREFIX);
  return res;
}

template <>
inline std::vector<uint8_t> payloads_key<altintegration::VbkBlock>(
    const altintegration::VbkBlock::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_VBK_PREFIX);
  return res;
}

struct PayloadsStorageImpl : public altintegration::PayloadsStorage {
  ~PayloadsStorageImpl() override = default;

  PayloadsStorageImpl(Storage& storage) : storage_(storage) {}

  template <typename pop_t>
  bool getPayloads(const typename pop_t::id_t& id,
                   pop_t& out,
                   altintegration::ValidationState& state) {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(payloads_key<pop_t>(id), bytes_out)) {
      return state.Invalid(
          "bad-" + pop_t::name(),
          fmt::format("can not read %s from storage", pop_t::name()));
    }

    altintegration::ReadStream stream(bytes_out);
    if (!altintegration::DeserializeFromVbkEncoding(stream, out, state)) {
      return state.Invalid(
          "bad-" + pop_t::name(),
          fmt::format("can not deserialize %s from bytes", pop_t::name()));
    }

    return true;
  }

  bool getATV(const altintegration::ATV::id_t& id,
              altintegration::ATV& out,
              altintegration::ValidationState& state) override {
    return getPayloads<altintegration::ATV>(id, out, state);
  }

  bool getVTB(const altintegration::VTB::id_t& id,
              altintegration::VTB& out,
              altintegration::ValidationState& state) override {
    return getPayloads<altintegration::VTB>(id, out, state);
  }

  bool getVBK(const altintegration::VbkBlock::id_t& id,
              altintegration::VbkBlock& out,
              altintegration::ValidationState& state) override {
    return getPayloads<altintegration::VbkBlock>(id, out, state);
  }

  void writePayloads(const altintegration::PopData& payloads) override {
    auto batch = storage_.generateWriteBatch();

    for (const auto& atv : payloads.atvs) {
      batch->write(
          payloads_key<altintegration::VbkBlock>(atv.blockOfProof.getId()),
          altintegration::SerializeToVbkEncoding(atv.blockOfProof));

      batch->write(payloads_key<altintegration::ATV>(atv.getId()),
                   altintegration::SerializeToVbkEncoding(atv));
    }

    for (const auto& vtb : payloads.vtbs) {
      batch->write(
          payloads_key<altintegration::VbkBlock>(vtb.containingBlock.getId()),
          altintegration::SerializeToVbkEncoding(vtb.containingBlock));

      batch->write(payloads_key<altintegration::VTB>(vtb.getId()),
                   altintegration::SerializeToVbkEncoding(vtb));
    }

    for (const auto& vbk : payloads.context) {
      batch->write(payloads_key<altintegration::VbkBlock>(vbk.getId()),
                   altintegration::SerializeToVbkEncoding(vbk));
    }

    batch->writeBatch();
  }

 private:
  Storage& storage_;
};

}  // namespace adaptors

#endif
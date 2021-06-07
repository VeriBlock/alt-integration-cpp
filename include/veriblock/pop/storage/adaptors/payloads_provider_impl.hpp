// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS

#include <veriblock/pop/c/extern.h>

#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/storage/payloads_provider.hpp>

#include "storage_interface.hpp"

namespace altintegration {

namespace adaptors {

const char DB_VBK_PREFIX = '^';
const char DB_VTB_PREFIX = '<';
const char DB_ATV_PREFIX = '>';

template <typename pop_t>
std::vector<uint8_t> payloads_key(const typename pop_t::id_t& id);

template <>
inline std::vector<uint8_t> payloads_key<ATV>(const ATV::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_ATV_PREFIX);
  return res;
}

template <>
inline std::vector<uint8_t> payloads_key<VTB>(const VTB::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_VTB_PREFIX);
  return res;
}

template <>
inline std::vector<uint8_t> payloads_key<VbkBlock>(const VbkBlock::id_t& id) {
  auto res = id.asVector();
  res.insert(res.begin(), DB_VBK_PREFIX);
  return res;
}

struct PayloadsStorageImpl : public PayloadsStorage {
  ~PayloadsStorageImpl() override = default;

  PayloadsStorageImpl(Storage& storage) : storage_(storage) {}

  template <typename pop_t>
  bool getPayloads(const typename pop_t::id_t& id,
                   pop_t& out,
                   ValidationState& state) {
    std::vector<uint8_t> bytes_out;
    if (!storage_.read(payloads_key<pop_t>(id), bytes_out)) {
      return state.Invalid(
          "bad-" + pop_t::name(),
          fmt::format("can not read {} from storage", pop_t::name()));
    }

    ReadStream stream(bytes_out);
    if (!DeserializeFromVbkEncoding(stream, out, state)) {
      return state.Invalid(
          "bad-" + pop_t::name(),
          fmt::format("can not deserialize {} from bytes", pop_t::name()));
    }

    return true;
  }

  bool getATV(const ATV::id_t& id, ATV& out, ValidationState& state) override {
    return getPayloads<ATV>(id, out, state);
  }

  bool getVTB(const VTB::id_t& id, VTB& out, ValidationState& state) override {
    return getPayloads<VTB>(id, out, state);
  }

  bool getVBK(const VbkBlock::id_t& id,
              VbkBlock& out,
              ValidationState& state) override {
    return getPayloads<VbkBlock>(id, out, state);
  }

  void writePayloads(const PopData& payloads) override {
    auto batch = storage_.generateWriteBatch();

    for (const auto& atv : payloads.atvs) {
      batch->write(payloads_key<VbkBlock>(atv.blockOfProof.getId()),
                   SerializeToVbkEncoding(atv.blockOfProof));

      batch->write(payloads_key<ATV>(atv.getId()), SerializeToVbkEncoding(atv));
    }

    for (const auto& vtb : payloads.vtbs) {
      batch->write(payloads_key<VbkBlock>(vtb.containingBlock.getId()),
                   SerializeToVbkEncoding(vtb.containingBlock));

      batch->write(payloads_key<VTB>(vtb.getId()), SerializeToVbkEncoding(vtb));
    }

    for (const auto& vbk : payloads.context) {
      batch->write(payloads_key<VbkBlock>(vbk.getId()),
                   SerializeToVbkEncoding(vbk));
    }

    batch->writeBatch();
  }

 private:
  Storage& storage_;
};

}  // namespace adaptors

}  // namespace altintegration

#endif
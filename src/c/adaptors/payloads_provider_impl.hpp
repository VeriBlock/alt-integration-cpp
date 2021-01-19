// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_PAYLOADS_PROVIDER_IMPL_BINDINGS

#include "veriblock/c/extern.h"
#include "veriblock/storage/payloads_provider.hpp"

namespace adaptors {

namespace details {
struct PayloadsReaderImpl : public altintegration::details::PayloadsReader {
  ~PayloadsReaderImpl() override = default;

  PayloadsReaderImpl(size_t maxSize) { buffer.resize(maxSize); }

  bool getATV(const altintegration::ATV::id_t& id,
              altintegration::ATV& out,
              altintegration::ValidationState& state) override {
    (void)id;
    (void)out;
    (void)state;
    // TODO implement
    return true;
  }

  bool getVTB(const altintegration::VTB::id_t& id,
              altintegration::VTB& out,
              altintegration::ValidationState& state) override {
    (void)id;
    (void)out;
    (void)state;
    // TODO implement
    return true;
  }

  bool getVBK(const altintegration::VbkBlock::id_t& id,
              altintegration::VbkBlock& out,
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

struct PayloadsWriterImpl : public altintegration::details::PayloadsWriter {
  ~PayloadsWriterImpl() override = default;

  void writePayloads(const std::vector<altintegration::ATV>& atvs) override {
    (void)atvs;
    // TODO implement
  }

  void writePayloads(const std::vector<altintegration::VTB>& vtbs) override {
    (void)vtbs;
    // TODO implement
  }

  void writePayloads(
      const std::vector<altintegration::VbkBlock>& vbks) override {
    (void)vbks;
    // TODO implement
  }

  void writePayloads(const altintegration::PopData& payloads) override {
    altintegration::details::PayloadsWriter::writePayloads(payloads);
  }
};

}  // namespace details

struct PayloadsProviderImpl : public altintegration::PayloadsProvider {
  ~PayloadsProviderImpl() override = default;

  PayloadsProviderImpl(size_t maxSize) : reader(maxSize) {}

  altintegration::details::PayloadsReader& getPayloadsReader() override {
    return reader;
  }

  altintegration::details::PayloadsWriter& getPayloadsWriter() override {
    return writer;
  }

 private:
  details::PayloadsReaderImpl reader;
  details::PayloadsWriterImpl writer;
};

}  // namespace adaptors

#endif
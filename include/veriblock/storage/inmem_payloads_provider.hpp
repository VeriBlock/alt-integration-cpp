// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_INMEM_PAYLOADS_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_INMEM_PAYLOADS_PROVIDER_HPP

#include <unordered_map>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/storage/payloads_provider.hpp>

namespace altintegration {

namespace details {
//! In-memory implementation of payloads provider. Used in tests.
struct InmemPayloadsReader : public details::PayloadsReader {
  ~InmemPayloadsReader() override = default;

  InmemPayloadsReader(
      std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>& atvs,
      std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>& vtbs,
      std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>& vbkblocks)
      : atvs_(atvs), vtbs_(vtbs), vbkblocks_(vbkblocks) {}

  bool getATVs(const std::vector<ATV::id_t>& ids,
               std::vector<ATV>& out,
               ValidationState& /* ignore */) override {
    out = getPayload<ATV>(ids);
    return true;
  }
  bool getVTBs(const std::vector<VTB::id_t>& ids,
               std::vector<VTB>& out,
               ValidationState& /* ignore */) override {
    out = getPayload<VTB>(ids);
    return true;
  }
  bool getVBKs(const std::vector<VbkBlock::id_t>& ids,
               std::vector<VbkBlock>& out,
               ValidationState& /* ignore */) override {
    out = getPayload<VbkBlock>(ids);
    return true;
  }

  bool getATV(const ATV::id_t& id, ATV& out, ValidationState& state) override {
    (void)state;
    auto vec = getPayload<ATV>({id});
    VBK_ASSERT(vec.size() == 1);
    out = vec.at(0);
    return true;
  }

  template <typename T>
  std::vector<T> getPayload(const std::vector<typename T::id_t>& ids) {
    std::vector<T> ret;
    ret.reserve(ids.size());

    auto& m = getMap<T>();
    for (const auto& id : ids) {
      auto it = m.find(id);
      // when fails, it means that AddPayloads was executed but according
      // payloadsProvider.write was not
      VBK_ASSERT_MSG(
          it != m.end(),
          "requested %s with id=%s not found in InmemPayloadsProvider.",
          T::name(),
          HexStr(id));
      ret.push_back(*it->second);
    }

    return ret;
  }

  template <typename T>
  std::unordered_map<typename T::id_t, std::shared_ptr<T>>& getMap();

 private:
  std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>& atvs_;
  std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>& vtbs_;
  std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>& vbkblocks_;
};

template <>
inline std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>&
InmemPayloadsReader::getMap() {
  return atvs_;
}
template <>
inline std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>&
InmemPayloadsReader::getMap() {
  return vtbs_;
}
template <>
inline std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>&
InmemPayloadsReader::getMap() {
  return vbkblocks_;
}

struct InmemPayloadsWriter : public PayloadsWriter {
  ~InmemPayloadsWriter() override = default;

  InmemPayloadsWriter(
      std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>& atvs,
      std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>& vtbs,
      std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>& vbkblocks)
      : atvs_(atvs), vtbs_(vtbs), vbkblocks_(vbkblocks) {}

  bool writePayloads(const std::vector<ATV>& atvs) override {
    for (const auto& atv : atvs) {
      atvs_.insert({atv.getId(), std::make_shared<ATV>(atv)});
      vbkblocks_.insert({atv.blockOfProof.getId(),
                         std::make_shared<VbkBlock>(atv.blockOfProof)});
    }
    return true;
  }

  bool writePayloads(const std::vector<VTB>& vtbs) override {
    for (const auto& vtb : vtbs) {
      vtbs_.insert({vtb.getId(), std::make_shared<VTB>(vtb)});
      vbkblocks_.insert({vtb.containingBlock.getId(),
                         std::make_shared<VbkBlock>(vtb.containingBlock)});
    }
    return true;
  }

  bool writePayloads(const std::vector<VbkBlock>& vbks) override {
    for (const auto& vbk : vbks) {
      vbkblocks_.insert({vbk.getId(), std::make_shared<VbkBlock>(vbk)});
    }

    return true;
  }

 private:
  std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>& atvs_;
  std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>& vtbs_;
  std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>& vbkblocks_;
};

}  // namespace details

struct InmemPayloadsProvider : public PayloadsProvider {
  ~InmemPayloadsProvider() override = default;

  InmemPayloadsProvider()
      : reader(atvs_, vtbs_, vbkblocks_), writer(atvs_, vtbs_, vbkblocks_) {}

  details::PayloadsReader& getPayloadsReader() override { return reader; }

  details::PayloadsWriter& getPayloadsWriter() override { return writer; }

 private:
  std::unordered_map<ATV::id_t, std::shared_ptr<ATV>> atvs_;
  std::unordered_map<VTB::id_t, std::shared_ptr<VTB>> vtbs_;
  std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>> vbkblocks_;

  details::InmemPayloadsReader reader;
  details::InmemPayloadsWriter writer;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_PAYLOADS_PROVIDER_HPP

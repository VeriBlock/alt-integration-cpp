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

struct InmemPayloadsProvider : public PayloadsProvider,
                               public details::PayloadsWriter,
                               public details::PayloadsReader {
  ~InmemPayloadsProvider() override = default;

  details::PayloadsReader& getPayloadsReader() override { return *this; }

  details::PayloadsWriter& getPayloadsWriter() override { return *this; }

  bool getATV(const ATV::id_t& id,
              ATV& out,
              ValidationState& /* ignore */) override {
    auto vec = getPayload<ATV>({id});
    VBK_ASSERT(vec.size() == 1);
    out = vec.at(0);
    return true;
  }
  bool getVTB(const VTB::id_t& id,
              VTB& out,
              ValidationState& /* ignore */) override {
    auto vec = getPayload<VTB>({id});
    VBK_ASSERT(vec.size() == 1);
    out = vec.at(0);
    return true;
  }
  bool getVBK(const VbkBlock::id_t& id,
              VbkBlock& out,
              ValidationState& /* ignore */) override {
    auto vec = getPayload<VbkBlock>({id});
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

  void writePayloads(const std::vector<ATV>& atvs) override {
    for (const auto& atv : atvs) {
      atvs_.insert({atv.getId(), std::make_shared<ATV>(atv)});
      vbkblocks_.insert({atv.blockOfProof.getId(),
                         std::make_shared<VbkBlock>(atv.blockOfProof)});
    }
  }

  void writePayloads(const std::vector<VTB>& vtbs) override {
    for (const auto& vtb : vtbs) {
      vtbs_.insert({vtb.getId(), std::make_shared<VTB>(vtb)});
      vbkblocks_.insert({vtb.containingBlock.getId(),
                         std::make_shared<VbkBlock>(vtb.containingBlock)});
    }
  }

  void writePayloads(const std::vector<VbkBlock>& vbks) override {
    for (const auto& vbk : vbks) {
      vbkblocks_.insert({vbk.getId(), std::make_shared<VbkBlock>(vbk)});
    }
  }

  void writePayloads(const PopData& payloads) override {
    details::PayloadsWriter::writePayloads(payloads);
  }

 private:
  std::unordered_map<ATV::id_t, std::shared_ptr<ATV>> atvs_;
  std::unordered_map<VTB::id_t, std::shared_ptr<VTB>> vtbs_;
  std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>> vbkblocks_;
};

template <>
inline std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>&
InmemPayloadsProvider::getMap() {
  return atvs_;
}
template <>
inline std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>&
InmemPayloadsProvider::getMap() {
  return vtbs_;
}
template <>
inline std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>&
InmemPayloadsProvider::getMap() {
  return vbkblocks_;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_PAYLOADS_PROVIDER_HPP

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

  bool getContainingAltPayloads(const BlockIndex<AltBlock>& block,
                                PopData& out,
                                ValidationState& state) override {
    (void)state;
    out.version = 1;
    out.context = getPayload<VbkBlock>(block.getPayloadIds<VbkBlock>());
    out.vtbs = getPayload<VTB>(block.getPayloadIds<VTB>());
    out.atvs = getPayload<ATV>(block.getPayloadIds<ATV>());
    return true;
  }

  bool getATV(const ATV::id_t& id, ATV& out, ValidationState& state) override {
    (void)state;
    auto vec = getPayload<ATV>({id});
    VBK_ASSERT(vec.size() == 1);
    out = vec.at(0);
    return true;
  }

  /**
   * Returns std::vector<VTB> stored in a block.
   * @param[in] block input block
   * @param[out] out std::vector<VTB> stored in a block
   * @param[out] state in case of error, will contain error message
   * @return true if payload has been loaded, false otherwise
   */
  bool getContainingVbkPayloads(const BlockIndex<VbkBlock>& block,
                                std::vector<VTB>& out,
                                ValidationState& state) override {
    (void)state;
    out = getPayload<VTB>(block.getPayloadIds<VTB>());
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
      VBK_ASSERT_MSG(it != m.end(),
                     "requested %s with id=%s not found in "
                     "InmemPayloadsProvider. Have you called 'write'?",
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

  bool writePayloads(const BlockIndex<AltBlock>& containing_block,
                     const std::vector<ATV>& atvs) override {
    (void)containing_block;

    for (const auto& atv : atvs) {
      atvs_.insert({atv.getId(), std::make_shared<ATV>(atv)});
      vbkblocks_.insert({atv.blockOfProof.getId(),
                         std::make_shared<VbkBlock>(atv.blockOfProof)});
    }

    return true;
  }

  bool writePayloads(const BlockIndex<AltBlock>& containing_block,
                     const std::vector<VTB>& vtbs) override {
    (void)containing_block;

    for (const auto& vtb : vtbs) {
      vtbs_.insert({vtb.getId(), std::make_shared<VTB>(vtb)});
      vbkblocks_.insert({vtb.containingBlock.getId(),
                         std::make_shared<VbkBlock>(vtb.containingBlock)});
    }

    return true;
  }

  bool writePayloads(const BlockIndex<AltBlock>& containing_block,
                     const std::vector<VbkBlock>& vbks) override {
    (void)containing_block;

    for (const auto& vbk : vbks) {
      vbkblocks_.insert({vbk.getId(), std::make_shared<VbkBlock>(vbk)});
    }

    return true;
  }

  bool writePayloads(const BlockIndex<VbkBlock>& containing_block,
                     const std::vector<VTB>& vtbs) override {
    (void)containing_block;

    for (const auto& vtb : vtbs) {
      vtbs_.insert({vtb.getId(), std::make_shared<VTB>(vtb)});
      vbkblocks_.insert({vtb.containingBlock.getId(),
                         std::make_shared<VbkBlock>(vtb.containingBlock)});
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

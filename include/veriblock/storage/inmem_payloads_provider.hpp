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

//! In-memory implementation of payloads provider. Used in tests.
struct InmemPayloadsProvider : public PayloadsProvider {
  ~InmemPayloadsProvider() override = default;

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

  void write(const PopData& data) {
    write(data.context);
    write(data.vtbs);
    write(data.atvs);
  }

  template <typename T>
  void write(const std::vector<T>& vs) {
    for (auto& v : vs) {
      write(v);
    }
  }
  template <typename T>
  void write(const T& v) {
    auto& m = getMap<T>();
    m.insert({v.getId(), std::make_shared<T>(v)});
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
  std::unordered_map<ATV::id_t, std::shared_ptr<ATV>> atvs;
  std::unordered_map<VTB::id_t, std::shared_ptr<VTB>> vtbs;
  std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>> vbkblocks;
};

template <>
inline std::unordered_map<ATV::id_t, std::shared_ptr<ATV>>&
InmemPayloadsProvider::getMap() {
  return atvs;
}
template <>
inline std::unordered_map<VTB::id_t, std::shared_ptr<VTB>>&
InmemPayloadsProvider::getMap() {
  return vtbs;
}
template <>
inline std::unordered_map<VbkBlock::id_t, std::shared_ptr<VbkBlock>>&
InmemPayloadsProvider::getMap() {
  return vbkblocks;
}

template <>
inline void InmemPayloadsProvider::write(const ATV& v) {
  auto& m = getMap<ATV>();
  m.insert({v.getId(), std::make_shared<ATV>(v)});
  write(v.blockOfProof);
}

template <>
inline void InmemPayloadsProvider::write(const VTB& v) {
  auto& m = getMap<VTB>();
  m.insert({v.getId(), std::make_shared<VTB>(v)});
  write(v.containingBlock);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_INMEM_PAYLOADS_PROVIDER_HPP

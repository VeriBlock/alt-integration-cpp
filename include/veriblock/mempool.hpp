// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vtb.hpp"
#include "mempool_result.hpp"

namespace altintegration {

struct MemPool {
  using vbk_hash_t = decltype(VbkBlock::previousBlock);

  struct VbkPayloadsRelations {
    using id_t = VbkBlock::id_t;

    VbkPayloadsRelations(const VbkBlock& b) : header(b) {}

    VbkBlock header;
    std::vector<std::shared_ptr<VTB>> vtbs;
    std::vector<std::shared_ptr<ATV>> atvs;

    PopData toPopData() const {
      PopData pop;
      pop.context.push_back(header);
      for (const auto& vtb : vtbs) {
        pop.vtbs.push_back(*vtb);
      }

      for (const auto& atv : atvs) {
        pop.atvs.push_back(*atv);
      }

      // TODO: we might want to sort VTBs in ascending order of their
      // blockOfProofs to guarantee that within a single block they all are
      // connected.

      return pop;
    }
  };

  template <typename Payload>
  using payload_map =
      std::unordered_map<typename Payload::id_t, std::shared_ptr<Payload>>;

  using vbkblock_map_t = payload_map<VbkBlock>;
  using atv_map_t = payload_map<ATV>;
  using vtb_map_t = payload_map<VTB>;
  using relations_map_t = payload_map<VbkPayloadsRelations>;

  ~MemPool() = default;
  MemPool(const AltChainParams& alt_param,
          const VbkChainParams& vbk_params,
          const BtcChainParams& btc_params)
      : alt_chain_params_(&alt_param),
        vbk_chain_params_(&vbk_params),
        btc_chain_params_(&btc_params) {}

  template <typename T>
  const T* get(const typename T::id_t& id) const {
    const auto& map = getMap<T>();
    auto it = map.find(id);
    if (it != map.end()) {
      return it->second.get();
    }

    return nullptr;
  }

  template <typename T>
  bool submit(const T& pl, ValidationState& state) {
    (void)pl;
    (void)state;
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::submit");
    return true;
  }

  MempoolResult submitAll(const PopData& pop);

  template <typename T>
  const payload_map<T>& getMap() const {
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::getMap");
  }

  PopData getPop(AltTree& tree);

  void removePayloads(const PopData& v_popData);

  void clear() {
    relations_.clear();
    vbkblocks_.clear();
    stored_vtbs_.clear();
    stored_atvs_.clear();
  }

 private:
  // relations between VBK block and payloads
  relations_map_t relations_;
  vbkblock_map_t vbkblocks_;
  atv_map_t stored_atvs_;
  vtb_map_t stored_vtbs_;

  const AltChainParams* alt_chain_params_{nullptr};
  const VbkChainParams* vbk_chain_params_{nullptr};
  const BtcChainParams* btc_chain_params_{nullptr};

  VbkPayloadsRelations& touchVbkBlock(const VbkBlock& block);
};

template <>
bool MemPool::submit(const ATV& atv, ValidationState& state);

template <>
bool MemPool::submit(const VTB& vtb, ValidationState& state);

template <>
bool MemPool::submit(const VbkBlock& block, ValidationState& state);

template <>
inline const MemPool::payload_map<VbkBlock>& MemPool::getMap() const {
  return vbkblocks_;
}

template <>
inline const MemPool::payload_map<ATV>& MemPool::getMap() const {
  return stored_atvs_;
}

template <>
inline const MemPool::payload_map<VTB>& MemPool::getMap() const {
  return stored_vtbs_;
}

namespace detail {

template <typename Value, typename T>
inline void mapToJson(Value& obj, const MemPool& mp, const std::string& key) {
  auto arr = json::makeEmptyArray<Value>();
  for (auto& p : mp.getMap<T>()) {
    json::arrayPushBack(arr, ToJSON<Value>(p.first));
  }
  json::putKV(obj, key, arr);
}
}  // namespace detail

template <typename Value>
Value ToJSON(const MemPool& mp) {
  auto obj = json::makeEmptyObject<Value>();

  detail::mapToJson<Value, VbkBlock>(obj, mp, "vbkblocks");
  detail::mapToJson<Value, ATV>(obj, mp, "atvs");
  detail::mapToJson<Value, VTB>(obj, mp, "vtbs");

  return obj;
}

}  // namespace altintegration

#endif  // !

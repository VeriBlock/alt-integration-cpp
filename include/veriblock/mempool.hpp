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
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vtb.hpp"

namespace altintegration {

typedef std::vector<uint8_t> (*Hash_Function)(
    const std::vector<uint8_t>& bytes);

struct MemPool {
  using vbk_hash_t = decltype(VbkBlock::previousBlock);

  struct VbkBlockPayloads {
    using id_t = VbkBlock::id_t;

    VbkBlock block;
    std::unordered_set<ATV::id_t> atvs;
    std::unordered_set<VTB::id_t> vtbs;
  };

  template <typename Payload>
  using payload_map =
      std::unordered_map<typename Payload::id_t, std::shared_ptr<Payload>>;

  using vbkblock_map_t = payload_map<VbkBlockPayloads>;
  using atv_map_t = payload_map<ATV>;
  using vtb_map_t = payload_map<VTB>;

  ~MemPool() = default;
  MemPool(const AltChainParams& alt_param,
          const VbkChainParams& vbk_params,
          const BtcChainParams& btc_params,
          Hash_Function function)
      : alt_chain_params_(&alt_param),
        vbk_chain_params_(&vbk_params),
        btc_chain_params_(&btc_params),
        hasher(function) {}

  bool submitVTB(const std::vector<VTB>& vtb, ValidationState& state);
  bool submitATV(const std::vector<ATV>& atv, ValidationState& state);

  std::vector<PopData> getPop(AltTree& tree);

  void removePayloads(const std::vector<PopData>& v_popData);

  const atv_map_t& getATVs() const { return stored_atvs_; }

  const vtb_map_t& getVTBs() const { return stored_vtbs_; }

  const vbkblock_map_t& getVbkBlocks() const { return vbkblocks_; }

 private:
  vbkblock_map_t vbkblocks_;
  atv_map_t stored_atvs_;
  vtb_map_t stored_vtbs_;

  const AltChainParams* alt_chain_params_{nullptr};
  const VbkChainParams* vbk_chain_params_{nullptr};
  const BtcChainParams* btc_chain_params_{nullptr};

  Hash_Function hasher;

  // create and return VbkBlockPayloads if not exist, or return existing
  VbkBlockPayloads& touchVbkBlock(const VbkBlock& block);

  bool fillContext(VbkBlock first_block,
                   std::vector<VbkBlock>& context,
                   AltTree& tree);
  void fillVTBs(std::vector<VTB>& vtbs,
                const std::vector<VbkBlock>& vbk_context);

  bool applyPayloads(const AltBlock& hack_block,
                     PopData& popdata,
                     AltTree& tree,
                     ValidationState& state);
};

template <typename Value>
Value ToJSON(const MemPool& mp) {
  auto obj = json::makeEmptyObject<Value>();

  auto vbk = json::makeEmptyArray<Value>();
  for (auto& p : mp.getVbkBlocks()) {
    json::arrayPushBack(vbk, ToJSON<Value>(p.first));
  }
  json::putKV(obj, "vbkblocks", vbk);

  auto vtbs = json::makeEmptyArray<Value>();
  for (auto& p : mp.getVTBs()) {
    json::arrayPushBack(vbk, ToJSON<Value>(p.first));
  }
  json::putKV(obj, "vtbs", vbk);

  auto atvs = json::makeEmptyArray<Value>();
  for (auto& p : mp.getATVs()) {
    json::arrayPushBack(vbk, ToJSON<Value>(p.first));
  }
  json::putKV(obj, "atvs", vbk);

  return obj;
}

}  // namespace altintegration

#endif  // !

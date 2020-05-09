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
  using block_index_t = std::unordered_map<vbk_hash_t, VbkBlock>;

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

  std::vector<PopData> getPop(const AltBlock& current_block, AltTree& tree);

  void removePayloads(const std::vector<PopData>& v_popData);

 private:
  block_index_t block_index_;

  std::unordered_map<ATV::id_t, ATV> stored_atvs_;
  std::unordered_map<BtcEndorsement::id_t, VTB> stored_vtbs_;

  const AltChainParams* alt_chain_params_{nullptr};
  const VbkChainParams* vbk_chain_params_{nullptr};
  const BtcChainParams* btc_chain_params_{nullptr};

  Hash_Function hasher;

  void uploadVbkContext(const VTB&);
  void uploadVbkContext(const ATV&);

  bool fillContext(VbkBlock first_block,
                   std::vector<VbkBlock>& context,
                   AltTree& tree);
  void fillVTBs(std::vector<VTB>& vtbs,
                const std::vector<VbkBlock>& vbk_contex);

  bool applyPayloads(const AltBlock& hack_block,
                     PopData& popdata,
                     AltTree& tree,
                     ValidationState& state);
};

}  // namespace altintegration

#endif  // !

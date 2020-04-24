#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/entities/altpoptx.hpp"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/vtb.hpp"

namespace altintegration {

typedef std::vector<uint8_t> (*Hash_Function)(
    const std::vector<uint8_t>& bytes);

struct MemPool {
  using vbk_hash_t = decltype(VbkBlock::previousBlock);

  ~MemPool() = default;
  MemPool(Hash_Function /*function*/) {}

  bool submitVTB(const std::vector<VTB>& vtb,
                 const AltTree& tree,
                 ValidationState& state);
  bool submitATV(const std::vector<ATV>& atv,
                 const AltTree& tree,
                 ValidationState& state);

  bool getPop(const AltBlock& current_block,
              AltTree& tree,
              AltPopTx* out_data,
              ValidationState& state);

 private:
  std::vector<ATV> stored_atvs;

  std::vector<VTB> stored_vtbs;

  // Hash_Function hasher;
};

}  // namespace altintegration

#endif  // !

#include "veriblock/mempool.hpp"

#include "veriblock/stateless_validation.hpp"

namespace altintegration {

bool MemPool::submitATV(const std::vector<ATV>& atvs,
                        const AltTree& tree,
                        ValidationState& state) {
  for (size_t i = 0; i < atvs.size(); ++i) {
    if (!checkATV(atvs[i], state, tree.getParams(), tree.vbk().getParams())) {
      return state.Invalid("mempool-submit-atv", i);
    }

    stored_atvs.push_back(atvs[i]);
  }

  return true;
}

bool MemPool::submitVTB(const std::vector<VTB>& vtbs,
                        const AltTree& tree,
                        ValidationState& state) {
  for (size_t i = 0; i < vtbs.size(); ++i) {
    if (!checkVTB(
            vtbs[i], state, tree.vbk().getParams(), tree.btc().getParams())) {
      state.Invalid("mempool-submit-vtb", i);
    }
    auto hash = vtbs[i].containingBlock.getHash().trimLE<vbk_hash_t::size()>();
    stored_vtbs.insert(std::make_pair(hash, vtbs[i]));
  }
}

bool MemPool::getPop(const AltBlock& current_block,
                     AltTree& tree,
                     AltPopTx* out_data,
                     ValidationState& state) {
  if (!tree.setState(current_block.getHash(), state)) {
    return state.Invalid("mempool-get-pop");
  }

  for (const auto& atv : stored_atvs) {
    VbkBlock first_block =
        atv.context.size() != 0 ? atv.context[0] : atv.containingBlock;

    auto range = stored_vtbs.equal_range(first_block.previousBlock);
  }

}  // namespace altintegration

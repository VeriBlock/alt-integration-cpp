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

    stored_vtbs.push_back(vtbs[i]);
  }

  return true;
}

bool MemPool::getPop(const AltBlock& current_block,
                     AltTree& tree,
                     AltPopTx* out_data,
                     ValidationState& state) {
  static constexpr size_t vbk_prev_block_hash_size =
      decltype(VbkBlock::previousBlock)::size();

  if (!tree.setState(current_block.getHash(), state)) {
    return state.Invalid("mempool-get-pop");
  }

  for (const auto& atv : stored_atvs) {
    VbkBlock first_block =
        atv.context.size() != 0 ? atv.context[0] : atv.containingBlock;

    if (tree.vbk().getBlockIndex(first_block.getHash()) == nullptr) {
      for (auto it = stored_vtbs.begin(); it != stored_vtbs.end();) {
        auto& vtb = *it;
        bool is_connected_to_atv = false;
        bool is_connected_to_the_tree = false;

        if (first_block.previousBlock ==
            vtb.containingBlock.getHash().trimLE<vbk_prev_block_hash_size>()) {
          is_connected_to_atv = true;
        }

        if (tree.vbk().getBlockIndex(vtb.containingBlock.previousBlock) !=
            nullptr) {
          is_connected_to_the_tree = true;
        }

        for (auto rit = vtb.context.rbegin();
             rit != vtb.context.rend() &&
             (!is_connected_to_atv || !is_connected_to_the_tree);
             ++rit) {
          auto& b = *rit;

          if (!is_connected_to_atv &&
              first_block.previousBlock ==
                  b.getHash().trimLE<vbk_prev_block_hash_size>()) {
            is_connected_to_atv = true;
          }
          if (tree.vbk().getBlockIndex(b.previousBlock) != nullptr) {
            is_connected_to_the_tree = true;
          }
        }

        if (is_connected_to_atv && is_connected_to_the_tree) {
          out_data->vtbs.push_back(vtb);
          it = stored_vtbs.erase(it);
          continue;
        }
        ++it;
      }
    }
  }

  return true;
}

}  // namespace altintegration

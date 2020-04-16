#ifndef ALTINTEGRATION_ALTINTEGRATION_HPP
#define ALTINTEGRATION_ALTINTEGRATION_HPP

#include "veriblock/alt-util.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/config.hpp"

namespace altintegration {

struct Altintegration {
  static AltTree create(const Config& config) {
    config.validate();
    AltTree tree(*config.alt, *config.vbk.params, *config.btc.params);
    ValidationState state;

    // first, bootstrap BTC
    if (config.btc.blocks.empty()) {
      tree.btc().bootstrapWithGenesis(state);
      assert(state.IsValid());
    } else {
      tree.btc().bootstrapWithChain(
          config.btc.startHeight, config.btc.blocks, state);
      assert(state.IsValid());
    }

    // then, bootstrap VBK
    if (config.vbk.blocks.empty()) {
      tree.vbk().bootstrapWithGenesis(state);
      assert(state.IsValid());
    } else {
      tree.vbk().bootstrapWithChain(
          config.vbk.startHeight, config.vbk.blocks, state);
      assert(state.IsValid());
    }

    tree.bootstrap(state);

    return tree;
  }

  // addPayloads with the full VTB and ATV validation
  static bool addPayloads(AltTree& tree,
                          const AltBlock& block,
                          const std::vector<AltPayloads>& payloads,
                          ValidationState& state) {
    for (size_t i = 0, size = payloads.size(); i < size; i++) {
      auto& p = payloads[i];
      if (p.hasAtv &&
          !checkATV(p.atv, state, tree.getParams(), tree.vbk().getParams())) {
        return state.addIndex(i).Invalid("bad-payloads-atv-stateless");
      }
      for (size_t j = 0; j < payloads[i].vtbs.size(); j++) {
        auto& vtb = payloads[i].vtbs[j];
        if (!checkVTB(vtb,
                      state,
                      tree.vbk().getParams(),
                      tree.vbk().btc().getParams())) {
          return state.addIndex(i)
              .addRejectReason("bad-payloads")
              .addIndex(j)
              .Invalid("bad-vtb-stateless");
        }
      }
    }

    return tree.addPayloads(block, payloads, state);
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALTINTEGRATION_HPP

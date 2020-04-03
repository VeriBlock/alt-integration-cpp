#include "veriblock/entities/context.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/entities/vtb.hpp"

namespace altintegration {

VbkContext VbkContext::fromContainer(const VTB& vtb) {
  VbkContext context;
  context.btc = vtb.transaction.blockOfProofContext;
  context.btc.push_back(vtb.transaction.blockOfProof);
  context.endorsement = BtcEndorsement::fromContainer(vtb);
  return context;
}

AltContext AltContext::fromContainer(const AltPayloads& altPayloads) {
  AltContext context;
  context.vbk = altPayloads.alt.atv.context;
  context.vbk.push_back(altPayloads.alt.atv.containingBlock);
  context.endorsement = VbkEndorsement::fromContainer(altPayloads);

  context.vbkContext.resize(altPayloads.vtbs.size());
  for (size_t i = 0; i < context.vbkContext.size(); ++i) {
    context.vbkContext[i] = {altPayloads.vtbs[i].containingBlock,
                             VbkContext::fromContainer(altPayloads.vtbs[i]),
                             altPayloads.vtbs[i].context};
  }

  context.updateContextVbk = altPayloads.vbkcontext;
  context.updateContextBtc = altPayloads.btccontext;

  return context;
}  // namespace altintegration
}  // namespace altintegration

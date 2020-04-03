#include "veriblock/entities/context.hpp"
#include "veriblock/entities/vtb.hpp"

namespace altintegration {

VbkContext VbkContext::fromContainer(const VTB& vtb) {
  VbkContext context;
  context.btc = vtb.transaction.blockOfProofContext;
  context.btc.push_back(vtb.transaction.blockOfProof);
  context.endorsement = BtcEndorsement::fromContainer(vtb);
  return context;
}
}  // namespace altintegration

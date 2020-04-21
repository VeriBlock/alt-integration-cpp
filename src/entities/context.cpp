#include <veriblock/entities/context.hpp>

#include "veriblock/entities/vtb.hpp"

namespace altintegration {

PartialVTB PartialVTB::fromVTB(const VTB& vtb) {
  PartialVTB p_vtb;
  p_vtb.endorsement = BtcEndorsement::fromContainer(vtb);

  for (const auto& b : vtb.transaction.blockOfProofContext) {
    p_vtb.btc.push_back(std::make_shared<BtcBlock>(b));
  }
  p_vtb.btc.push_back(std::make_shared<BtcBlock>(vtb.transaction.blockOfProof));

  p_vtb.containing = std::make_shared<VbkBlock>(vtb.containingBlock);

  return p_vtb;
}

std::vector<PartialVTB> PartialVTB::fromVTB(const std::vector<VTB>& vtbs) {
  std::vector<PartialVTB> p_vtbs;
  p_vtbs.reserve(vtbs.size());

  std::transform(
      vtbs.begin(),
      vtbs.end(),
      std::back_inserter(p_vtbs),
      [&](const VTB& vtb) -> PartialVTB { return PartialVTB::fromVTB(vtb); });

  return p_vtbs;
}

VbkBlock PartialVTB::getContainingBlock() const { return *containing; }

bool PartialVTB::containsEndorsements() const { return true; }

BtcEndorsement PartialVTB::getEndorsement() const { return endorsement; }

typename BtcEndorsement::id_t PartialVTB::getEndorsementId() const {
  return endorsement.id;
}

bool operator==(const PartialVTB& a, const PartialVTB& b) {
  bool is_equal = a.btc.size() == b.btc.size();
  for (size_t i = 0; i < a.btc.size() && is_equal; ++i) {
    if (*a.btc[i] != *b.btc[i]) {
      is_equal = false;
    }
  }
  return is_equal && a.endorsement == b.endorsement &&
         *a.containing == *b.containing;
}

}  // namespace altintegration

#include "veriblock/mempool_relations.hpp"

namespace altintegration {

PopData VbkPayloadsRelations::toPopData() const {
  PopData pop;
  pop.context.push_back(*header);
  for (const auto& vtb : vtbs) {
    pop.vtbs.push_back(*vtb);
  }

  for (const auto& atv : atvs) {
    pop.atvs.push_back(*atv);
  }

  VBK_ASSERT(
      std::all_of(pop.vtbs.begin(), pop.vtbs.end(), [this](const VTB& a) {
        return a.containingBlock == *header;
      }));

  // all VTBs have same VBK containing block, sort them based on time of
  // earliest BTC block (based on time).
  // note: use stable sort to preserve relative insertion order to mempool
  std::stable_sort(
      pop.vtbs.begin(), pop.vtbs.end(), [](const VTB& a, const VTB& b) {
        auto& earliestA = a.transaction.blockOfProofContext.empty()
                              ? a.transaction.blockOfProof
                              : a.transaction.blockOfProofContext.front();
        auto& earliestB = b.transaction.blockOfProofContext.empty()
                              ? b.transaction.blockOfProof
                              : b.transaction.blockOfProofContext.front();
        return earliestA.timestamp < earliestB.timestamp;
      });

  return pop;
}

void VbkPayloadsRelations::removeVTB(const VTB::id_t& vtb_id) {
  auto it = std::find_if(
      vtbs.begin(), vtbs.end(), [&vtb_id](const std::shared_ptr<VTB>& vtb) {
        return vtb->getId() == vtb_id;
      });

  if (it != vtbs.end()) {
    vtbs.erase(it);
  }
}

void VbkPayloadsRelations::removeATV(const ATV::id_t& atv_id) {
  auto it = std::find_if(
      atvs.begin(), atvs.end(), [&atv_id](const std::shared_ptr<ATV>& atv) {
        return atv->getId() == atv_id;
      });

  if (it != atvs.end()) {
    atvs.erase(it);
  }
}

}  // namespace altintegration

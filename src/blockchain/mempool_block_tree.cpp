#include "veriblock/blockchain/blockchain_util.hpp"
#include "veriblock/blockchain/mempool_block_tree.hpp"
#include "veriblock/keystone_util.hpp"

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

  // we sort VTBs in ascending order of their containing VBK blocks to guarantee
  // that within a single block they all are connected.
  std::sort(pop.vtbs.begin(), pop.vtbs.end(), [](const VTB& a, const VTB& b) {
    return a.containingBlock.height < b.containingBlock.height;
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

bool MemPoolBlockTree::areStronglyEquivalent(const ATV& atv1, const ATV& atv2) {
  return atv1.transaction.getHash() == atv2.transaction.getHash() &&
         atv1.blockOfProof.getHash() == atv2.blockOfProof.getHash();
}

bool MemPoolBlockTree::areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  return (vtb1.transaction.bitcoinTransaction ==
          vtb2.transaction.bitcoinTransaction) &&
         (vtb1.transaction.blockOfProof == vtb2.transaction.blockOfProof);
}

bool MemPoolBlockTree::areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2) {
  bool is_on_the_same_btc_chain = areOnSameChain(vtb1.transaction.blockOfProof,
                                                 vtb2.transaction.blockOfProof,
                                                 tree_.btc());

  bool is_on_the_same_vbk_chain =
      areOnSameChain(vtb1.transaction.publishedBlock,
                     vtb2.transaction.publishedBlock,
                     tree_.vbk());

  bool are_the_same_keystone_period =
      areOnSameKeystoneInterval(vtb1.transaction.publishedBlock.height,
                                vtb2.transaction.publishedBlock.height,
                                tree_.vbk().getParams().getKeystoneInterval());

  return ((vtb1.transaction.publishedBlock ==
           vtb2.transaction.publishedBlock) &&
          is_on_the_same_btc_chain) ||
         (is_on_the_same_vbk_chain && are_the_same_keystone_period &&
          is_on_the_same_btc_chain);
}

int MemPoolBlockTree::weaklyCompare(const VTB& vtb1, const VTB& vtb2) {
  VBK_ASSERT_MSG(areWeaklyEquivalent(vtb1, vtb2),
                 "vtbs should be weakly equivalent");

  auto* blockOfProof1 =
      tree_.btc().getBlockIndex(vtb1.transaction.blockOfProof.getHash());
  auto* blockOfProof2 =
      tree_.btc().getBlockIndex(vtb2.transaction.blockOfProof.getHash());

  VBK_ASSERT_MSG(blockOfProof1,
                 "unknown block %s",
                 vtb1.transaction.blockOfProof.toPrettyString());
  VBK_ASSERT_MSG(blockOfProof2,
                 "unknown block %s",
                 vtb2.transaction.blockOfProof.toPrettyString());

  if (blockOfProof1->getHeight() > blockOfProof2->getHeight()) {
    return -1;
  }

  if (blockOfProof1->getHeight() < blockOfProof2->getHeight()) {
    return 1;
  }

  if (vtb1.transaction.publishedBlock.height >
      vtb2.transaction.publishedBlock.height) {
    return -1;
  }

  if (vtb1.transaction.publishedBlock.height <
      vtb2.transaction.publishedBlock.height) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() >
      vtb2.transaction.blockOfProofContext.size()) {
    return 1;
  }

  if (vtb1.transaction.blockOfProofContext.size() <
      vtb2.transaction.blockOfProofContext.size()) {
    return -1;
  }

  VBK_ASSERT_MSG(
      areStronglyEquivalent(vtb1, vtb2),
      "if we can not define the best vtb they should be strongly equivalent");
  return 0;
}

}  // namespace altintegration
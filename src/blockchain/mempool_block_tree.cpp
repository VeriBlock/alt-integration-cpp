#include "veriblock/blockchain/mempool_block_tree.hpp"

namespace altintegration {

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
  bool is_on_the_same_btc_chain = tree_.btc().areOnSameChain(
      vtb1.transaction.blockOfProof, vtb2.transaction.blockOfProof);

  bool is_on_the_same_vbk_chain = tree_.vbk().areOnSameChain(
      vtb1.transaction.publishedBlock, vtb2.transaction.publishedBlock);

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
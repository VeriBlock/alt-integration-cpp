#include "veriblock/blockchain/mempool_block_tree.hpp"

namespace altintegration {

typename VbkPayloadsRelations::height_t VbkPayloadsRelations::getHeight()
    const {
  return this->header->height;
}

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

const VbkBlock* VbkPayloadsRelations::getAncestor(
    typename VbkPayloadsRelations::height_t height) const {
  if (height < 0 || height > header->height) {
    return nullptr;
  }

  VbkPayloadsRelations* relation_index =
      const_cast<VbkPayloadsRelations*>(this);
  BlockIndex<VbkBlock>* block_index = nullptr;
  while (block_index != nullptr || relation_index != nullptr) {
    if ((relation_index != nullptr && relation_index->getHeight() > height) ||
        (block_index != nullptr && block_index->getHeight() > height)) {
      if (relation_index->relation_pprev_ != nullptr) {
        VBK_ASSERT_MSG(
            relation_index->block_pprev_ == nullptr && block_index == nullptr,
            "block pprev ptr should be equal to nullptr while "
            "relation pprev ptr is defined");

        relation_index = relation_index->relation_pprev_;
      } else {
        VBK_ASSERT_MSG(relation_index->relation_pprev_ == nullptr &&
                           relation_index == nullptr,
                       "relation pprev ptr should be equal to nullptr while "
                       "block pprev ptr is defined");

        relation_index = nullptr;
        block_index = relation_pprev_->block_pprev_;
      }
    } else if ((relation_index != nullptr &&
                relation_index->getHeight() == height) ||
               (block_index != nullptr && block_index->getHeight() == height)) {
      if (relation_index->relation_pprev_ != nullptr) {
        VBK_ASSERT_MSG(
            relation_index->block_pprev_ == nullptr && block_index == nullptr,
            "block pprev ptr should be equal to nullptr while "
            "relation pprev ptr is defined");

        return relation_index->header.get();
      } else {
        VBK_ASSERT_MSG(relation_index->relation_pprev_ == nullptr &&
                           relation_index == nullptr,
                       "relation pprev ptr should be equal to nullptr while "
                       "block pprev ptr is defined");

        return &block_index->getHeader();
      }
    } else {
      return nullptr;
    }
  }

  return nullptr;
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

VbkPayloadsRelations* MemPoolBlockTree::getVbkRelationIndex(
    const typename VbkBlock::hash_t& hash) const {
  auto trimmed_hash = hash.trimLE<VbkBlock::prev_hash_t::size()>();
  auto it = vbk_blocks_.find(trimmed_hash);
  return it == vbk_blocks_.end() ? nullptr : it->second.get();
}

bool MemPoolBlockTree::areOnSameChain(const VbkBlock& blk1,
                                      const VbkBlock& blk2) const {
  auto hash1 = blk1.getHash();
  auto hash2 = blk2.getHash();

  BlockIndex<VbkBlock>* blk_index1 = nullptr;
  BlockIndex<VbkBlock>* blk_index2 = nullptr;
  VbkPayloadsRelations* vbk_relation1 = getVbkRelationIndex(hash1);
  VbkPayloadsRelations* vbk_relation2 = getVbkRelationIndex(hash2);

  if (vbk_relation1 == nullptr) {
    blk_index1 = tree_.vbk().getBlockIndex(hash1);
    VBK_ASSERT_MSG(blk_index1, "unknown block %s", blk1.toPrettyString());
  }

  if (vbk_relation2 == nullptr) {
    blk_index2 = tree_.vbk().getBlockIndex(hash2);
    VBK_ASSERT_MSG(blk_index2, "unknown block %s", blk2.toPrettyString());
  }

  if (blk1.height > blk2.height) {
    auto* ancestor = vbk_relation1 != nullptr
                         ? vbk_relation1->getAncestor(blk2.height)
                         : &blk_index1->getAncestor(blk2.height)->getHeader();
    return ancestor->getHash() == hash2;
  } else {
    VbkBlock* ancestor = nullptr;
    if (vbk_relation1 != nullptr) {
      ancestor = vbk_relation1->getAncestor(blk2.height);
    }
    auto* ancestor = vbk_relation2 != nullptr
                         ? vbk_relation2->getAncestor(blk1.height)
                         : &blk_index2->getAncestor(blk1.height)->getHeader();
    return ancestor->getHash() == hash1;
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
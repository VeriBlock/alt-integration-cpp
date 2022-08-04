#include <veriblock/pop/mempool_relations.hpp>

#include "veriblock/pop/blockchain/alt_block_tree.hpp"
#include "veriblock/pop/blockchain/alt_chain_params.hpp"
#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/entities/atv.hpp"
#include "veriblock/pop/entities/coin.hpp"
#include "veriblock/pop/entities/publication_data.hpp"
#include "veriblock/pop/entities/vbktx.hpp"

namespace altintegration {

int VbkPayloadsRelations::TxFeeComparator::operator()(
    const std::shared_ptr<ATV>& a, const std::shared_ptr<ATV>& b) const {
  auto aFee = a->transaction.calculateTxFee();
  auto bFee = b->transaction.calculateTxFee();
  if (aFee.units == bFee.units) return 0;
  if (aFee.units > bFee.units) return 1;
  return -1;
}

int VbkPayloadsRelations::EndorsedAltComparator::operator()(
    const std::shared_ptr<ATV>& a, const std::shared_ptr<ATV>& b) const {
  auto endorsedHash =
      tree_.getParams().getHash(a->transaction.publicationData.header);
  auto* aEndorsedIndex = tree_.getBlockIndex(endorsedHash);
  endorsedHash =
      tree_.getParams().getHash(b->transaction.publicationData.header);
  auto* bEndorsedIndex = tree_.getBlockIndex(endorsedHash);
  if (aEndorsedIndex == bEndorsedIndex) return 0;
  if (aEndorsedIndex == nullptr) return -1;
  if (bEndorsedIndex == nullptr) return 1;
  if (aEndorsedIndex->getHeight() == bEndorsedIndex->getHeight()) return 0;
  if (aEndorsedIndex->getHeight() < bEndorsedIndex->getHeight()) return 1;
  return -1;
}

bool VbkPayloadsRelations::AtvCombinedComparator::operator()(
    const std::shared_ptr<ATV>& a, const std::shared_ptr<ATV>& b) const {
  auto ret1 = txFeeComparator(a, b);
  if (ret1 != 0) return ret1 > 0;
  auto ret2 = endorsedAltComparator(a, b);
  if (ret2 != 0) return ret2 > 0;
  return a < b;
}

}  // namespace altintegration

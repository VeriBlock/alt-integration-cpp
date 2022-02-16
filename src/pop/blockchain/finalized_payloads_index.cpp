#include <veriblock/pop/blockchain/finalized_payloads_index.hpp>

namespace altintegration {

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<AltBlock>>& pl,
                               const BlockIndex<AltBlock>& index) {
  VBK_ASSERT_MSG(index.finalized, index.toPrettyString());

  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<ATV>()) {
    pl.add(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.add(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VbkBlock>()) {
    pl.add(id.asVector(), hash);
  }
}

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<VbkBlock>>& pl,
                               const BlockIndex<VbkBlock>& index) {
  VBK_ASSERT_MSG(index.finalized, index.toPrettyString());

  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.add(id.asVector(), hash);
  }
}

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<BtcBlock>>&,
                               const BlockIndex<BtcBlock>& index) {
  VBK_ASSERT_MSG(index.finalized, index.toPrettyString());
  // intentionally do nothing, as BTC blocks do not store any payloads
}

}  // namespace altintegration

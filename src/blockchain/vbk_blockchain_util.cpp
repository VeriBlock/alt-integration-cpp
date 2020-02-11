#include <veriblock/blockchain/vbk_blockchain_util.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace VeriBlock {

///TODO: https://veriblock.atlassian.net/browse/BTC-219
template <>
VbkBlock Miner<VbkBlock, VbkChainParams>::getBlockTemplate(
    const BlockIndex<VbkBlock>& tip, const merkle_t& merkle) const {
  (void)tip;
  (void)merkle;
  VbkBlock block{};
  return block;
}

///TODO: https://veriblock.atlassian.net/browse/BTC-220
template <>
void determineBestChain(Chain<VbkBlock>& currentBest,
                        BlockIndex<VbkBlock>& indexNew) {
  (void)currentBest;
  (void)indexNew;
}

///TODO: https://veriblock.atlassian.net/browse/BTC-221
// copied from BTC
template <>
uint32_t getNextWorkRequired(const BlockIndex<VbkBlock>& currentTip,
                             const VbkBlock& block,
                             const VbkChainParams& params) {
  (void)currentTip;
  (void)block;
  (void)params;
  return 0;
}

}  // namespace VeriBlock

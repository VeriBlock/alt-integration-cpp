#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_

#include <cassert>
#include <memory>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace VeriBlock {

void determineBestChain(Chain<BtcBlock>& currentBest,
                        BlockIndex<BtcBlock>& indexNew);


// void determineBestChain(std::shared_ptr<BlockIndex<VbkBlock>>*
// pindexBestHeader,
//                        const BlockIndex<VbkBlock>& indexNew) {
//  (void)pindexBestHeader;
//  (void)indexNew;
//  assert(false && "TODO: implement fork resolution for VBK");
//}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_

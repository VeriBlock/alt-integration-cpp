#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_

#include <cassert>
#include <memory>
#include <veriblock/blockchain/fork_resolution.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace VeriBlock {

template <>
void determineBestChain(Chain<BtcBlock>& currentBest,
                        BlockIndex<BtcBlock>& indexNew);


}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_

#include <veriblock/blockchain/blockchain.hpp>
#include <veriblock/blockchain/fork_resolution.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace VeriBlock {

void determineBestChain(Chain<BtcBlock>& currentBest,
                        BlockIndex<BtcBlock>& indexNew) {
  if (currentBest.tip() == nullptr ||
      currentBest.tip()->chainWork < indexNew.chainWork) {
    currentBest.setTip(&indexNew);
  }
}

///// contains explicit instantiations
//template struct Blockchain<BtcBlock>;

}  // namespace VeriBlock
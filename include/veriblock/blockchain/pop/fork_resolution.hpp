#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

#include <set>
#include <vector>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace VeriBlock {

struct ProtoKeystoneContext {
  int vbkBlockHeight;
  uint32_t timestampOfEndorsedBlock;

  ProtoKeystoneContext(int height, int time)
      : vbkBlockHeight(height), timestampOfEndorsedBlock(time) {}

  // A map of the Bitcoin blocks which reference (endorse a block header which
  // is the represented block or references the represented block) to the index
  // of the earliest
  std::set<BlockIndex<BtcBlock>*> referencedByBtcBlocks;
};

struct KeystoneContext {
  int vbkBlockHeight;
  int firstBtcBlockPublicationHeight;
};

struct ComparePopScore {
  explicit ComparePopScore(uint32_t keystoneInt)
      : keystoneInterval(keystoneInt) {
    assert(keystoneInterval > 0);
  }

  int operator()(const std::vector<KeystoneContext>& chainA,
                 const std::vector<KeystoneContext>& chainB);

 private:
  int keystoneInterval;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_FORK_RESOLUTION_HPP_

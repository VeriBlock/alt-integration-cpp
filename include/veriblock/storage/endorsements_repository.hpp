#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_ENDORSEMENTS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_ENDORSEMENTS_REPOSITORY_HPP_

#include <functional>
#include <string>
#include <vector>

#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"

namespace VeriBlock {

struct Endorsement {
  // The unique key that identifies this endorsement
  std::string endorsementKey;
  // The hash of the VeriBlock block that this DefaultEndorsement endorses
  VbkBlock::hash_t endorsedVbkHash;
  // The hash of the VeriBlock block that contains this DefaultEndorsement
  std::vector<VbkBlock::hash_t> containingVbkHashes;
  // The Bitcoin block header corresponding to the DefaultEndorsement
  // publication transaction
  BtcBlock::hash_t blockOfProof;
  // The PoP miner who mined the endorsement
  std::string popMinerAddress;
};

struct EndorsementsRepository {
  virtual ~EndorsementsRepository() = default;

  using chainContainsBlockFunc =
      std::function<bool(const VbkBlock::hash_t& hash)>;

  virtual void put(const VbkPopTx& popTx,
                   const VbkBlock::hash_t& containingBlockHash) = 0;

  virtual void clear() = 0;

  virtual std::vector<Endorsement> getEndorsementsInChain(
      const VbkBlock::hash_t& endorsedBlockHash,
      chainContainsBlockFunc func) const = 0;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_ENDORSEMENTS_REPOSITORY_HPP_

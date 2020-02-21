#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_ENDORSEMENTS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_ENDORSEMENTS_REPOSITORY_INMEM_HPP_

#include <unordered_map>

#include "veriblock/storage/endorsements_repository.hpp"

namespace VeriBlock {

struct EndorsementsRepositoryInmem : public EndorsementsRepository {
  ~EndorsementsRepositoryInmem() override = default;

  EndorsementsRepositoryInmem() {}

  void put(const VbkPopTx& popTx,
           const VbkBlock::hash_t& containingBlockHash) override;

  void clear() override;

  std::vector<Endorsement> getEndorsementsInChain(
      const VbkBlock::hash_t& endorsedBlockHash,
      chainContainsBlockFunc func) const override;

 private:
  std::unordered_map<VbkBlock::hash_t, std::vector<Endorsement>> endorsed_map;
};
}  // namespace VeriBlock

#endif

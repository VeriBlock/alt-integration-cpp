#include "veriblock/storage/endorsements_repository_inmem.hpp"

using namespace VeriBlock;

void EndorsementsRepositoryInmem::put(
    const VbkPopTx& popTx, const VbkBlock::hash_t& containingBlockHash) {
  VbkBlock::hash_t endorsedBlockHash = popTx.publishedBlock.getHash();
  std::string popTxHash = popTx.getHash().asString();

  std::vector<Endorsement>& endrs = endorsed_map[endorsedBlockHash];
  auto endorsement = endrs.begin();
  for (; endorsement != endrs.end(); ++endorsement) {
    if (endorsement->endorsementKey == popTxHash) {
      endorsement->containingVbkHashes.push_back(containingBlockHash);
      break;
    }
  }
  if (endorsement == endrs.end()) {
    endrs.push_back({
        popTx.getHash(),
        popTx.publishedBlock.getHash(),
        {containingBlockHash},
        popTx.blockOfProof.getHash(),
        popTx.address.toString(),
    });
  }
}

void EndorsementsRepositoryInmem::clear() { endorsed_map.clear(); }

std::vector<Endorsement> EndorsementsRepositoryInmem::getEndorsementsInChain(
    const VbkBlock::hash_t& endorsedBlockHash,
    chainContainsBlockFunc func) const {
  std::vector<Endorsement> result;

  auto it = endorsed_map.find(endorsedBlockHash);

  if (it == endorsed_map.end()) {
    return result;
  }

  for (const auto& endorsement : it->second) {
    for (const auto& containingHashes : endorsement.containingVbkHashes) {
      if (func(containingHashes)) {
        result.push_back(endorsement);
        break;
      }
    }
  }

  return result;
}

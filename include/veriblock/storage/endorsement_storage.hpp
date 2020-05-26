#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_

#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/blockchain/command_group.hpp>

namespace altintegration {

template <typename Endorsement>
class EndorsementStorage {
  using endorsement_t = Endorsement;
  //using block_t = typename Endorsement::containingBlock;
  using containing_hash_t = typename decltype(Endorsement::containingBlock)::hash_t;

  virtual bool removeEndorsement(containing_hash_t containingBlock, endorsement_t id) {
    (void)containingBlock;
    (void)id;
    return false;
  }

  virtual std::vector<CommandGroup> getCommands(containing_hash_t containingBlock) {
    (void)containingBlock;
    return {};
  }

  //virtual bool getPayloads(id, Payloads&);
  //virtual void addPayloads(containing_hash_t hash, Payloads);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
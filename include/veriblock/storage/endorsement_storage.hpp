#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_

#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/commands/addblock.hpp>

namespace altintegration {

template <typename Endorsement, typename Tree>
class EndorsementStorage {
  using endorsement_t = Endorsement;
  using block_t = typename Tree::block_t;
  using eid_t = typename Endorsement::id_t;
  using containing_hash_t = typename block_t::hash_t;

 public:
  virtual ~EndorsementStorage() = default;

  virtual bool removeEndorsement(containing_hash_t containingBlock, eid_t id) {
    Endorsement payloads;
    bool found = getPayloads(id, payloads);
    if (found) {
      endorsements.erase(id);
    }

    auto it = containingEndorsements.find(containingBlock);
    if (it == containingEndorsements.end()) {
      return false;
    }

    auto ids = it->second;
    ids.erase(id);
    containingEndorsements[containingBlock] = ids;
    return true;
  }

  virtual void payloadsToCommands(Tree& tree,
                          const Endorsement& p,
                          std::vector<CommandPtr>& commands) {
    tree.payloadsToCommands(p, commands);
  }

  virtual std::vector<CommandGroup> getCommands(
      Tree& tree, containing_hash_t containingBlock) {
    std::vector<CommandGroup> out{};
    auto it = containingEndorsements.find(containingBlock);
    if (it == containingEndorsements.end()) {
      return out;
    }
    auto ids = it->second;
    for (const eid_t& id : ids) {
      auto eit = endorsements.find(id);
      if (eit == endorsements.end()) {
        continue;
      }

      std::vector<CommandPtr> commands{};
      payloadsToCommands(tree, eit->second, commands);
      CommandGroup group{id, commands};
      out.push_back(group);
    }
    return out;
  }

  virtual bool getPayloads(eid_t id, Endorsement& payloads) {
    auto it = endorsements.find(id);
    if (it == endorsements.end()) {
      return false;
    }
    payloads = it->second;
    return true;
  }

  virtual void addPayloads(containing_hash_t containingBlock,
                           const Endorsement& payloads) {
    auto id = payloads.getId();
    endorsements[id] = payloads;
    auto ids = containingEndorsements[containingBlock];
    ids.insert(id);
    containingEndorsements[containingBlock] = ids;
  }

  private:
   std::unordered_map<eid_t, endorsement_t> endorsements;
   std::unordered_map<containing_hash_t, std::set<eid_t>> containingEndorsements;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_ENDORSEMENT_STORAGE_HPP_
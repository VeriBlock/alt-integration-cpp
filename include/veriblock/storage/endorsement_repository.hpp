#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_HPP_

#include <functional>
#include <string>
#include <vector>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/storage/cursor.hpp"

namespace VeriBlock {

template <typename Endorsement>
struct EndorsementRepository {
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;
  using cursor_t = Cursor<eid_t, Endorsement>;

  virtual ~EndorsementRepository() = default;

  virtual void remove(const eid_t& id) = 0;
  virtual void remove(const container_t& container) = 0;

  virtual void put(const container_t& container) = 0;

  virtual void put(const endorsement_t& container) = 0;

  virtual std::vector<endorsement_t> get(
      const endorsed_hash_t& endorsedBlockHash) const = 0;

  virtual std::shared_ptr<cursor_t> newCursor() const = 0;
};

template <typename Endorsement>
void endorsementRepositoryCopy(
    const EndorsementRepository<Endorsement>& copy_from,
    EndorsementRepository<Endorsement>& copy_to) {
  auto cursor = copy_from.newCursor();

  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    copy_to.put(cursor->value());
  }
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_HPP_

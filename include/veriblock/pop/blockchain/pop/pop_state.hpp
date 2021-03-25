// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STATE_HPP
#define VERIBLOCK_POP_CPP_POP_STATE_HPP

#include <map>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

//! @private
template <typename EndorsementT>
struct PopState {
  using endorsement_t = EndorsementT;
  using eid_t = typename endorsement_t::id_t;
  using containing_endorsement_store_t =
      std::multimap<eid_t, std::shared_ptr<endorsement_t>>;

  //! (memory-only) list of endorsements pointing to this block.
  // must be a vector, because we can have duplicates here
  std::vector<const endorsement_t*> endorsedBy;

  const containing_endorsement_store_t& getContainingEndorsements() const {
    return _containingEndorsements;
  }

  const std::vector<const endorsement_t*>& getEndorsedBy() const {
    return endorsedBy;
  }

  void insertContainingEndorsement(std::shared_ptr<endorsement_t> e) {
    VBK_ASSERT_MSG(e != nullptr, "Inserted endorsement should not be nullptr");
    _containingEndorsements.emplace(e->id, std::move(e));
    setDirty();
  }

  const typename containing_endorsement_store_t::const_iterator
  findContainingEndorsement(const eid_t& id) const {
    return _containingEndorsements.lower_bound(id);
  }

  void removeContainingEndorsement(
      const typename containing_endorsement_store_t::const_iterator it) {
    _containingEndorsements.erase(it);
    setDirty();
  }

  // hide setters from public usage
 protected:
  //! (stored as vector) list of containing endorsements in this block
  containing_endorsement_store_t _containingEndorsements{};

  void setDirty();

  void setNull() {
    _containingEndorsements.clear();
    endorsedBy.clear();
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STATE_HPP

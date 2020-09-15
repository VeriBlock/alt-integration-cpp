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
#include <veriblock/serde.hpp>
#include <veriblock/uint.hpp>

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
  std::vector<endorsement_t*> endorsedBy;

  const containing_endorsement_store_t& getContainingEndorsements() const {
    return _containingEndorsements;
  }

  const std::vector<endorsement_t*>& getEndorsedBy() const {
    return endorsedBy;
  }

  void insertContainingEndorsement(std::shared_ptr<endorsement_t> e) {
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

  void toRaw(WriteStream& stream) const {
    // write containingEndorsements as vector
    writeContainer<decltype(_containingEndorsements)>(
        stream,
        _containingEndorsements,
        [](WriteStream& w,
           const typename decltype(_containingEndorsements)::value_type& endorsement) {
          endorsement.second->toVbkEncoding(w);
        });
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

  void initAddonFromRaw(ReadStream& stream) {
    // read containingEndorsements as vector
    auto endorsements = readArrayOf<endorsement_t>(
        stream, [](ReadStream& r) { return endorsement_t::fromVbkEncoding(r); });

    for (auto& endorsement : endorsements) {
      auto pair = _containingEndorsements.emplace(
          endorsement.getId(), std::make_shared<endorsement_t>(endorsement));
      VBK_ASSERT(pair->second);
    }
    // do not restore 'endorsedBy', it will be done later
  }

  void initAddonFromOther(const PopState& other) {
    _containingEndorsements = other._containingEndorsements;
    endorsedBy = other.endorsedBy;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STATE_HPP

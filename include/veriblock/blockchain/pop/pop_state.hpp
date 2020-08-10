// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STATE_HPP
#define VERIBLOCK_POP_CPP_POP_STATE_HPP

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <veriblock/serde.hpp>

namespace altintegration {

template <typename EndorsementT>
struct PopState {
  using endorsement_t = EndorsementT;
  using eid_t = typename endorsement_t::id_t;
  using containing_endorsement_store_t =
      std::unordered_multimap<eid_t, std::shared_ptr<endorsement_t>>;

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
    _containingEndorsements.insert(std::make_pair(e->id, std::move(e)));
    setDirty();
  }

  void removeContainingEndorsement(std::shared_ptr<endorsement_t> e) {
    auto range = _containingEndorsements.equal_range(e->id);
    auto endorsement_it = std::find_if(
        range.first,
        range.second,
        [&](const typename containing_endorsement_store_t::reference item) {
          return *item.second == *e;
        });

    VBK_ASSERT(endorsement_it != range.second &&
               "state corruption: tried to remove an endorsement that the "
               "block doesn't contain");
    _containingEndorsements.erase(endorsement_it);

    setDirty();
  }

  void toRaw(WriteStream& w) const {
    // write containingEndorsements as vector
    writeContainer<decltype(_containingEndorsements)>(
        w,
        _containingEndorsements,
        [](WriteStream& W,
           const typename decltype(_containingEndorsements)::value_type& e) {
          e.second->toVbkEncoding(W);
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

  void initAddonFromRaw(ReadStream& r) {
    // read containingEndorsements as vector
    auto v = readArrayOf<endorsement_t>(
        r, [](ReadStream& r) { return endorsement_t::fromVbkEncoding(r); });

    for (auto& e : v) {
      auto pair = _containingEndorsements.insert(
          {e.getId(), std::make_shared<endorsement_t>(e)});
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

// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STATE_HPP
#define VERIBLOCK_POP_CPP_POP_STATE_HPP

#include <map>
#include <memory>
#include <set>
#include <vector>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

//! @private
template <typename EndorsementT>
struct PopState {
  using endorsement_t = EndorsementT;
  using eid_t = typename endorsement_t::id_t;
  using containing_endorsement_store_t =
      std::multimap<eid_t, std::shared_ptr<endorsement_t>>;

  const containing_endorsement_store_t& getContainingEndorsements() const {
    return _containingEndorsements;
  }

  const std::vector<const endorsement_t*>& getEndorsedBy() const {
    return _endorsedBy;
  }

  void insertEndorsedBy(const endorsement_t* e) {
    VBK_ASSERT_MSG(e != nullptr, "Inserted endorsement should not be nullptr");
    _endorsedBy.push_back(e);
  }

  bool eraseLastFromEndorsedBy(const endorsement_t* endorsement) {
    auto rm = [&endorsement](const endorsement_t* e) -> bool {
      return e == endorsement;
    };
    return erase_last_item_if<const endorsement_t*>(_endorsedBy, rm);
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

  void toVbkEncoding(WriteStream& stream) const {
    // write containingEndorsements as vector
    using value_t = typename decltype(_containingEndorsements)::value_type;
    writeContainer<decltype(_containingEndorsements)>(
        stream,
        _containingEndorsements,
        [&](WriteStream&, const value_t& endorsement) {
          endorsement.second->toVbkEncoding(stream);
        });
  }

  // hide setters from public usage
 protected:
  //! (stored as vector) list of containing endorsements in this block
  containing_endorsement_store_t _containingEndorsements{};

  //! (memory-only) list of endorsements pointing to this block.
  // must be a vector, because we can have duplicates here
  std::vector<const endorsement_t*> _endorsedBy;

  void setDirty();

  void setNull() {
    _containingEndorsements.clear();
    _endorsedBy.clear();
  }

  template <typename T>
  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         PopState<T>& out,
                                         ValidationState& state);
};

//! @overload
template <typename T>
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopState<T>& out,
                                ValidationState& state) {
  std::vector<T> endorsements;
  auto max = std::max(MAX_POPDATA_ATV, MAX_POPDATA_VTB);
  if (!readArrayOf<T>(
          stream,
          endorsements,
          state,
          0,
          max,
          [](ReadStream& stream, T& t, ValidationState& state) -> bool {
            return DeserializeFromVbkEncoding(stream, t, state);
          })) {
    return state.Invalid("popstate-bad-endorsement");
  }

  for (const auto& endorsement : endorsements) {
    auto it = out._containingEndorsements.emplace(
        endorsement.getId(), std::make_shared<T>(endorsement));
    // newly created endorsement must not be null
    VBK_ASSERT(it->second);
  }
  // do not restore 'endorsedBy' here, it will be done later during tree
  // loading
  return true;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STATE_HPP

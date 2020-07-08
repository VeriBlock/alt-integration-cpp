// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STATE_HPP
#define VERIBLOCK_POP_CPP_POP_STATE_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <veriblock/serde.hpp>

namespace altintegration {

template <typename EndorsementT>
struct PopState {
  using endorsement_t = EndorsementT;
  using eid_t = typename endorsement_t::id_t;

  //! (stored as vector) list of containing endorsements in this block
  std::unordered_map<eid_t, std::shared_ptr<endorsement_t>>
      containingEndorsements{};

  //! (memory-only) list of endorsements pointing to this block
  std::vector<endorsement_t*> endorsedBy;

  void setNull() {
    containingEndorsements.clear();
    endorsedBy.clear();
  }

  void toRaw(WriteStream& w) const {
    // write containingEndorsements as vector
    writeContainer<decltype(containingEndorsements)>(
        w,
        containingEndorsements,
        [](WriteStream& W,
           const typename decltype(containingEndorsements)::value_type& e) {
          e.second->toVbkEncoding(W);
        });
  }

  void initFromRaw(ReadStream& r) {
    // read containingEndorsements as vector
    auto v = readArrayOf<endorsement_t>(
        r, [](ReadStream& r) { return endorsement_t::fromVbkEncoding(r); });

    for (auto& e : v) {
      auto pair = containingEndorsements.insert(
          {e.getId(), std::make_shared<endorsement_t>(e)});
      VBK_ASSERT(pair.second);
    }

    // do not restore 'endorsedBy', it will be done later
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STATE_HPP

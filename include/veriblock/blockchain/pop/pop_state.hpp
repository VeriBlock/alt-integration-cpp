// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STATE_HPP
#define VERIBLOCK_POP_CPP_POP_STATE_HPP

#include <memory>
#include <unordered_map>
#include <vector>

namespace altintegration {

template <typename EndorsementT>
struct PopState {
  using endorsement_t = EndorsementT;
  using eid_t = typename endorsement_t::id_t;

  //! list of containing endorsements in this block
  std::unordered_map<eid_t, std::shared_ptr<endorsement_t>>
      containingEndorsements{};

  //! list of endorsements pointing to this block
  std::vector<endorsement_t *> endorsedBy;

  void setNull() {
    containingEndorsements.clear();
    endorsedBy.clear();
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STATE_HPP

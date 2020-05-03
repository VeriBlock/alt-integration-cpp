// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

//  Protecting Chain:
//    A - B - C - D - E - F - G - H - I - J
//                       /
//                    block of proof
//  Protected Chain:
//    A - B - C - D - E - F - G - H - I - J
//           /                /
//    endorsed hash      containing hash
//
// "endorsed hash" is a block hash that is endorsed by POP TX.
// "block of proof" is a block from protecting chain that contains a tx with
// proof. "containing hash" is a block which contains POP TX which endorses
// "endorsed block"

template <class EndorsedHash,
          class ContainingHash,
          class Container,
          class EndorsedBlockHeight>
struct Endorsement {
  using type =
      Endorsement<EndorsedHash, ContainingHash, Container, EndorsedBlockHeight>;
  using id_t = uint256;
  using endorsed_hash_t = EndorsedHash;
  using containing_hash_t = ContainingHash;
  using container_t = Container;
  using endorsed_height_t = EndorsedBlockHeight;

  // The unique key that identifies this endorsement
  id_t id;
  EndorsedHash endorsedHash;
  EndorsedBlockHeight endorsedHeight;
  EndorsedHash containingHash;
  ContainingHash blockOfProof;
  std::vector<uint8_t> payoutInfo;

  static Endorsement fromVbkEncoding(std::string) {
    // TODO: remove
    return {};
  }

  std::vector<uint8_t> toVbkEncoding() const {
    // TODO: remove
    return {};
  }

  // TODO(Bogdan): temporary used to disable duplicate check
  static bool checkForDuplicates;

  static type fromContainer(const Container& c);

  static type::id_t getId(const Container& c);

  bool operator==(const type& other) const { return id == other.id; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

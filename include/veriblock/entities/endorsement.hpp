#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

#include <veriblock/entities/payloads.hpp>
#include <veriblock/uint.hpp>

namespace VeriBlock {

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

template <class EndorsedHash, class ContainingHash, class Container>
struct Endorsement {
  using type = Endorsement<EndorsedHash, ContainingHash, Container>;
  using id_t = uint256;
  using endorsed_hash_t = EndorsedHash;
  using containing_hash_t = ContainingHash;
  using container_t = Container;

  // The unique key that identifies this endorsement
  id_t id;
  EndorsedHash endorsedHash;
  EndorsedHash containingHash;
  ContainingHash blockOfProof;

  static type fromContainer(const Container& c);

  static type::id_t getId(const Container& c);

  bool operator==(const type& other) const { return id == other.id; }
};

// endorsement of VBK blocks in BTC
using BtcEndorsement = Endorsement<VbkBlock::hash_t, BtcBlock::hash_t, VTB>;
template <>
BtcEndorsement BtcEndorsement ::fromContainer(const VTB& c);
template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c);

// endorsement of ALT blocks in VBK
using VbkEndorsement =
    Endorsement<std::vector<uint8_t>, VbkBlock::hash_t, AltProof>;
template <>
VbkEndorsement VbkEndorsement ::fromContainer(const AltProof& c);
template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltProof& c);

}  // namespace VeriBlock

namespace std {

template <>
struct hash<VeriBlock::BtcEndorsement> {
  size_t operator()(const VeriBlock::BtcEndorsement& e) const {
    using eid = VeriBlock::BtcEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

template <>
struct hash<VeriBlock::VbkEndorsement> {
  size_t operator()(const VeriBlock::VbkEndorsement& e) const {
    using eid = VeriBlock::VbkEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

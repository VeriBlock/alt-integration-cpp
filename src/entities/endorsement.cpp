#include <veriblock/entities/endorsement.hpp>

namespace VeriBlock {

template <>
BtcEndorsement BtcEndorsement::fromContainer(const VTB& c) {
  BtcEndorsement e;
  e.id = BtcEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
  return e;
}

template <>
VbkEndorsement VbkEndorsement::fromContainer(const AltProof& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.containingHash = c.containing.hash;
  return e;
}

template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c) {
  return c.transaction.getHash();
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltProof& c) {
  return c.atv.transaction.getHash();
}

}  // namespace VeriBlock
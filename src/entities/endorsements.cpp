// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

template <>
VbkEndorsement VbkEndorsement::fromContainer(const VTB& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
  e.endorsedHeight = c.transaction.publishedBlock.height;
  e.payoutInfo = {};
  return e;
}

template <>
AltEndorsement AltEndorsement::fromContainer(const AltPayloads& c) {
  AltEndorsement e;
  e.id = AltEndorsement::getId(c);
  e.blockOfProof = c.popData.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.endorsedHeight = c.endorsed.height;
  e.containingHash = c.containingBlock.hash;
  e.payoutInfo = c.popData.atv.transaction.publicationData.payoutInfo;
  return e;
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const VTB& c) {
  return c.getId();
}

template <>
AltEndorsement::id_t AltEndorsement::getId(const AltPayloads& c) {
  return c.popData.atv.getId();
}

}  // namespace altintegration

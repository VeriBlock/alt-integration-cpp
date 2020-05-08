// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

template <>
BtcEndorsement BtcEndorsement::fromContainer(const VTB& c) {
  BtcEndorsement e;
  e.id = BtcEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
  e.endorsedHeight = c.transaction.publishedBlock.height;
  e.payoutInfo = {};
  return e;
}

template <>
VbkEndorsement VbkEndorsement::fromContainer(const AltPayloads& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.popData.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.endorsedHeight = c.endorsed.height;
  e.containingHash = c.containingBlock.hash;
  e.payoutInfo = c.popData.atv.transaction.publicationData.payoutInfo;
  return e;
}

template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c) {
  auto left = c.transaction.bitcoinTransaction.getHash();
  auto right = c.transaction.blockOfProof.getHash();
  return sha256(left, right);
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltPayloads& c) {
  auto left = c.popData.atv.transaction.getHash();
  auto right = c.popData.atv.containingBlock.getHash();
  return sha256(left, right);
}

}  // namespace altintegration

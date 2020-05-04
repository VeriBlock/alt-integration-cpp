// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

// TODO(Bogdan): used temporarily to disable duplication validation of VTBs
template <>
bool BtcEndorsement::checkForDuplicates = false;

template <>
bool VbkEndorsement::checkForDuplicates = true;

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
  WriteStream stream;
  c.transaction.bitcoinTransaction.toVbkEncoding(stream);
  c.transaction.blockOfProof.toRaw(stream);
  return sha256(stream.data());
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltPayloads& c) {
  WriteStream stream;
  c.popData.atv.transaction.toRaw(stream);
  c.popData.atv.containingBlock.toRaw(stream);
  return sha256(stream.data());
}

}  // namespace altintegration

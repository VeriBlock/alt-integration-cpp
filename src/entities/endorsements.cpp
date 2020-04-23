// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

inline std::string hash(const std::string& hex) { return hex.substr(0, 8); }

template <>
std::string BtcEndorsement::toPrettyString() const {
  std::ostringstream ss;
  ss << "BtcEndorsement{";
  ss << "id=" << hash(id.toHex()) << ", ";
  ss << "blockOfProof=" << hash(blockOfProof.toHex()) << ", ";
  ss << "containingHash=" << hash(containingHash.toHex()) << ", ";
  ss << "endorsedHash=" << hash(endorsedHash.toHex()) << ", ";
  ss << "endorsedHeight=" << endorsedHeight << ", ";
  ss << "payoutInfo=" << hash(HexStr(payoutInfo)) << "}";
  return ss.str();
}

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
std::string VbkEndorsement::toPrettyString() const {
  std::ostringstream ss;
  ss << "BtcEndorsement{";
  ss << "id=" << hash(id.toHex()) << ", ";
  ss << "blockOfProof=" << hash(blockOfProof.toHex()) << ", ";
  ss << "containingHash=" << hash(HexStr(containingHash)) << ", ";
  ss << "endorsedHash=" << hash(HexStr(endorsedHash)) << ", ";
  ss << "endorsedHeight=" << endorsedHeight << ", ";
  ss << "payoutInfo=" << hash(HexStr(payoutInfo)) << "}";
  return ss.str();
}

template <>
VbkEndorsement VbkEndorsement::fromContainer(const AltPayloads& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.endorsedHeight = c.endorsed.height;
  e.containingHash = c.containingBlock.hash;
  e.payoutInfo = c.atv.transaction.publicationData.payoutInfo;
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
  c.atv.transaction.toRaw(stream);
  c.atv.containingBlock.toRaw(stream);
  return sha256(stream.data());
}

}  // namespace altintegration

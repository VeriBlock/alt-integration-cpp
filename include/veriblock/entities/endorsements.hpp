// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ENDORSEMENTS_HPP
#define ALTINTEGRATION_ENDORSEMENTS_HPP

#include <veriblock/third_party/fmt/printf.h>
#include "veriblock/entities/endorsement.hpp"

namespace altintegration {

struct VTB;
struct AltPayloads;
struct PartialVTB;

// endorsement of VBK blocks in BTC
using BtcEndorsement = Endorsement<uint192, uint256, VTB, int32_t>;

template <>
BtcEndorsement BtcEndorsement ::fromContainer(const VTB& c);
template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c);

// endorsement of ALT blocks in VBK
using VbkEndorsement =
    Endorsement<std::vector<uint8_t>, uint192, AltPayloads, int32_t>;
template <>
VbkEndorsement VbkEndorsement ::fromContainer(const AltPayloads& c);
template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltPayloads& c);

template <>
inline bool VbkEndorsement::checkForDuplicates() {
  return true;
}

// TODO: once mempool is integrated, remove this hacky thing
template <>
inline bool BtcEndorsement::checkForDuplicates() {
  return false;
}

struct DummyEndorsement {
  using id_t = bool;
};

template <>
inline std::string BtcEndorsement::toPrettyString(size_t level) const {
  return fmt::sprintf("%sBtcEndorsement{containing=%s, endorsed=%s, endorsedHeight=%ld, blockOfProof=%s}",
      std::string(level, ' '),
      HexStr(containingHash),
      HexStr(endorsedHash),
      endorsedHeight,
      HexStr(blockOfProof));
}

template <>
inline std::string VbkEndorsement::toPrettyString(size_t level) const {
  return fmt::sprintf(
      "%sVbkEndorsement{containing=%s, endorsed=%s, endorsedHeight=%ld, "
      "blockOfProof=%s}",
      std::string(level, ' '),
      HexStr(containingHash),
      HexStr(endorsedHash),
      endorsedHeight,
      HexStr(blockOfProof));
}

}  // namespace altintegration

namespace std {

template <>
struct hash<altintegration::BtcEndorsement> {
  size_t operator()(const altintegration::BtcEndorsement& e) const {
    using eid = altintegration::BtcEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

template <>
struct hash<altintegration::VbkEndorsement> {
  size_t operator()(const altintegration::VbkEndorsement& e) const {
    using eid = altintegration::VbkEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

}  // namespace std

#endif  // ALTINTEGRATION_ENDORSEMENTS_HPP

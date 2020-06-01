// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ENDORSEMENTS_HPP
#define ALTINTEGRATION_ENDORSEMENTS_HPP

#include "veriblock/entities/endorsement.hpp"
#include "veriblock/fmt.hpp"

namespace altintegration {

struct VTB;
struct AltPayloads;
struct PartialVTB;

// endorsement of VBK blocks in BTC
using VbkEndorsement = Endorsement<uint192, uint256, VTB, int32_t>;

template <>
VbkEndorsement VbkEndorsement ::fromContainer(const VTB& c);
template <>
VbkEndorsement::id_t VbkEndorsement::getId(const VTB& c);

// endorsement of ALT blocks in VBK
using AltEndorsement =
    Endorsement<std::vector<uint8_t>, uint192, AltPayloads, int32_t>;
template <>
AltEndorsement AltEndorsement ::fromContainer(const AltPayloads& c);
template <>
AltEndorsement::id_t AltEndorsement::getId(const AltPayloads& c);

template <>
inline bool AltEndorsement::checkForDuplicates() {
  return true;
}

// TODO: once mempool is integrated, remove this hacky thing
template <>
inline bool VbkEndorsement::checkForDuplicates() {
  return false;
}

struct DummyEndorsement {
  using id_t = bool;
};

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

template <>
inline std::string AltEndorsement::toPrettyString(size_t level) const {
  return fmt::sprintf(
      "%sAltEndorsement{containing=%s, endorsed=%s, endorsedHeight=%ld, "
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
struct hash<altintegration::VbkEndorsement> {
  size_t operator()(const altintegration::VbkEndorsement& e) const {
    using eid = altintegration::VbkEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

template <>
struct hash<altintegration::AltEndorsement> {
  size_t operator()(const altintegration::AltEndorsement& e) const {
    using eid = altintegration::AltEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

}  // namespace std

#endif  // ALTINTEGRATION_ENDORSEMENTS_HPP

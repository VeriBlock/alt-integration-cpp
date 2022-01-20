// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ENDORSEMENTS_HPP
#define ALTINTEGRATION_ENDORSEMENTS_HPP

#include <veriblock/pop/entities/endorsement.hpp>
#include <veriblock/pop/fmt.hpp>

//! @cond Doxygen_Suppress

namespace altintegration {

struct VTB;
struct ATV;

// endorsement of VBK blocks in BTC
using VbkEndorsement = Endorsement<uint192, uint256, VTB>;

template <>
VbkEndorsement VbkEndorsement ::fromContainer(const VTB& c);
template <>
VbkEndorsement::id_t VbkEndorsement::getId(const VTB& c);

// endorsement of ALT blocks in VBK
using AltEndorsement = Endorsement<std::vector<uint8_t>, uint192, ATV>;

template <>
AltEndorsement AltEndorsement ::fromContainer(
    const ATV& c,
    const std::vector<uint8_t>& containingHash,
    const std::vector<uint8_t>& endorsedHash);
template <>
AltEndorsement::id_t AltEndorsement::getId(const ATV& c);

template <>
inline const std::string AltEndorsement::name() {
  return "ALT";
}

template <>
inline const std::string VbkEndorsement::name() {
  return "VBK";
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkEndorsement& out,
                                ValidationState& state);

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AltEndorsement& out,
                                ValidationState& state);

}  // namespace altintegration

//! @private
template <>
struct std::hash<altintegration::VbkEndorsement> {
  size_t operator()(const altintegration::VbkEndorsement& e) const {
    using eid = altintegration::VbkEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

//! @private
template <>
struct std::hash<altintegration::AltEndorsement> {
  size_t operator()(const altintegration::AltEndorsement& e) const {
    using eid = altintegration::AltEndorsement::id_t;
    return std::hash<eid>{}(e.id);
  }
};

//! @endcond

#endif  // ALTINTEGRATION_ENDORSEMENTS_HPP

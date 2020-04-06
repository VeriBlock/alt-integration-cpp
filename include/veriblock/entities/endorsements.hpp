#ifndef ALTINTEGRATION_ENDORSEMENTS_HPP
#define ALTINTEGRATION_ENDORSEMENTS_HPP

#include "veriblock/entities/endorsement.hpp"

namespace altintegration {

struct VTB;
struct AltPayloads;

// endorsement of VBK blocks in BTC
using BtcEndorsement = Endorsement<uint192, uint256, VTB, int32_t>;

template <>
BtcEndorsement BtcEndorsement::fromVbkEncoding(ReadStream& stream);
template <>
BtcEndorsement BtcEndorsement::fromVbkEncoding(const std::string& bytes);
template <>
void BtcEndorsement::toVbkEncoding(WriteStream& stream) const;
template <>
std::vector<uint8_t> BtcEndorsement::toVbkEncoding() const;
template <>
BtcEndorsement BtcEndorsement ::fromContainer(const VTB& c);
template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c);

// endorsement of ALT blocks in VBK
using VbkEndorsement =
    Endorsement<std::vector<uint8_t>, uint192, AltPayloads, int32_t>;
template <>
VbkEndorsement VbkEndorsement::fromVbkEncoding(ReadStream& stream);
template <>
VbkEndorsement VbkEndorsement::fromVbkEncoding(const std::string& bytes);
template <>
void VbkEndorsement::toVbkEncoding(WriteStream& stream) const;
template <>
std::vector<uint8_t> VbkEndorsement::toVbkEncoding() const;
template <>
VbkEndorsement VbkEndorsement ::fromContainer(const AltPayloads& c);
template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltPayloads& c);

struct DummyEndorsement {
  using id_t = bool;
};

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

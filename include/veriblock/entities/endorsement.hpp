// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

#include <memory>

#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

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
  std::vector<uint8_t> payoutInfo;

  static Endorsement fromVbkEncoding(ReadStream& stream) {
    Endorsement out;
    out.id = readSingleByteLenValue(stream);
    out.endorsedHash = readSingleByteLenValue(stream).asVector();
    out.containingHash = readSingleByteLenValue(stream).asVector();
    out.blockOfProof = readSingleByteLenValue(stream).asVector();
    out.payoutInfo = readSingleByteLenValue(stream).asVector();
    return out;
  }

  static Endorsement fromVbkEncoding(const std::string& bytes) {
    ReadStream stream(bytes);
    return fromVbkEncoding(stream);
  }

  void toVbkEncoding(WriteStream& stream) const {
    writeSingleByteLenValue(stream, id);
    writeSingleByteLenValue(stream, endorsedHash);
    writeSingleByteLenValue(stream, containingHash);
    writeSingleByteLenValue(stream, blockOfProof);
    writeSingleByteLenValue(stream, payoutInfo);
  }

  std::vector<uint8_t> toVbkEncoding() const {
    WriteStream stream;
    toVbkEncoding(stream);
    return stream.data();
  }

  static type fromContainer(const Container& c);

  static type fromContainer(const Container& c,
                            const EndorsedHash& containingHash,
                            const EndorsedHash& endorsedHash);

  static std::shared_ptr<type> fromContainerPtr(
      const Container& c,
      const EndorsedHash& containingHash,
      const EndorsedHash& endorsedHash) {
    return std::make_shared<type>(
        fromContainer(c, containingHash, endorsedHash));
  }

  static std::shared_ptr<type> fromContainerPtr(const Container& c) {
    return std::make_shared<type>(fromContainer(c));
  }

  static type::id_t getId(const Container& c);

  type::id_t getId() const { return id; }

  bool operator==(const type& other) const { return id == other.id; }

  friend bool operator<(const type& a, const type& b) {
    if (a.id < b.id) return true;
    if (a.id > b.id) return false;
    if (a.endorsedHash < b.endorsedHash) return true;
    if (a.endorsedHash > b.endorsedHash) return false;
    if (a.containingHash < b.containingHash) return true;
    if (a.containingHash > b.containingHash) return false;
    if (a.blockOfProof < b.blockOfProof) return true;
    if (a.blockOfProof > b.blockOfProof) return false;
    return a.payoutInfo < b.payoutInfo;
  }

  bool operator!=(const type& other) const { return !operator==(other); }

  std::string toPrettyString(size_t level = 0) const;
};

template <typename Value, class A, class B, class C>
Value ToJSON(const Endorsement<A, B, C>& e) {
  auto obj = json::makeEmptyObject<Value>();
  json::putStringKV(obj, "id", HexStr(e.id));
  json::putStringKV(obj, "endorsedHash", HexStr(e.endorsedHash));
  json::putStringKV(obj, "containingHash", e.containingHash);
  json::putStringKV(obj, "blockOfProof", e.blockOfProof);
  json::putStringKV(obj, "payoutInfo", e.payoutInfo);
  return obj;
}

template <typename A, typename B, typename C>
void PrintTo(const Endorsement<A, B, C>& e, std::ostream* os) {
  *os << e.toPrettyString();
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

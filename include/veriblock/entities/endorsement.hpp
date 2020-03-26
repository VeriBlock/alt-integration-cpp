#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

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

  /**
   * Read VBK data from the stream and convert it to Endorsement
   * @param stream data stream to read from
   * @return AltProof
   */
  static Endorsement fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * Endorsement
   * @param string data bytes to read from
   * @return AltProof
   */
  static Endorsement fromVbkEncoding(const std::string& bytes);

  /**
   * Convert Endorsement to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert Endorsement to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  static type fromContainer(const Container& c);

  static type::id_t getId(const Container& c);

  bool operator==(const type& other) const {
    return id == other.id && endorsedHash == other.endorsedHash &&
           containingHash == other.containingHash &&
           blockOfProof == other.blockOfProof;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ENDORSEMENT_HPP_

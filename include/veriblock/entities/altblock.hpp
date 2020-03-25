#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/entities/endorsements.hpp"

namespace altintegration {

struct AltPayloads;

struct AltBlock {
  using height_t = int32_t;
  using hash_t = std::vector<uint8_t>;
  using payloads_t = AltPayloads;
  using endorsement_t = VbkEndorsement;

  hash_t hash{};
  hash_t previousBlock{};
  uint32_t timestamp{};
  height_t height{};

  /**
   * Read VBK data from the stream and convert it to AltBlock
   * @param stream data stream to read from
   * @return AltBlock
   */
  static AltBlock fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * AltBlock
   * @param string data bytes to read from
   * @return AltBlock
   */
  static AltBlock fromVbkEncoding(const std::string& bytes);

  /**
   * Convert AltBlock to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltBlock to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  hash_t getHash() const { return hash; }

  friend bool operator==(const AltBlock& a, const AltBlock& b) {
    // clang-format off
    return a.timestamp == b.timestamp &&
           a.hash == b.hash &&
           a.previousBlock == b.previousBlock &&
           a.height == b.height;
    // clang-format on
  }
};

}  // namespace altintegration

namespace std {

template <>
struct hash<std::vector<uint8_t>> {
  size_t operator()(const std::vector<uint8_t>& x) const {
    return std::hash<std::string>{}(std::string{x.begin(), x.end()});
  }
};

template <>
struct hash<altintegration::AltBlock> {
  size_t operator()(const altintegration::AltBlock& block) {
    std::hash<std::vector<uint8_t>> hasher;
    return hasher(block.getHash());
  }
};

}  // namespace std

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALTBLOCK_HPP_

#ifndef ALT_INTEGRATION_PAYLOADS_HPP
#define ALT_INTEGRATION_PAYLOADS_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>

namespace altintegration {

struct AltProof {
  AltBlock endorsed;
  AltBlock containing;
  ATV atv;

  /**
   * Read VBK data from the stream and convert it to AltProof
   * @param stream data stream to read from
   * @return AltProof
   */
  static AltProof fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * AltProof
   * @param string data bytes to read from
   * @return AltProof
   */
  static AltProof fromVbkEncoding(const std::string& bytes);

  /**
   * Convert AltProof to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltProof to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  friend bool operator==(const AltProof& a, const AltProof& b) {
    // clang-format off
    return a.atv == b.atv &&
           a.containing == b.containing &&
           a.endorsed == b.endorsed;
    // clang-format on
  }
};

struct Payloads {
  AltProof alt{};
  std::vector<VTB> vtbs{};
  std::vector<BtcBlock> btccontext{};
  std::vector<VbkBlock> vbkcontext{};

  /**
   * Read VBK data from the stream and convert it to Payloads
   * @param stream data stream to read from
   * @return Payloads
   */
  static Payloads fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * Payloads
   * @param string data bytes to read from
   * @return AltProof
   */
  static Payloads fromVbkEncoding(const std::string& bytes);

  /**
   * Convert Payloads to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert Payloads to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  friend bool operator==(const Payloads& a, const Payloads& b) {
    // clang-format off
    return a.alt == b.alt &&
           a.vtbs == b.vtbs &&
           a.btccontext == b.btccontext &&
           a.vbkcontext == b.vbkcontext;
    // clang-format on
  }
};

}  // namespace altintegration

namespace std {

template <>
struct hash<altintegration::AltProof> {
  size_t operator()(const altintegration::AltProof& el) const {
    std::hash<std::vector<uint8_t>> hasher;
    return hasher(el.toVbkEncoding());
  }
};

template <>
struct hash<altintegration::Payloads> {
  size_t operator()(const altintegration::Payloads& el) const {
    std::hash<std::vector<uint8_t>> hasher;
    return hasher(el.toVbkEncoding());
  }
};
}  // namespace std

#endif  // ALT_INTEGRATION_PAYLOADS_HPP

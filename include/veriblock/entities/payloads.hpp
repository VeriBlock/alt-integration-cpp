#ifndef ALT_INTEGRATION_PAYLOADS_HPP
#define ALT_INTEGRATION_PAYLOADS_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>

namespace VeriBlock {

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
};

struct Payloads {
  AltProof alt{};
  std::vector<VTB> vtbs{};
  std::vector<BtcBlock> btccontext{};
  std::vector<VbkBlock> vbkcontext{};
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_PAYLOADS_HPP

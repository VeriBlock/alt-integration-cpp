#ifndef ALT_INTEGRATION_PAYLOADS_HPP
#define ALT_INTEGRATION_PAYLOADS_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct AltPayloads {
  using id_t = uint256;

  AltBlock endorsed;
  AltBlock containingBlock;
  uint256 containingTx;
  bool hasAtv{false};
  ATV atv{};
  std::vector<VTB> vtbs{};

  /**
   * Read VBK data from the stream and convert it to Payloads
   * @param stream data stream to read from
   * @return Payloads
   */
  static AltPayloads fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * Payloads
   * @param string data bytes to read from
   * @return AltProof
   */
  static AltPayloads fromVbkEncoding(const std::string& bytes);

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

  /**
   * Calculate a Payloads id that is the sha256 hash of the payloads rawBytes
   * @return id sha256 hash
   */

  id_t getId() const { return sha256(containingTx, containingBlock.hash); }

  /**
   * Return a containing AltBlock
   * @return containing block from AltProof
   */
  AltBlock getContainingBlock() const { return containingBlock; }

  /**
   * Return a endorsed AltBlock from the AltProof
   * @return endorsed block
   */
  AltBlock getEndorsedBlock() const { return endorsed; }

  /**
   * Return true if contains endorsement data
   * @return true if contains endorsement data
   */
  bool containsEndorsements() const { return hasAtv; }

  friend bool operator==(const AltPayloads& a, const AltPayloads& b) {
    return a.getId() == b.getId();
  }
};

struct DummyPayloads {
  using id_t = bool;

  id_t getId() const { return true; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_PAYLOADS_HPP

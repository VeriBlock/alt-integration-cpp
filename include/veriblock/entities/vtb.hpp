#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#include <vector>

#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

struct VTB {
  using id_t = uint256;

  VbkPopTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};
  // for compatibility with AltPayloads
  bool hasAtv{false};

  /**
   * Read VBK data from the stream and convert it to VTB
   * @param stream data stream to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to VTB
   * @param string data bytes to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(Slice<const uint8_t> bytes);

  /**
   * Read VBK data from the string raw byte representation and convert it to VTB
   * @param string data bytes to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(const std::string& bytes);

  /**
   * Convert VTB to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert VTB to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Calculate a VTB id that is the sha256 hash of the VTB rawBytes
   * @return id sha256 hash
   */
  id_t getId() const;

  /**
   * Return a containing VbkBlock
   * @return containing block
   */
  VbkBlock getContainingBlock() const;

  /**
   * Return a endorsed VbkBlock from the VbkPopTx
   * @return endorsed block
   */
  VbkBlock getEndorsedBlock() const;

  /**
   * Return true if contains endorsement data
   * @return true if contains endorsement data
   */
  bool containsEndorsements() const { return true; }

  friend bool operator==(const VTB& a, const VTB& b) {
    // clang-format off
    WriteStream a_stream, b_stream;

    a.transaction.toVbkEncoding(a_stream);
    a.merklePath.toVbkEncoding(a_stream);
    a.containingBlock.toVbkEncoding(a_stream);

    b.transaction.toVbkEncoding(b_stream);
    b.merklePath.toVbkEncoding(b_stream);
    b.containingBlock.toVbkEncoding(b_stream);
    // we dont compare 'context' field
    return a_stream.data() == b_stream.data();
    // clang-format on
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_

#include <stdint.h>

#include <vector>

#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

namespace altintegration {

struct AltPopTx {
  int32_t version{};

  std::vector<VbkBlock> vbk_context;
  bool hasAtv{false};
  ATV atv{};
  std::vector<VTB> vtbs{};

  /**
   * Read VBK data from the stream and convert it to AltPopTx
   * @param stream data stream to read from
   * @return AltPopTx
   */
  static AltPopTx fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the raw byte representation and convert it to AltPopTx
   * @param string data bytes to read from
   * @return AltPopTx
   */
  static AltPopTx fromVbkEncoding(Slice<const uint8_t> bytes);

  /**
   * Convert AltPopTx to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert AltPopTx to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Return true if contains endorsement data
   * @return true if contains endorsement data
   */
  bool containsEndorsements() const;

  friend bool operator==(const AltPopTx& a, const AltPopTx& b) {
    // clang-format off
    return a.toVbkEncoding() == b.toVbkEncoding();
    // clang-format on
  }
};

}  // namespace altintegration

#endif

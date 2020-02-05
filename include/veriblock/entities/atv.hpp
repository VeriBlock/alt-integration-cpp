#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct ATV {
  VbkTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};

  /**
   * Read VBK data from the stream and convert it to ATV
   * @param stream data stream to read from
   * @return ATV
   */
  static ATV fromVbkEncoding(ReadStream& stream);

  /**
   * Convert ATV to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ATV_HPP_

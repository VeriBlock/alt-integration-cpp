#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#include <vector>

#include "veriblock/entities/vbk_merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct VTB {
  VbkPopTx transaction{};
  VbkMerklePath merklePath{};
  VbkBlock containingBlock{};
  std::vector<VbkBlock> context{};

  /**
   * Read VBK data from the stream and convert it to VTB
   * @param stream data stream to read from
   * @return VTB
   */
  static VTB fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VTB to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VTB_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/address.hpp"
#include "veriblock/entities/coin.hpp"
#include "veriblock/entities/output.hpp"
#include "veriblock/entities/publication_data.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/uint.hpp"

namespace VeriBlock {

struct VbkTx {
  NetworkBytePair networkOrType{};
  Address sourceAddress{};
  Coin sourceAmount{};
  std::vector<Output> outputs{};
  int64_t signatureIndex{};
  PublicationData publicationData{};
  std::vector<uint8_t> signature{};
  std::vector<uint8_t> publicKey{};

  /**
   * Read basic data from the stream, signature, publicKey and convert it to
   * VbkTx
   * @param stream data stream to read from
   * @param _signature bytes
   * @param _publicKey bytes
   * @return VbkTx
   */
  static VbkTx fromRaw(ReadStream& stream,
                       Slice<const uint8_t> _signature,
                       Slice<const uint8_t> _publicKey);

  /**
   * Read VBK data from the stream and convert it to VbkTx
   * @param stream data stream to read from
   * @return VbkTx
   */
  static VbkTx fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VbkTx to data stream using VbkTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkTx to data stream using VbkTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the vbk transaction
   * @return hash vbk transaction hash
   */
  uint256 getHash() const;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/entities/address.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/btctx.hpp"
#include "veriblock/entities/merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

namespace AltIntegrationLib {

struct VbkPopTx {
  using hash_t = uint256;

  NetworkBytePair networkOrType{};
  Address address{};
  VbkBlock publishedBlock{};
  BtcTx bitcoinTransaction{};
  MerklePath merklePath{};
  BtcBlock blockOfProof{};
  std::vector<BtcBlock> blockOfProofContext{};
  std::vector<uint8_t> signature{};
  std::vector<uint8_t> publicKey{};

  /**
   * Read basic data from the stream, signature, publicKey and convert it to
   * VbkPopTx
   * @param stream data stream to read from
   * @param _signature bytes
   * @param _publicKey bytes
   * @return VbkPopTx
   */
  static VbkPopTx fromRaw(ReadStream& stream,
                          Slice<const uint8_t> _signature,
                          Slice<const uint8_t> _publicKey);

  /**
   * Read VBK data from the stream and convert it to VbkPopTx
   * @param stream data stream to read from
   * @return VbkPopTx
   */
  static VbkPopTx fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VbkPopTx to data stream using VbkPopTx basic byte format without
   * signature and publicKey data
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert VbkPopTx to data stream using VbkPopTx VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Calculate the hash of the vbk pop transaction
   * @return hash vbk pop transaction hash
   */
  hash_t getHash() const;
};

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

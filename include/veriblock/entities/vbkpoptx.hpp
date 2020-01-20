#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

#include "veriblock/entities/address.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/btctx.hpp"
#include "veriblock/entities/merkle_path.hpp"
#include "veriblock/entities/vbkblock.hpp"

namespace VeriBlock {

struct VbkPopTx {
  NetworkBytePair networkOrType{};
  Address address{};
  VbkBlock publishedBlock{};
  BtcTx bitcoinTransaction{};
  MerklePath merklePath{};
  BtcBlock blockOfProof{};
  std::vector<BtcBlock> blockOfProofContext{};
  std::vector<uint8_t> signature{};
  std::vector<uint8_t> publicKey{};

  static VbkPopTx fromRaw(ReadStream& stream,
                          Slice<const uint8_t> _signature,
                          Slice<const uint8_t> _publicKey) {
    VbkPopTx tx{};
    tx.networkOrType = readNetworkByte(stream, TxType::VBK_POP_TX);
    tx.address = Address::fromVbkEncoding(stream);
    tx.publishedBlock = VbkBlock::fromVbkEncoding(stream);
    tx.bitcoinTransaction = BtcTx::fromVbkEncoding(stream);

    auto hash = sha256twice(tx.bitcoinTransaction.tx);
    tx.merklePath = MerklePath::fromVbkEncoding(stream, hash);
    tx.blockOfProof = BtcBlock::fromVbkEncoding(stream);

    tx.blockOfProofContext = readArrayOf<BtcBlock>(
        stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
          return BtcBlock::fromVbkEncoding(stream);
        });
    tx.signature = std::vector<uint8_t>(_signature.begin(), _signature.end());
    tx.publicKey = std::vector<uint8_t>(_publicKey.begin(), _publicKey.end());

    return tx;
  }

  static VbkPopTx fromVbkEncoding(ReadStream& stream) {
    auto rawTx = readVarLenValue(stream, 0, MAX_RAWTX_SIZE_VBKPOPTX);
    auto signature = readSingleByteLenValue(stream, 0, MAX_SIGNATURE_SIZE);
    auto publicKey = readSingleByteLenValue(stream, 0, PUBLIC_KEY_SIZE);
    ReadStream rawTxStream(rawTx);
    return fromRaw(rawTxStream, signature, publicKey);
  }

  void toRaw(WriteStream& stream) const {
    NetworkBytePair popNetworkPair = {networkOrType.hasNetworkByte,
                                      networkOrType.networkByte,
                                      (uint8_t)TxType::VBK_POP_TX};
    writeNetworkByte(stream, popNetworkPair);
    address.toVbkEncoding(stream);
    publishedBlock.toVbkEncoding(stream);
    bitcoinTransaction.toVbkEncoding(stream);
    merklePath.toVbkEncoding(stream);
    blockOfProof.toVbkEncoding(stream);

    writeSingleBEValue(stream, blockOfProofContext.size());
    for (const auto& block : blockOfProofContext) {
      block.toVbkEncoding(stream);
    }
  }

  void toVbkEncoding(WriteStream& stream) const {
    WriteStream txStream;
    toRaw(txStream);
    writeVarLenValue(stream, txStream.data());
    writeSingleByteLenValue(stream, signature);
    writeSingleByteLenValue(stream, publicKey);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

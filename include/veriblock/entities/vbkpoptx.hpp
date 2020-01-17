#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/consts.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/sha256.h"

#include "veriblock/entities/address.hpp"
#include "veriblock/entities/btctx.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/merkle_path.hpp"

namespace VeriBlock {

struct VbkPopTx {
  Address address;
  VbkBlock publishedBlock;
  BtcTx bitcoinTransaction;
  MerklePath merklePath;
  BtcBlock blockOfProof;
  std::vector<BtcBlock> blockOfProofContext;
  std::vector<uint8_t> signature;
  std::vector<uint8_t> publicKey;
  uint8_t networkByte;

  VbkPopTx(Address _address,
           VbkBlock _publishedBlock,
           BtcTx _bitcoinTransaction,
           MerklePath _merklePath,
           BtcBlock _blockOfProof,
           std::vector<BtcBlock> _blockOfProofContext,
           Slice<const uint8_t> _signature,
           Slice<const uint8_t> _publicKey,
           uint8_t _networkByte)
      : address(std::move(_address)),
        publishedBlock(std::move(_publishedBlock)),
        bitcoinTransaction(std::move(_bitcoinTransaction)),
        merklePath(std::move(_merklePath)),
        blockOfProof(std::move(_blockOfProof)),
        blockOfProofContext(std::move(_blockOfProofContext)),
        signature(_signature.begin(), _signature.end()),
        publicKey(_publicKey.begin(), _publicKey.end()),
        networkByte(_networkByte) {}

  static VbkPopTx fromRaw(ReadStream& stream,
                          Slice<const uint8_t> _signature,
                          Slice<const uint8_t> _publicKey) {
    NetworkBytePair networkOrType = readNetworkByte(stream, TxType::VBK_POP_TX);
    Address address = Address::fromVbkEncoding(stream);
    VbkBlock publishedBlock = VbkBlock::fromVbkEncoding(stream);
    BtcTx bitcoinTransaction = BtcTx::fromVbkEncoding(stream);

    uint8_t btctxHash[SHA256_HASH_SIZE]{};
    sha256(btctxHash,
           bitcoinTransaction.tx.data(),
           (uint32_t)bitcoinTransaction.tx.size());
    sha256(btctxHash, btctxHash, SHA256_HASH_SIZE);

    Slice<uint8_t> btctxSlice(btctxHash, SHA256_HASH_SIZE);
    MerklePath merklePath =
        MerklePath::fromVbkEncoding(stream, Sha256Hash(btctxSlice));
    BtcBlock blockOfProof = BtcBlock::fromVbkEncoding(stream);

    auto btcContext = readArrayOf<BtcBlock>(
        stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
          return BtcBlock::fromVbkEncoding(stream);
        });

    return VbkPopTx(address,
                    publishedBlock,
                    bitcoinTransaction,
                    merklePath,
                    blockOfProof,
                    btcContext,
                    _signature,
                    _publicKey,
                    networkOrType.networkByte);
  }

  static VbkPopTx fromVbkEncoding(ReadStream& stream) {
    auto rawTx = readVarLenValue(stream, 0, MAX_RAWTX_SIZE_VBKPOPTX);
    auto signature = readSingleByteLenValue(stream, 0, MAX_SIGNATURE_SIZE);
    auto publicKey = readSingleByteLenValue(stream, 0, PUBLIC_KEY_SIZE);
    ReadStream rawTxStream(rawTx);
    return fromRaw(rawTxStream, signature, publicKey);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKPOPTX_HPP_

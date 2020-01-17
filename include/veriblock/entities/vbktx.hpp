#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

#include "veriblock/entities/address.hpp"
#include "veriblock/entities/coin.hpp"
#include "veriblock/entities/output.hpp"
#include "veriblock/entities/publication_data.hpp"

namespace VeriBlock {

struct VbkTx {
  uint8_t txtype;
  Address sourceAddress;
  Coin sourceAmount;
  std::vector<Output> outputs;
  int64_t signatureIndex;
  PublicationData publicationData;
  std::vector<uint8_t> signature;
  std::vector<uint8_t> publicKey;

  VbkTx(uint8_t _txtype,
        Address _sourceAddress,
        Coin _sourceAmount,
        std::vector<Output> _outputs,
        int64_t _signatureIndex,
        PublicationData _publicationData,
        Slice<const uint8_t> _signature,
        Slice<const uint8_t> _publicKey)
      : txtype(_txtype),
        sourceAddress(std::move(_sourceAddress)),
        sourceAmount(_sourceAmount),
        outputs(std::move(_outputs)),
        signatureIndex(_signatureIndex),
        publicationData(std::move(_publicationData)),
        signature(_signature.begin(), _signature.end()),
        publicKey(_publicKey.begin(), _publicKey.end()) {}

  static VbkTx fromRaw(ReadStream& stream,
                       Slice<const uint8_t> _signature,
                       Slice<const uint8_t> _publicKey) {
    NetworkBytePair networkOrType = readNetworkByte(stream, TxType::VBK_TX);
    Address sourceAddress = Address::fromVbkEncoding(stream);
    Coin sourceAmount = Coin::fromVbkEncoding(stream);

    uint8_t outputSize = stream.readBE<uint8_t>();
    std::vector<Output> outputs;
    outputs.reserve(outputSize);
    for (size_t i = 0; i < outputSize; i++) {
      outputs.emplace_back(Output::fromVbkEncoding(stream));
    }

    int64_t signatureIndex = readSingleBEValue<int64_t>(stream);
    auto pubBytes = readVarLenValue(stream, 0, MAX_SIZE_PUBLICATION_DATA);

    ReadStream pubBytesStream{pubBytes};
    PublicationData publicationData = PublicationData::fromRaw(pubBytesStream);

    return VbkTx(networkOrType.typeId,
                 sourceAddress,
                 sourceAmount,
                 outputs,
                 signatureIndex,
                 publicationData,
                 _signature,
                 _publicKey);
  }

  static VbkTx fromVbkEncoding(ReadStream& stream) {
    auto rawTx = readVarLenValue(stream, 0, MAX_RAWTX_SIZE_VBKTX);
    auto signature = readSingleByteLenValue(stream, 0, MAX_SIGNATURE_SIZE);
    auto publicKey = readSingleByteLenValue(stream, 0, PUBLIC_KEY_SIZE);
    ReadStream rawTxStream{rawTx};
    return fromRaw(rawTxStream, signature, publicKey);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

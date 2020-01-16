#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

namespace VeriBlock {

struct VbkTx {
  uint8_t txtype;
  int64_t signatureIndex{};
  PublicationData publicationData{};

  static VbkTx fromRaw(ReadStream& stream) {
    VbkTx tx;
    NetworkBytePair networkOrType = readNetworkByte(stream, TxType::VBK_TX);
    txtype = networkOrType.typeId;

    //Address sourceAddress = Address::fromVbkEncoding(stream);
    Coin sourceAmount = Coin::fromVbkEncoding(stream);

    uint8_t outputSize = stream.readBE<uint8_t>();

    std::vector<Output> outputs;
    outputs.reserve(outputSize);
    for (size_t i = 0; i < outputSize; i++) {
      //outputs.emplace_back(Output::fromVbkEncoding(stream));
    }

    signatureIndex = readSingleBEValue<int64_t>(stream);
    auto pubBytes = readVarLenValue(stream, 0, MAX_SIZE_PUBLICATION_DATA);

    ReadStream pubBytesStream(pubBytes);
    publicationData = PublicationData::fromRaw(pubBytesStream);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_VBKTX_HPP_

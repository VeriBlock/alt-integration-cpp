#include "veriblock/bfi/bitcoin/serialize.hpp"
#include "veriblock/pop/entities/btcblock.hpp"

namespace altintegration {

namespace btc {

struct BlockHeader : public BtcBlock {
  BlockHeader() {}

  BlockHeader(int32_t version,
              uint256 previousBlock,
              uint256 merkleRoot,
              uint32_t timestamp,
              uint32_t bits,
              uint32_t nonce)
      : BtcBlock(version, previousBlock, merkleRoot, timestamp, bits, nonce) {}

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->version);
    READWRITE(this->previousBlock);
    READWRITE(this->merkleRoot);
    READWRITE(this->timestamp);
    READWRITE(this->bits);
    READWRITE(this->nonce);
  }
};

}  // namespace btc

}  // namespace altintegration
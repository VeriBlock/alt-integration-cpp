#include "veriblock/abfi/bitcoin/serialize.hpp"
#include "veriblock/pop/entities/btcblock.hpp"

namespace btc {

struct BlockHeader : public altintegration::BtcBlock {
  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(this->version);
    READWRITE(this->previousBlock.asVector());
    READWRITE(this->merkleRoot.asVector());
    READWRITE(this->timestamp);
    READWRITE(this->bits);
    READWRITE(this->nonce);
  }
};

}  // namespace btc
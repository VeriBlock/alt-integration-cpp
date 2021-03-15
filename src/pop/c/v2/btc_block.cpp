#include <veriblock/pop/c/v2/details/btc_block.h>

#include <veriblock/pop/entities/btcblock.hpp>

#include "array.hpp"
#include "serde.hpp"

namespace ai = altintegration;

struct __pop_btc_block {
  altintegration::BtcBlock block;
};

extern "C" {

void pop_btc_block_free(const pop_btc_block_t* self) {
  if (self != nullptr) {
    delete self;
  }
}

void pop_btc_block_serialize_raw(const pop_btc_block_t* self,
                                 pop_array_u8_t** out) {
  VBK_ASSERT(out);
  *out = c::SerializeToRaw(self->block);
}
void pop_btc_block_serialize_vbk(const pop_btc_block_t* self,
                                 pop_array_u8_t** out) {
  VBK_ASSERT(out);
  *out = c::SerializeToVbkEncoding(self->block);
}
bool pop_btc_block_deserialize_raw(const pop_array_u8_t* in,
                                   pop_btc_block_t** out) {
  VBK_ASSERT(out);

  ai::ReadStream rs(in->data, in->size);
  ai::ValidationState state;
  ai::BtcBlock block;
  if (!ai::DeserializeFromRaw(rs, block, state)) {
    return false;
  }

  *out = new pop_btc_block_t;
  (*out)->block = block;
  return true;
}

bool pop_btc_block_deserialize_vbk(const pop_array_u8_t* in,
                                   pop_btc_block_t** out) {
  VBK_ASSERT(out);

  ai::ReadStream rs(in->data, in->size);
  ai::ValidationState state;
  ai::BtcBlock block;
  if (!ai::DeserializeFromVbkEncoding(rs, block, state)) {
    return false;
  }

  *out = new pop_btc_block_t;
  (*out)->block = block;
  return true;
}

pop_array_u8_t pop_btc_block_get_hash(const pop_btc_block_t* self) {
  return makeArray(self->block.getHash());
}

uint32_t pop_btc_block_get_version(const pop_btc_block_t* self) {
  return self->block.getVersion();
}

uint32_t pop_btc_block_get_timestamp(const pop_btc_block_t* self) {
  return self->block.getTimestamp();
}

uint32_t pop_btc_block_get_difficulty(const pop_btc_block_t* self) {
  return self->block.getDifficulty();
}

uint32_t pop_btc_block_get_nonce(const pop_btc_block_t* self) {
  return self->block.getNonce();
}

pop_array_u8_t pop_btc_block_get_previous_block(const pop_btc_block_t* self) {
  return makeArray(self->block.getPreviousBlock());
}

pop_array_u8_t pop_btc_block_get_merkle_root(const pop_btc_block_t* self) {
  return makeArray(self->block.getMerkleRoot());
}
}
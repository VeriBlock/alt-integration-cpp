// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_VBLAKE_H
#define ALT_INTEGRATION_VERIBLOCK_VBLAKE_H

#include <stddef.h>
#include <stdint.h>

#define VBLAKE_HASH_SIZE 24

namespace altintegration {
/**
 * State context.
 */
typedef struct {
  uint8_t b[64];  ///< input buffer
  uint64_t h[8];  ///< chained state
} vblake_ctx;

/**
 * Initialize the hashing context.
 * @param ctx context to initialize.
 */
void vblake_init(vblake_ctx *ctx);  // secret key

/**
 * Add "inlen" bytes from "in" into the hash.
 * @param ctx initialized context.
 * @param in input buffer.
 * @param inlen size of input buffer. At most 64 bytes.
 * @return 0 if succeeded, -1 if input buffer is more than 64 bytes.
 */
int vblake_update(vblake_ctx *ctx,  // context
                  const void *in,
                  size_t inlen);  // data to be hashed

/**
 * Generate the message digest.
 * @param ctx initialized context.
 * @param out resulting hash is written here. Should be 24 bytes.
 */
void vblake_final(vblake_ctx *ctx, void *out);

/**
 * Convenience function for all-in-one computation.
 * @param out output hash - 24 bytes
 * @param in input data
 * @param inlen length of input data, not more than 64 bytes
 * @return 0 if succeeded, -1 if inlen is more than 64 bytes
 */
int vblake(void *out,  // return buffer for digest
           const void *in,
           size_t inlen);  // data to be hashed

}  // namespace altintegration

#endif  // ALT_INTEGRATION_VBLAKE_HPP

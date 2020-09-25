#pragma once

#include <cstring>

#include "veriblock/crypto/endian.hpp"
#include "veriblock/crypto/progpow/ethash.hpp"

#define ENABLE_SSE 0

#if defined(_M_X64) && ENABLE_SSE
#include <smmintrin.h>
#endif

namespace altintegration {
namespace progpow {

// compile time settings
#define NODE_WORDS (64 / 4)
#define MIX_WORDS (VBK_ETHASH_MIX_BYTES / 4)
#define MIX_NODES (MIX_WORDS / NODE_WORDS)

typedef union node {
  uint8_t bytes[NODE_WORDS * 4];
  uint32_t words[NODE_WORDS];
  uint64_t double_words[NODE_WORDS / 2];

#if defined(_M_X64) && ENABLE_SSE
  __m128i xmm[NODE_WORDS / 4];
#endif

} node;

static inline void ethash_h256_reset(ethash_h256_t* hash) {
  std::memset(hash, 0, 32);
}

/**
 * Allocate and initialize a new ethash_light handler. Internal version
 *
 * @param cache_size    The size of the cache in bytes
 * @param seed          Block seedhash to be used during the computation of the
 *                      cache nodes
 * @return              Newly allocated ethash_light handler or NULL in case of
 *                      ERRNOMEM or invalid parameters used for @ref
 * ethash_compute_cache_nodes()
 */
ethash_light_t ethash_light_new_internal(uint64_t cache_size,
                                         ethash_h256_t const* seed);

void ethash_calculate_dag_item(node* const ret,
                               uint32_t node_index,
                               ethash_light_t const cache);

uint64_t ethash_get_datasize(uint64_t const block_number);
uint64_t ethash_get_cachesize(uint64_t const block_number);

}  // namespace progpow
}  // namespace altintegration
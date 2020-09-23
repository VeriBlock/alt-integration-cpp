/*
  This file is part of ethash.

  ethash is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ethash is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cpp-ethereum.	If not, see <http://www.gnu.org/licenses/>.
*/
#include "internal.hpp"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>

#include <cstring>

#include "data_sizes.hpp"
#include "fnv.hpp"
#include "sha3.hpp"
#include "veriblock/crypto/compiler.hpp"
#include "veriblock/crypto/endian.hpp"
#include "veriblock/crypto/progpow/ethash.hpp"

namespace altintegration {
namespace progpow {

uint64_t ethash_get_epoch(uint64_t block) {
  return (block / VBK_ETHASH_EPOCH_LENGTH) + VBK_ETHASH_EPOCH_OFFSET;
}

uint64_t ethash_get_datasize(uint64_t const block_number) {
  assert(block_number / VBK_ETHASH_EPOCH_LENGTH < VBK_MAX_EPOCHS_SIZE);
  return dag_sizes[block_number / VBK_ETHASH_EPOCH_LENGTH];
}

uint64_t ethash_get_cachesize(uint64_t const block_number) {
  assert(block_number / VBK_ETHASH_EPOCH_LENGTH < VBK_MAX_EPOCHS_SIZE);
  return cache_sizes[block_number / VBK_ETHASH_EPOCH_LENGTH];
}

// Follows Sergio's "STRICT MEMORY HARD HASHING FUNCTIONS" (2014)
// https://bitslog.files.wordpress.com/2013/12/memohash-v0-3.pdf
// SeqMemoHash(s, R, N)
static bool ethash_compute_cache_nodes(node* const nodes,
                                       uint64_t cache_size,
                                       ethash_h256_t const* seed) {
  if (cache_size % sizeof(node) != 0) {
    return false;
  }
  uint32_t const num_nodes = (uint32_t)(cache_size / sizeof(node));

  SHA3_512(nodes[0].bytes, (uint8_t*)seed, 32);

  for (uint32_t i = 1; i != num_nodes; ++i) {
    SHA3_512(nodes[i].bytes, nodes[i - 1].bytes, 64);
  }

  for (uint32_t j = 0; j != VBK_ETHASH_CACHE_ROUNDS; j++) {
    for (uint32_t i = 0; i != num_nodes; i++) {
      uint32_t const idx = nodes[i].words[0] % num_nodes;
      node data;
      data = nodes[(num_nodes - 1 + i) % num_nodes];
      for (uint32_t w = 0; w != NODE_WORDS; ++w) {
        data.words[w] ^= nodes[idx].words[w];
      }
      SHA3_512(nodes[i].bytes, data.bytes, sizeof(data));
    }
  }

  // now perform endian conversion
  fix_endian_arr32(nodes->words, num_nodes * NODE_WORDS);
  return true;
}

void ethash_calculate_dag_node(ethash_dag_node_t* out,
                               uint32_t node_index,
                               ethash_light_t const light) {
  ethash_calculate_dag_item((node*)(out->words), node_index, light);
}

// calcDatasetItem
void ethash_calculate_dag_item(node* const ret,
                               uint32_t node_index,
                               ethash_light_t const light) {
  uint32_t num_parent_nodes = (uint32_t)(light->cache_size / sizeof(node));
  node const* cache_nodes = (node const*)light->cache;
  node const* init = &cache_nodes[node_index % num_parent_nodes];
  std::memcpy(ret, init, sizeof(node));
  ret->words[0] ^= node_index;
  SHA3_512(ret->bytes, ret->bytes, sizeof(node));
#if defined(_M_X64) && ENABLE_SSE
  __m128i const fnv_prime = _mm_set1_epi32(FNV_PRIME);
  __m128i xmm0 = ret->xmm[0];
  __m128i xmm1 = ret->xmm[1];
  __m128i xmm2 = ret->xmm[2];
  __m128i xmm3 = ret->xmm[3];
#endif

  for (uint32_t i = 0; i != VBK_ETHASH_DATASET_PARENTS; ++i) {
    uint32_t parent_index =
        fnv_hash(node_index ^ i, ret->words[i % NODE_WORDS]) % num_parent_nodes;
    node const* parent = &cache_nodes[parent_index];

#if defined(_M_X64) && ENABLE_SSE
    {
      xmm0 = _mm_mullo_epi32(xmm0, fnv_prime);
      xmm1 = _mm_mullo_epi32(xmm1, fnv_prime);
      xmm2 = _mm_mullo_epi32(xmm2, fnv_prime);
      xmm3 = _mm_mullo_epi32(xmm3, fnv_prime);
      xmm0 = _mm_xor_si128(xmm0, parent->xmm[0]);
      xmm1 = _mm_xor_si128(xmm1, parent->xmm[1]);
      xmm2 = _mm_xor_si128(xmm2, parent->xmm[2]);
      xmm3 = _mm_xor_si128(xmm3, parent->xmm[3]);

      // have to write to ret as values are used to compute index
      ret->xmm[0] = xmm0;
      ret->xmm[1] = xmm1;
      ret->xmm[2] = xmm2;
      ret->xmm[3] = xmm3;
    }
#else
    {
      for (unsigned w = 0; w != NODE_WORDS; ++w) {
        ret->words[w] = fnv_hash(ret->words[w], parent->words[w]);
      }
    }
#endif
  }
  SHA3_512(ret->bytes, ret->bytes, sizeof(node));
}

// dagSeed
ethash_h256_t ethash_get_seedhash(uint64_t block_number) {
  ethash_h256_t ret;
  ethash_h256_reset(&ret);
  if (block_number + (VBK_ETHASH_EPOCH_OFFSET * VBK_ETHASH_EPOCH_LENGTH) >=
      VBK_ETHASH_EPOCH_LENGTH) {
    uint64_t const epochs = ethash_get_epoch(block_number);
    for (uint32_t i = 0; i < epochs; ++i) {
      SHA3_256(&ret, (uint8_t*)&ret, 32);
    }
  }
  return ret;
}

ethash_light_t ethash_light_new_internal(uint64_t cache_size,
                                         ethash_h256_t const* seed) {
  struct ethash_light* ret;
  ret = (ethash_light_t)calloc(sizeof(*ret), 1);
  if (!ret) {
    return NULL;
  }
  ret->cache = malloc((size_t)cache_size);
  if (!ret->cache) {
    free(ret->cache);
    return NULL;
  }
  node* nodes = (node*)ret->cache;
  if (!ethash_compute_cache_nodes(nodes, cache_size, seed)) {
    free(ret->cache);
    free(ret);
    return NULL;
  }
  ret->cache_size = cache_size;
  return ret;
}

ethash_light_t ethash_light_new(uint64_t block_number) {
  ethash_h256_t seedhash = ethash_get_seedhash(block_number);
  uint64_t cachesize = ethash_get_cachesize(block_number);
  ethash_light_t ret;
  ret = ethash_light_new_internal(cachesize, &seedhash);
  if (!ret) {
    return NULL;
  }
  ret->block_number = block_number;
  return ret;
}

void ethash_light_delete(ethash_light_t light) {
  if (light->cache) {
    free(light->cache);
  }
  free(light);
}

}  // namespace progpow
}  // namespace altintegration

/*
  This file is part of ethash.

  ethash is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ethash is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ethash.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <cstdint>
#include <string>

#include <veriblock/uint.hpp>

#define VBK_ETHASH_REVISION 23
#define VBK_ETHASH_DATASET_BYTES_INIT 1073741824U  // 2**30
#define VBK_ETHASH_DATASET_BYTES_GROWTH 8388608U   // 2**23
#define VBK_ETHASH_CACHE_BYTES_INIT 1073741824U    // 2**24
#define VBK_ETHASH_CACHE_BYTES_GROWTH 131072U      // 2**17
#define VBK_ETHASH_EPOCH_LENGTH 8000U
#define VBK_ETHASH_EPOCH_OFFSET 323
#define VBK_ETHASH_MIX_BYTES 128
#define VBK_ETHASH_HASH_BYTES 64
#define VBK_ETHASH_DATASET_PARENTS 256
#define VBK_ETHASH_CACHE_ROUNDS 3
#define VBK_ETHASH_ACCESSES 64
#define VBK_ETHASH_DAG_NODE_SIZE (VBK_ETHASH_HASH_BYTES / 4)

namespace altintegration {
namespace progpow {

uint64_t ethash_get_epoch(uint64_t block);

struct ethash_cache {
  void* cache;
  uint64_t cache_size;
  uint64_t epoch;
};

typedef struct ethash_dag_node {
  uint32_t words[VBK_ETHASH_DAG_NODE_SIZE];
} ethash_dag_node_t;

void ethash_calculate_dag_node(ethash_dag_node_t* out,
                               uint32_t node_index,
                               ethash_cache* const light);

/**
 * Allocate and initialize a new ethash_light handler
 *
 * @param block_number   The block number for which to create the handler
 * @return               Newly allocated ethash_light handler or NULL in case of
 *                       ERRNOMEM or invalid parameters used for
 * ethash_compute_cache_nodes()
 */
ethash_cache* ethash_light_new(uint64_t block_number);
/**
 * Frees a previously allocated ethash_light handler
 * @param light        The light handler to free
 */
void ethash_light_delete(ethash_cache* light);

/**
 * Calculate the seedhash for a given block number
 */
uint256 ethash_get_seedhash(uint64_t block_number);

}  // namespace progpow
}  // namespace altintegration
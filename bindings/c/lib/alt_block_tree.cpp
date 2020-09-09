// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include "alt_block_tree.h"

#include <veriblock/blockchain/alt_chain_params.hpp>

#include "alt_block_tree_util.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/base_block_tree.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/fmt.hpp"
#include "veriblock/rewards/poprewards.hpp"
#include "veriblock/storage/payloads_index.hpp"
#include "veriblock/storage/payloads_provider.hpp"
#include "veriblock/validation_state.hpp"


struct AltBlockTree {
  altintegration::AltBlockTree* abt;
};

extern void VBK_FreeAltBlockTree(AltBlockTree_t* alt) {
  if (alt) {
    delete alt;
  }
}

extern AltBlockTree_t* NewAltBlockTree(Config_t* config) {
  //	return new AltBlockTree(config);
}

extern void acceptBlock(AltBlockTree_t* altblocktree,
                        const char* block,
                        const char* payloads) {
  altblocktree->abt->acceptBlock(block, payloads);
}

extern void acceptBlock(AltBlockTree_t* altblocktree,
                        const char* index,
                        const char* payloads);

extern int comparePopScore(AltBlockTree_t* altblocktree,
                           const const char* A,
                           const char* B);
extern bool acceptBlockHeader(AltBlockTree_t* altblocktree,
                              const char* block,
                              const char* state);
extern bool addPayloads(AltBlockTree_t* altblocktree,
                        const char* block,
                        const char* popData,
                        const char* state);
extern bool addPayloads(AltBlockTree_t* altblocktree,
                        const char* index,
                        const char* payloads,
                        const char* state);
extern bool bootstrap(AltBlockTree_t* altblocktree, const char* state);
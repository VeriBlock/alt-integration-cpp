// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_H
#define VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_H

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AltBlockTree AltBlockTree_t;

extern void VBK_FreeAltBlockTree(AltBlockTree_t* config);

extern AltBlockTree_t* NewAltBlockTree(Config_t* config);

extern void acceptBlock(AltBlockTree_t* altblocktree,
                        const char* block,
                        const char* payloads);

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

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_TREE_H

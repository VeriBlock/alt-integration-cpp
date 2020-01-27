/**********************************************************************
 * Copyright (c) 2017 Andrew Poelstra                                 *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#ifndef _SECP256K1_SCRATCH_IMPL_H_
#define _SECP256K1_SCRATCH_IMPL_H_

#include "util.h"
#include "scratch.h"

static secp256k1_scratch* secp256k1_scratch_create(const secp256k1_callback* error_callback, size_t size) {
    const size_t base_alloc = ((sizeof(secp256k1_scratch) + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT;
    void *alloc = checked_malloc(error_callback, base_alloc + size);
    secp256k1_scratch* ret = (secp256k1_scratch *)alloc;
    if (ret != NULL) {
        memset(ret, 0, sizeof(*ret));
        memcpy(ret->magic, "scratch", 8);
        ret->data = (void *) ((char *) alloc + base_alloc);
        ret->max_size = size;
    }
    return ret;
}

static void secp256k1_scratch_destroy(const secp256k1_callback* error_callback, secp256k1_scratch* scratch) {
    if (scratch != NULL) {
        VERIFY_CHECK(scratch->alloc_size == 0); /* all checkpoints should be applied */
        if (memcmp(scratch->magic, "scratch", 8) != 0) {
            secp256k1_callback_call(error_callback, "invalid scratch space");
            return;
        }
        memset(scratch->magic, 0, sizeof(scratch->magic));
        free(scratch);
    }
}

#endif

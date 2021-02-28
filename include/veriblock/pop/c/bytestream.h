// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BYTESTREAM_H
#define VERIBLOCK_POP_CPP_BYTESTREAM_H

/**
 * @defgroup c-api C interface
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VbkByteStream VBK_ByteStream;

/**
 * Read from VbkByteStream
 *
 * @param[in] self VbkByteStream
 * @param[out] buffer output buffer (must be prealocated)
 * @param[in] toRead number of bytes to read
 * @return number of bytes actually read.
 */
size_t VBK_ByteStream_Read(VBK_ByteStream* self,
                           uint8_t* buffer,
                           size_t toRead);

/**
 * Closes stream and deallocates resources.
 * @param[in] self VbkByteStream
 */
void VBK_ByteStream_Free(VBK_ByteStream* self);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
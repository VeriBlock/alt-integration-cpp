/**
 * \file sha256.h
 *
 *  Copyright (C) 2006-2010, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef _SHA2_HPP
#define _SHA2_HPP

#include <cstdint>
#include <veriblock/pop/consts.hpp>

namespace altintegration {

/**
 * \brief          SHA-256 context structure
 */
typedef struct {
  uint32_t total[2];  /*!< number of bytes processed  */
  uint32_t state[8];  /*!< intermediate digest state  */
  uint8_t buffer[64]; /*!< data block being processed */

  uint8_t ipad[64]; /*!< HMAC: inner padding        */
  uint8_t opad[64]; /*!< HMAC: outer padding        */
} sha256_context;

/**
 * Initialize sha256 context
 * @param ctx
 */
void sha256_init(sha256_context *ctx);

/**
 * Add bytes to context
 * @param ctx context
 * @param input input bytes
 * @param ilen length of input bytes
 */
void sha256_update(sha256_context *ctx, const uint8_t *input, uint32_t ilen);

/**
 * Finalize hash calculation. Hash will be written to \p out.
 * @param ctx context
 * @param out preallocated output of size 32 bytes
 */
void sha256_finish(sha256_context *ctx, uint8_t out[SHA256_HASH_SIZE]);

/**
 * All-in-one sha256 hash calculation.
 * @param out preallocated buffer of 32 bytes
 * @param buf input buffer
 * @param nsize size of input buffer
 */
void sha256(uint8_t out[SHA256_HASH_SIZE], const uint8_t *buf, uint32_t nsize);

}  // namespace altintegration

#endif /* sha2.h */

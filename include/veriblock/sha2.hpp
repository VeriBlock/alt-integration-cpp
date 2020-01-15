/**
 * \file sha2.h
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
#ifndef _SHA2_H
#define _SHA2_H

#include <vector>

/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    uint32_t total[2];     /*!< number of bytes processed  */
    uint32_t state[8];     /*!< intermediate digest state  */
    uint8_t buffer[64];   /*!< data block being processed */

    uint8_t ipad[64];     /*!< HMAC: inner padding        */
    uint8_t opad[64];     /*!< HMAC: outer padding        */
    int is224;            /*!< 0 => SHA-256, else SHA-224 */
}
sha2_context;

//=====================================================================================
//  External interfaces
//=====================================================================================
namespace VeriBlock {
    void sha256_init    (sha2_context *ctx);
    void sha256_update  (sha2_context *ctx, const unsigned char *input, uint32_t ilen);
    std::vector<uint8_t> sha256_finish(sha2_context *ctx);
    void sha256_reset   (sha2_context *ctx);
    std::vector<uint8_t> sha256(const uint8_t* buf, size_t nsize);
    

    template <typename T,
        typename = typename std::enable_if<sizeof(typename T::value_type) ==
        1>::type>
        std::vector<uint8_t> sha256(const T &container) {
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(container.data());
        return sha256(ptr, container.size());
    }

}


#endif /* sha2.h */

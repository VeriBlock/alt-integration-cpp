// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __SIGNUTIL__HPP__
#define __SIGNUTIL__HPP__

#include <stdexcept>
#include <vector>
#include "slice.hpp"
#include "blob.hpp"

namespace altintegration {

static const size_t PRIVATE_KEY_SIZE = 32;
static const size_t PUBLIC_KEY_COMPRESSED_SIZE = 33;
static const size_t PUBLIC_KEY_UNCOMPRESSED_SIZE = 65;

using PrivateKey = Blob<PRIVATE_KEY_SIZE>;
using PublicKey = Blob<PUBLIC_KEY_UNCOMPRESSED_SIZE>;
using Signature = std::vector<uint8_t>;

// VBK encoded keys are plain byte arrays
using PrivateKeyVbk = std::vector<uint8_t>;
using PublicKeyVbk = std::vector<uint8_t>;

/**
 * Convert VBK encoded private key to the PrivateKey type.
 * @param key VBK encoded private key
 * @throws std::out_of_range if key is malformed
 * @return PrivateKey for inner use
 */
PrivateKey privateKeyFromVbk(PrivateKeyVbk key);

/**
 * Convert VBK encoded public key to the PublicKey type.
 * @param key VBK encoded public key
 * @throws std::out_of_range if key is malformed
 * @return PublicKey for inner use
 */
PublicKey publicKeyFromVbk(PublicKeyVbk key);

/**
 * Convert PublicKey type to VBK encoding.
 * @param key PublicKey format public key
 * @throws std::out_of_range if key is malformed
 * @return byte array with VBK encoded public key
 */
PublicKeyVbk publicKeyToVbk(PublicKey key);

/**
 * Derive public key from the private key.
 * @param privateKey use this private key to generate public key
 * @throws std::out_of_range if privateKey is malformed
 * @return PublicKey type generated public key
 */
PublicKey derivePublicKey(PrivateKey privateKey);

/**
 * Sign message for VBK usage.
 * This function calculates SHA256 of the message and applies
 * secp256k1 signature. Result is encoded in VBK format.
 * @param message message to sign
 * @param privateKey sign with this private key
 * @throws std::out_of_range if privateKey is malformed
 * @return byte array with VBK encoded signature
 */
Signature veriBlockSign(Slice<const uint8_t> message,
                        PrivateKey privateKey);

/**
 * Verify message previously signed with veriBlockSign.
 * This function calculates SHA256 of the message, decodes
 * signature from VBK format and verifies signature
 * using provided public key. Signature should be formed
 * with secp256k1 algorithm. Public key should be derived
 * from signer's private key.
 * @param message message to verify with
 * @param signature VBK encoded signature to verify
 * @param publicKey verify signature with this public key
 * @throws std::out_of_range if publicKey is malformed
 * @return 1 if signature is valid, 0 - otherwise
 */
int veriBlockVerify(Slice<const uint8_t> message,
                    Signature signature,
                    PublicKey publicKey);

}  // namespace altintegration

#endif  //__SIGNUTIL__HPP__

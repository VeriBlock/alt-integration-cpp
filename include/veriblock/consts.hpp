// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_CONSTS_HPP
#define ALT_INTEGRATION_VERIBLOCK_CONSTS_HPP

#include <cstdint>
#include <vector>

/**
 * @file consts.hpp
 *
 * All constants in alt-cpp.
 */

namespace altintegration {

// If you will change some values, do not forget update the same values in the
// bindigs implementation

/**
 * Veriblock transaction type.
 */
enum class TxType {
  //! regular transaction
  VBK_TX = 0x01,
  //! pop transaction
  VBK_POP_TX = 0x02,
};

constexpr const auto ADDRESS_POP_DATA_SIZE_PROGPOW = 15;
constexpr const auto SHA256_HASH_SIZE = 32;
constexpr const auto BTC_TX_MAX_RAW_SIZE = 4 * 1000 * 1000;
constexpr const uint32_t BTC_HEADER_SIZE = 80;
constexpr const auto HISTORY_FOR_TIMESTAMP_AVERAGE = 20;
constexpr const auto VBK_MINIMUM_TIMESTAMP_ONSET_BLOCK_HEIGHT = 110000;
constexpr const auto MAX_HEADER_SIZE_PUBLICATION_DATA = 1024;
constexpr const auto MAX_PAYOUT_SIZE_PUBLICATION_DATA = 100;
constexpr const auto MAX_CONTEXT_SIZE_PUBLICATION_DATA = 100;
constexpr const auto MAX_CONTEXT_COUNT_VBK_PUBLICATION = 65535;
constexpr const auto BTC_BLOCK_HASH_SIZE = 32;
constexpr const auto VBK_BLOCK_HASH_SIZE = 24;
constexpr const auto VBK_MERKLE_ROOT_HASH_SIZE = 16;
constexpr const auto VBLAKE_PREVIOUS_BLOCK_HASH_SIZE = 12;
constexpr const auto VBLAKE_PREVIOUS_KEYSTONE_HASH_SIZE = 9;

//! absolute maximum number of VBK blocks per ALT block
constexpr const auto MAX_POPDATA_VBK = 50000;
//! absolute maximum number of VTB blocks per ALT block
constexpr const auto MAX_POPDATA_VTB = 50000;
//! absolute maximum number of ATV blocks per ALT block
constexpr const auto MAX_POPDATA_ATV = 50000;
//! absolute maximum allowed size for altchain hash
constexpr const auto MIN_ALT_HASH_SIZE = 12;
constexpr const auto MAX_ALT_HASH_SIZE = 1024;
//! absolute maximum of 'refs' for BTC addon
constexpr const auto MAX_BTCADDON_REFS = 65536;
//! absolute maximum number of PopTxes (VTBs) per VBK block
constexpr const auto MAX_VBKPOPTX_PER_VBK_BLOCK = 1024;
constexpr const auto VTB_ID_SIZE = 32;
constexpr const auto ATV_ID_SIZE = 32;
constexpr const auto VBK_ID_SIZE = 12;
constexpr const auto MIN_PAYOUT_INFO_SIZE = 0;
// https://bitcoin.stackexchange.com/a/35881/85437
constexpr const auto MAX_PAYOUT_INFO_SIZE = 10000;



constexpr const uint32_t VBK_HEADER_SIZE_VBLAKE = 64;
constexpr const uint32_t VBK_HEADER_SIZE_PROGPOW = 65;
constexpr const auto MAX_LAYER_COUNT_MERKLE = 40;
//! NodeCore is using byte value when serializing outputs so we limit to 255
constexpr const auto MAX_OUTPUTS_COUNT = 255;
//! ASN.1/DER ECDSA encoding max value
constexpr const auto MAX_SIGNATURE_SIZE = 72;
//! X509 encoding. Max value is based on experimental data.
constexpr const auto PUBLIC_KEY_SIZE = 88;
constexpr const auto ADDRESS_SIZE = 30;
constexpr const auto MULTISIG_ADDRESS_SIZE = 30;

constexpr const auto VBK_MAXIMUM_DIFFICULTY =
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

constexpr const auto MAX_MERKLE_BYTES =
    (SHA256_HASH_SIZE + 1) * MAX_LAYER_COUNT_MERKLE + 5 + 5 + 5 + 5 + 4;

constexpr const auto MAX_SIZE_PUBLICATION_DATA =
    // identifier.size, identifier
    9 +
    // header.size.size, header.size, header
    5 + MAX_HEADER_SIZE_PUBLICATION_DATA +
    // payoutInfo.size.size, payoutInfo.size, payoutInfo
    5 + MAX_PAYOUT_SIZE_PUBLICATION_DATA +
    // contextInfo.size.size, contextInfo.size, contextInfo
    5 + MAX_CONTEXT_SIZE_PUBLICATION_DATA;

constexpr const auto MAX_RAWTX_SIZE_VBKTX =
    // network byte, type
    1 + 1 +
    // sourceAddress.size, sourceAddress
    1 + ADDRESS_SIZE +
    // sourceAmount.size, sourceAmount
    1 + 8 +
    // outputs.size, outputs.size * (output.address.size + output.address +
    // output.amount.size + output.amount)
    1 + MAX_OUTPUTS_COUNT * (1 + ADDRESS_SIZE + 1 + 8) +
    // signatureIndex.size, signatureIndex
    1 + 8 +
    // data.size.size, data.size, data
    5 + MAX_SIZE_PUBLICATION_DATA +
    // signature.size, signature
    1 + MAX_SIGNATURE_SIZE +
    // publicKey.size, publicKey
    1 + PUBLIC_KEY_SIZE +
    // raw.size.size, raw.size
    5;

constexpr const auto MAX_RAWTX_SIZE_VBKPOPTX =
    // network byte, type
    1 + 1 +
    // address.size, address
    1 + ADDRESS_SIZE +
    // publishedBlock.size, publishedBlock
    1 + VBK_HEADER_SIZE_PROGPOW +
    // bitcoinTransaction.size.size, bitcoinTransaction.size, bitcoinTransaction
    5 + BTC_TX_MAX_RAW_SIZE + MAX_MERKLE_BYTES +
    // blockOfProof.size, blockOfProof
    1 + BTC_HEADER_SIZE +
    // blockOfProofContext.size.size, blockOfProofContext.size,
    // blockOfProofContext
    5 + (BTC_HEADER_SIZE + 1) * MAX_CONTEXT_COUNT_VBK_PUBLICATION +
    // signature.size, signature
    1 + MAX_SIGNATURE_SIZE +
    // publicKey.size, publicKey
    1 + PUBLIC_KEY_SIZE +
    // raw.size.size, raw.size
    5;

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

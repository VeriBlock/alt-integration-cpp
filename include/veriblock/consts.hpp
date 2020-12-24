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
constexpr const auto MAX_BTC_BLOCKS_IN_VBKPOPTX = 65535;
constexpr const auto BTC_BLOCK_HASH_SIZE = 32;
constexpr const auto VBK_BLOCK_HASH_SIZE = 24;
constexpr const auto VBK_MERKLE_ROOT_HASH_SIZE = 16;
constexpr const auto VBK_PREVIOUS_BLOCK_HASH_SIZE = 12;
constexpr const auto VBK_PREVIOUS_KEYSTONE_HASH_SIZE = 9;

// https://bitcoin.stackexchange.com/a/35881/85437
constexpr const auto MAX_PAYOUT_INFO_SIZE = 10000;
constexpr const auto MAX_HEADER_SIZE_PUBLICATION_DATA = 1024;
constexpr const auto MAX_CONTEXT_SIZE_PUBLICATION_DATA = 10000;
//! absolute maximum size for PublicationData
constexpr const auto MAX_PUBLICATIONDATA_SIZE =
    1 + sizeof(int64_t) + 1 + MAX_HEADER_SIZE_PUBLICATION_DATA + 1 +
    MAX_PAYOUT_INFO_SIZE + 1 + MAX_CONTEXT_SIZE_PUBLICATION_DATA;
//! maximum size of single PopData in a single ALT block, in bytes.
//! VTB can have a size up to 5.3 MB, so this number must be at least 5.3MB
constexpr const auto MAX_POPDATA_SIZE = 5500000;
//! absolute maximum number of VBK blocks per ALT block
constexpr const auto MAX_POPDATA_VBK = 50000;
//! absolute maximum number of VTB blocks per ALT block
constexpr const auto MAX_POPDATA_VTB = 50000;
//! absolute maximum number of ATV blocks per ALT block
constexpr const auto MAX_POPDATA_ATV = 50000;
//! absolute maximum number of pauouts per ALT block
constexpr const auto MAX_PAYOUT = 50000;
//! absolute maximum allowed size for altchain hash
constexpr const auto MIN_ALT_HASH_SIZE = 0;
constexpr const auto MAX_ALT_HASH_SIZE = 1024;
//! absolute maximum of 'refs' for BTC addon
constexpr const auto MAX_BTCADDON_REFS = 65536;
//! absolute maximum number of PopTxes (VTBs) per VBK block
constexpr const auto MAX_VBKPOPTX_PER_VBK_BLOCK = 1024;
constexpr const auto VTB_ID_SIZE = 32;
constexpr const auto ATV_ID_SIZE = 32;
constexpr const auto VBK_ID_SIZE = 12;
// 65 = VBK header + 15 POP bytes of Address
constexpr const auto VBK_PUBLICATIONDATA_SIZE = 80;

constexpr const uint32_t VBK_HEADER_SIZE_VBLAKE = 64;
constexpr const uint32_t VBK_HEADER_SIZE_PROGPOW = 65;
constexpr const auto MAX_LAYER_COUNT_MERKLE = 40;
//! NodeCore is using byte value when serializing outputs so we limit to 255
constexpr const auto MAX_OUTPUTS_COUNT = 255;
//! ASN.1/DER ECDSA encoding max value
constexpr const auto MAX_SIGNATURE_SIZE = 72;
//! X509 encoding. Max value is based on experimental data.
constexpr const auto MAX_PUBLIC_KEY_SIZE = 88;
constexpr const auto ADDRESS_SIZE = 30;
constexpr const auto MULTISIG_ADDRESS_SIZE = 30;

//! maximum number of Ethash cache sizes stored in this library.
constexpr const auto VBK_MAX_CALCULATED_EPOCHS_SIZE = 4096U;
//! size of 1 ethash epoch in progpow
constexpr const auto VBK_ETHASH_EPOCH_LENGTH = 8000U;
constexpr const auto VBK_ETHASH_EPOCH_OFFSET = 323U;

constexpr const auto VBK_MAXIMUM_DIFFICULTY =
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

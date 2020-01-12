#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

namespace VeriBlock {

enum TxType {
  VBK_TX = 0x01,
  VBK_POP_TX = 0x02,
};

constexpr const auto MAX_HEADER_SIZE_PUBLICATION_DATA = 1024;
constexpr const auto MAX_PAYOUT_SIZE_PUBLICATION_DATA = 100;
constexpr const auto MAX_CONTEXT_SIZE_PUBLICATION_DATA = 100;
constexpr const auto SHA256_HASH_SIZE = 32;
constexpr const auto VBK_MERKLE_ROOT_SIZE = 16;
constexpr const auto VBLAKE_PREVIOUS_BLOCK_SIZE = 12;
constexpr const auto VBLAKE_PREVIOUS_KEYSTONE_SIZE = 9;
constexpr const auto BTC_HEADER_SIZE = 80;
constexpr const auto VBK_HEADER_SIZE = 64;
constexpr const auto BTC_TX_MAX_RAW_SIZE = 4 * 1000 * 1000;
constexpr const auto MAX_LAYER_COUNT_MERKLE = 40;
/**
 * size = (hash + hash.length) * MAX_LAYER_COUNT + (index + index.length) +
 * (layers.size + layers.size.length) +
 * (subject.length.size + subject.length.size.size) + (subject.length) +
 * (data_size)
 */
constexpr const auto MAX_MERKLE_BYTES =
    (SHA256_HASH_SIZE + 1) * MAX_LAYER_COUNT_MERKLE + 5 + 5 + 5 + 5 + 4;
}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

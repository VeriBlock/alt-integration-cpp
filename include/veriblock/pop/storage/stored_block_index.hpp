// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORED_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORED_BLOCK_INDEX_HPP_

#include <veriblock/pop/entities/altblock.hpp>

namespace altintegration {

struct AltChainParams;

template <typename Block>
struct StoredBlockIndex {
  using block_t = Block;
  using height_t = typename block_t::height_t;
  using stored_addon_t = typename block_t::stored_addon_t;

  ~StoredBlockIndex() = default;

  bool isDeleted() const { return status & BLOCK_DELETED; };

  void toVbkEncoding(WriteStream& stream) const {
    using height_t = typename Block::height_t;
    stream.writeBE<height_t>(height);
    header->toRaw(stream);
    stream.writeBE<uint32_t>(status);
    addon.toVbkEncoding(stream);
  }

  std::vector<uint8_t> toVbkEncoding() const {
    WriteStream stream;
    toVbkEncoding(stream);
    return stream.data();
  }

  std::string toPrettyString(size_t level = 0) const {
    return format(
        "{}{}StoredBlockIndex(height={}, hash={}, status={}, header={}, {})",
        std::string(level, ' '),
        Block::name(),
        height,
        HexStr(header->getHash()),
        status,
        header->toPrettyString(),
        addon.toPrettyString());
  }

  //! height of the entry in the chain
  height_t height = 0;

  //! block header
  std::shared_ptr<block_t> header = std::make_shared<block_t>();

  //! contains status flags
  uint32_t status = BLOCK_VALID_UNKNOWN;

  stored_addon_t addon;
};

template <typename Block>
bool DeserializeFromVbkEncoding(
    ReadStream& stream,
    StoredBlockIndex<Block>& out,
    ValidationState& state,
    const AltChainParams& params,
    typename Block::hash_t precalculatedHash = typename Block::hash_t()) {
  const auto& name = Block::name();
  using height_t = typename Block::height_t;
  if (!stream.readBE<height_t>(out.height, state)) {
    return state.Invalid(name + "-stored-block-index-height");
  }
  Block block{};
  if (!DeserializeFromRaw(stream, block, state, params, precalculatedHash)) {
    return state.Invalid(name + "-stored-block-index-header");
  }
  out.header = std::make_shared<Block>(block);
  if (!stream.readBE<uint32_t>(out.status, state)) {
    return state.Invalid(name + "-stored-block-index-status");
  }

  if (!DeserializeFromVbkEncoding(stream, out.addon, state)) {
    return state.Invalid(name + "-stored-block-index-addon");
  }
  return true;
}

}  // namespace altintegration
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_STORED_BLOCK_INDEX_HPP_

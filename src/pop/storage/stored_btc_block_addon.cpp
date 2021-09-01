// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/storage/stored_btc_block_addon.hpp>

namespace altintegration {

StoredBtcBlockAddon::StoredBtcBlockAddon(const addon_t& other) {
  blockOfProofEndorsementHashes =
      map_get_id_from_pointers<uint256, const VbkEndorsement>(
          other.getBlockOfProofEndorsement());
  refs = other.getRefs();
}

void StoredBtcBlockAddon::toVbkEncoding(WriteStream& w) const {
  writeArrayOf<uint256>(
      w, blockOfProofEndorsementHashes, writeSingleByteLenValue);
  writeArrayOf<ref_height_t>(
      w, refs, [&](WriteStream& /*ignore*/, ref_height_t value) {
        w.writeBE<ref_height_t>(value);
      });
}

void StoredBtcBlockAddon::toInmem(StoredBtcBlockAddon::addon_t& to) const {
  to.clearRefs();
  for (const auto& r : refs) {
    to.addRef(r);
  }
}

std::string StoredBtcBlockAddon::toPrettyString() const {
  return altintegration::format(
      "refs={}[{}]", refs.size(), fmt::join(refs, ","));
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredBtcBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<uint256>(
          stream,
          out.blockOfProofEndorsementHashes,
          state,
          0,
          MAX_POPDATA_VBK,
          [](ReadStream& stream, uint256& o, ValidationState& state) -> bool {
            return readSingleByteLenValue(
                stream, o, state, uint256::size(), uint256::size());
          })) {
    return state.Invalid("stored-btc-block-addon-bad-block-of-proof-hash");
  }

  if (!readArrayOf<StoredBtcBlockAddon::ref_height_t>(
          stream,
          out.refs,
          state,
          0,
          MAX_BTCADDON_REFS,
          [](ReadStream& stream, int32_t& out, ValidationState& state) -> bool {
            return stream.readBE<StoredBtcBlockAddon::ref_height_t>(out, state);
          })) {
    return state.Invalid("bad-refs");
  }

  return true;
}

}  // namespace altintegration

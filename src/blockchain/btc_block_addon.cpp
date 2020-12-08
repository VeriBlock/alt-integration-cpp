// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/entities/btcblock.hpp>

namespace altintegration {

void BtcBlockAddon::setDirty() {
  static_cast<BlockIndex<BtcBlock>*>(this)->setDirty();
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<int32_t>(stream,
                            out.refs,
                            state,
                            0,
                            MAX_BTCADDON_REFS,
                            [&](int32_t& out) -> bool {
                              return stream.readBE<int32_t>(out, state);
                            })) {
    return state.Invalid("bad-refs");
  }

  return true;
}

void BtcBlockAddon::toVbkEncoding(WriteStream& w) const {
  // save only refs
  writeArrayOf<ref_height_t>(
      w, refs, [&](WriteStream& /*ignore*/, ref_height_t value) {
        w.writeBE<ref_height_t>(value);
      });
}

void BtcBlockAddon::setNullInmemFields() {
  chainWork = 0;
  blockOfProofEndorsements.clear();
}

void BtcBlockAddon::setIsBootstrap(bool isBootstrap) {
  if (isBootstrap) {
    // pretend this block is referenced by the genesis block of the SI chain
    addRef(0);
  } else {
    VBK_ASSERT(false && "not supported");
  }
}

uint32_t BtcBlockAddon::refCount() const { return (uint32_t)refs.size(); }

void BtcBlockAddon::addRef(BtcBlockAddon::ref_height_t referencedAtHeight) {
  refs.push_back(referencedAtHeight);
  setDirty();
}

void BtcBlockAddon::removeRef(BtcBlockAddon::ref_height_t referencedAtHeight) {
  auto ref_it = find(refs.begin(), refs.end(), referencedAtHeight);
  VBK_ASSERT(ref_it != refs.end() &&
             "state corruption: tried removing a nonexistent reference to a "
             "BTC block");
  refs.erase(ref_it);
  setDirty();
}

std::string BtcBlockAddon::toPrettyString() const {
  return fmt::format("chainwork={}", chainWork.toHex());
}

void BtcBlockAddon::setNull() {
  refs.clear();
  chainWork = 0;
}

}  // namespace altintegration

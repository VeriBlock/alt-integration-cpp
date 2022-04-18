// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/blockchain/btc_block_addon.hpp>
#include <veriblock/pop/entities/btcblock.hpp>

namespace altintegration {

void BtcBlockAddon::setDirty() {
  static_cast<BlockIndex<BtcBlock>*>(this)->setDirty();
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<BtcBlockAddon::ref_height_t>(
          stream,
          out.refs,
          state,
          0,
          MAX_BTCADDON_REFS,
          [](ReadStream& stream, int32_t& out, ValidationState& state) -> bool {
            return stream.readBE<BtcBlockAddon::ref_height_t>(out, state);
          })) {
    return state.Invalid("bad-refs");
  }

  return true;
}

void BtcBlockAddon::toVbkEncoding(WriteStream& w) const {
  // save only refs
  writeContainer(w, refs, [&](WriteStream& /*ignore*/, ref_height_t value) {
    w.writeBE<ref_height_t>(value);
  });
}

void BtcBlockAddon::setNullInmemFields() {
  chainWork = 0;
  _blockOfProofEndorsements.clear();
}

void BtcBlockAddon::setIsBootstrap(bool isBootstrap) {
  if (isBootstrap) {
    // pretend this block is referenced by the genesis block of the SI chain
    addRef(0);
  } else {
    VBK_ASSERT_MSG(false, "not supported");
  }
}

uint32_t BtcBlockAddon::refCount() const { return (uint32_t)refs.size(); }

void BtcBlockAddon::addRef(BtcBlockAddon::ref_height_t referencedAtHeight) {
  refs.push_back(referencedAtHeight);
  setDirty();
}

void BtcBlockAddon::clearRefs() {
  refs.clear();
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

void BtcBlockAddon::insertBlockOfProofEndorsement(const VbkEndorsement* e) {
  this->_blockOfProofEndorsements.push_back(e);
  setDirty();
}

bool BtcBlockAddon::eraseLastFromBlockOfProofEndorsement(
    const VbkEndorsement* endorsement) {
  auto rm = [&endorsement](const VbkEndorsement* e) -> bool {
    return e == endorsement;
  };
  auto res =
      erase_last_item_if<const VbkEndorsement*>(_blockOfProofEndorsements, rm);
  if (res) {
    setDirty();
  }
  return res;
}

void BtcBlockAddon::clearBlockOfProofEndorsement() {
  if (this->_blockOfProofEndorsements.empty()) return;
  this->_blockOfProofEndorsements.clear();
  setDirty();
}

const std::vector<const VbkEndorsement*>&
BtcBlockAddon::getBlockOfProofEndorsement() const {
  return this->_blockOfProofEndorsements;
}

std::string BtcBlockAddon::toPrettyString() const {
  return format("chainwork={}", chainWork.toHex());
}

void BtcBlockAddon::setNull() {
  refs.clear();
  chainWork = 0;
}

}  // namespace altintegration

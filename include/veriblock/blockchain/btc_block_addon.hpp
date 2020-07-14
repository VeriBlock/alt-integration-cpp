// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

#include <veriblock/arith_uint256.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

struct BtcBlockAddon {
  using ref_height_t = int32_t;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  void setIsBootstrap(bool isBootstrap) {
    if (isBootstrap) {
      // pretend this block is referenced by the genesis block of the SI chain
      addRef(0);
    } else {
      VBK_ASSERT(false && "not supported");
    }
  }

  uint32_t refCount() const { return refs.size(); }

  void addRef(ref_height_t referencedAtHeight) {
    refs.push_back(referencedAtHeight);
    setDirty();
  }

  void removeRef(ref_height_t referencedAtHeight) {
    auto ref_it = find(refs.begin(), refs.end(), referencedAtHeight);
    VBK_ASSERT(ref_it != refs.end() &&
               "state corruption: tried removing a nonexistent reference to a "
               "BTC block");
    refs.erase(ref_it);
    setDirty();
  }

  void serializeRefs(WriteStream& w) const {
    w.writeBE<uint32_t>(refs.size());
    for (auto ref : refs) {
      w.writeBE<ref_height_t>(ref);
    }
  }

  void toRaw(WriteStream& w) const {
    // save only refs
    return serializeRefs(w);
  }

  // not static, on purpose
  void deserializeRefs(ReadStream& r) {
    auto refCount = r.readBE<uint32_t>();
    refs.clear();
    refs.reserve(refCount);
    for (uint32_t i = 0; i < refCount; i++) {
      refs.push_back(r.readBE<uint32_t>());
    }
  }

  void initAddonFromRaw(ReadStream& r) { return deserializeRefs(r); }

  bool operator==(const BtcBlockAddon& o) const {
    // comparing reference counts does not seem like a good idea
    // as the only situation where they would be different is
    // comparing blocks across different trees eg mock miner vs
    // the test tree and in this situation the references and counts
    // are likely to differ
    bool a = true;  // refs == o.refs;
    bool b = chainWork == o.chainWork;
    return a && b;
  }

  std::string toPrettyString() const {
    return fmt::format("chainwork={}", chainWork.toHex());
  }

 protected:
  //! reference counter for fork resolution
  std::vector<ref_height_t> refs{};

  void setDirty();

  void setNull() {
    refs.clear();
    chainWork = 0;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

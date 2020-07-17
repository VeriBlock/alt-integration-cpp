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

  void toRaw(WriteStream& w) const {
    // save only refs
    writeArrayOf<ref_height_t>(
        w, refs, [](WriteStream& stream, ref_height_t value) {
          stream.writeBE<ref_height_t>(value);
        });
  }

  void initAddonFromRaw(ReadStream& r) {
    refs = readArrayOf<ref_height_t>(
        r, [](ReadStream& stream) { return stream.readBE<ref_height_t>(); });
  }

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
  // Ideally we would want a sorted collection with cheap addition, deletion and
  // lookup. In practice, due to VBK/BTC block time ratio(20x) and multiple APMs
  // running, there's little chance of VTBs shipping non-empty BTC context. The
  // altchain mempool prioritization algo will realistically pick 1 VTB per VBK
  // keystone period(20 blocks), providing us with the BTC block mined during
  // this period. Thus, we can expect to have 1-2 references per block and 2x
  // that during fork resolution making std::vector the fastest storage option
  // in this case.
  // TODO: figure out if this is somehow abusable by spammers/dosers

  std::vector<ref_height_t> refs{};

  void setDirty();

  void setNull() {
    refs.clear();
    chainWork = 0;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

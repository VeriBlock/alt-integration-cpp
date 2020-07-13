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
  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  uint32_t getRefCounter() const { return _refCounter; }
  void incRefCounter() { _refCounter++; }
  void decRefCounter() { _refCounter--; }

  bool operator==(const BtcBlockAddon& o) const {
    bool a = _refCounter == o._refCounter;
    bool b = chainWork == o.chainWork;
    return a && b;
  }

  std::string toPrettyString() const {
    return fmt::format("chainwork={}", chainWork.toHex());
  }

  void toRaw(WriteStream& w) const { w.writeBE<uint32_t>(_refCounter); }

 protected:
  //! reference counter for fork resolution
  uint32_t _refCounter = 0;

  void setNull() {
    _refCounter = 0;
    chainWork = 0;
  }

    // not static, on purpose
  void initAddonFromRaw(ReadStream& r) { _refCounter = r.readBE<uint32_t>(); }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

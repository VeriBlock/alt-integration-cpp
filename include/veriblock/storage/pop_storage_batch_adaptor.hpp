// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_STORAGE_REPO_BATCH_ADAPTOR_HPP
#define VERIBLOCK_POP_CPP_POP_STORAGE_REPO_BATCH_ADAPTOR_HPP

#include "batch_adaptor.hpp"
#include "pop_storage.hpp"

namespace altintegration {

struct PopStorageBatchAdaptor : public BatchAdaptor {
  ~PopStorageBatchAdaptor() override = default;

  PopStorageBatchAdaptor(PopStorage& pop) : pop_(pop) {}

  bool writeBlock(const BlockIndex<BtcBlock>& value) override {
    pop_.getBlockRepo<BlockIndex<BtcBlock>>().put(value);
    return true;
  }
  bool writeBlock(const BlockIndex<VbkBlock>& value) override {
    pop_.getBlockRepo<BlockIndex<VbkBlock>>().put(value);
    return true;
  }
  bool writeBlock(const BlockIndex<AltBlock>& value) override {
    pop_.getBlockRepo<BlockIndex<AltBlock>>().put(value);
    return true;
  }

  bool writeTip(const BlockIndex<BtcBlock>& value) override {
    pop_.saveTip(value);
    return true;
  }
  bool writeTip(const BlockIndex<VbkBlock>& value) override {
    pop_.saveTip(value);
    return true;
  }
  bool writeTip(const BlockIndex<AltBlock>& value) override {
    pop_.saveTip(value);
    return true;
  }

 private:
  PopStorage& pop_;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_POP_STORAGE_REPO_BATCH_ADAPTOR_HPP

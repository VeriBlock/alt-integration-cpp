#include "veriblock/state_manager.hpp"

using namespace VeriBlock;

void StateChange::putBtcBlock(const BlockIndex<BtcBlock>& block) {
  btcRepoBatch->put(block);
}

void StateChange::putVbkBlock(const BlockIndex<VbkBlock>& block) {
  vbkRepoBatch->put(block);
}

void StateChange::commit() {
  btcRepoBatch->commit();
  vbkRepoBatch->commit();
}

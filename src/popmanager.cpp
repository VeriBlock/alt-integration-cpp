#include <algorithm>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace altintegration {

bool PopManager::addPayloads(const Payloads& payloads,
                             StateChange& stateChange,
                             ValidationState& state) {
  return tryValidateWithResources(
      [&]() {
        /// update btc context
        for (const auto& b : payloads.btccontext) {
          BlockIndex<BtcBlock> index;
          if (!this->btc().acceptBlock(b, state, &index)) {
            return state.addStackFunction("addPayloads");
          }
          // stateChange.putBtcBlock(index);
        }

        /// update vbk context
        for (const auto& b : payloads.vbkcontext) {
          BlockIndex<VbkBlock> index;
          if (!this->vbk().acceptBlock(b, state, &index)) {
            return state.addStackFunction("addPayloads");
          }
          // stateChange.putVbkBlock(index);
        }

        /// ADD ALL VTBs
        for (const auto& vtb : payloads.vtbs) {
          if (!this->addVTB(vtb, stateChange, state)) {
            return state.addStackFunction("addPayloads");
          }
        }

        /// ADD ATV
        if (!this->addAltProof(payloads.alt, stateChange, state)) {
          return state.addStackFunction("addPayloads");
        }

        uncommitted_.push_back(payloads);

        return true;
      },
      [&]() { this->removePayloads(payloads, stateChange); });
}

void PopManager::removePayloads(const Payloads& payloads,
                                StateChange& stateChange) noexcept {
  /// first, remove ATV
  removeAltProof(payloads.alt, stateChange);

  /// second, remove VTBs in reverse order
  auto& v = payloads.vtbs;
  std::for_each(v.rbegin(), v.rend(), [this, &stateChange](const VTB& vtb) {
    removeVTB(vtb, stateChange);
  });

  /// remove vbk context
  for (const auto& b : payloads.vbkcontext) {
    this->vbk().invalidateBlockByHash(b.getHash());
    // stateChange.removeVbkBlock(b.getHash());
  }

  /// remove btc context
  for (const auto& b : payloads.btccontext) {
    this->btc().invalidateBlockByHash(b.getHash());
    // stateChange.removeBtcBlock(b.getHash());
  }
}

bool PopManager::addVTB(const VTB& vtb, StateChange&, ValidationState& state) {
  if (!checkVTB(vtb, state, *vbkparam_, *btcparam_)) {
    return state.addStackFunction("addVTB");
  }

  // firstly, add btc context blocks
  for (const auto& block : vtb.transaction.blockOfProofContext) {
    BlockIndex<BtcBlock> index;
    if (!btc_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addVTB");
    }
  }

  // secondly, add VBK endorsements in BTC
  btce_->put(vtb);

  // thirdly, add vbk context blocks
  for (const auto& block : vtb.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addVTB");
    }
  }

  return true;
}

void PopManager::removeVTB(const VTB& vtb, StateChange&) noexcept {
  // remove VBK context in reverse order
  auto& vbkctx = vtb.context;
  std::for_each(vbkctx.rbegin(), vbkctx.rend(), [this](const VbkBlock& b) {
    vbk_->invalidateBlockByHash(b.getHash());
  });

  // remove endorsement
  btce_->remove(vtb);

  // remove BTC context in reverse order
  auto& btcctx = vtb.transaction.blockOfProofContext;
  std::for_each(btcctx.rbegin(), btcctx.rend(), [this](const BtcBlock& b) {
    btc_->invalidateBlockByHash(b.getHash());
  });
}

bool PopManager::addAltProof(const AltProof& payloads,
                             StateChange&,
                             ValidationState& state) {
  if (!checkATV(payloads.atv, state, *vbkparam_)) {
    return state.addStackFunction("addPayloads");
  }

  for (const auto& block : payloads.atv.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addPayloads");
    }
  }

  vbke_->put(payloads);

  return true;
}

void PopManager::removeAltProof(const AltProof& alt, StateChange&) noexcept {
  auto& vbkctx = alt.atv.context;
  std::for_each(vbkctx.rbegin(), vbkctx.rend(), [this](const VbkBlock& b) {
    vbk_->invalidateBlockByHash(b.getHash());
  });

  vbke_->remove(alt);
}

void PopManager::rollback(StateChange& stateChange) noexcept {
  // rollback in reverse order
  std::for_each(uncommitted_.rbegin(),
                uncommitted_.rend(),
                [this, &stateChange](const Payloads& p) {
                  removePayloads(p, stateChange);
                });

  uncommitted_.clear();
}

void PopManager::commit() { uncommitted_.clear(); }

bool PopManager::hasUncommittedChanges() const noexcept {
  return !uncommitted_.empty();
}

int PopManager::compareTwoBranches(const Chain<AltBlock>& chain1,
                                   const Chain<AltBlock>& chain2) {
  auto pkcChain1 =
      getProtoKeystoneContext(chain1, *vbk_, vbke_, *altChainParams_);
  auto kcChain1 = getKeystoneContext(pkcChain1, *vbk_);

  auto pkcChain2 =
      getProtoKeystoneContext(chain2, *vbk_, vbke_, *altChainParams_);
  auto kcChain2 = getKeystoneContext(pkcChain2, *vbk_);

  return altChainCompare_(kcChain1, kcChain2);
}

}  // namespace altintegration

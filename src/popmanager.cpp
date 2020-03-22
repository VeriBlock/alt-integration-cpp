#include <algorithm>
#include <veriblock/blockchain/alt_block_index.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace altintegration {

bool PopManager::addPayloads(const Payloads& payloads,
                             ValidationState& state,
                             StateChange* change) {
  return tryValidateWithResources(
      [&]() {
        /// update btc context
        for (const auto& b : payloads.btccontext) {
          BlockIndex<BtcBlock> index;
          if (!this->btc().acceptBlock(b, state, &index)) {
            return state.addStackFunction("addPayloads");
          }
          if (change) {
            change->putBtcBlock(index);
          }
        }

        /// update vbk context
        for (const auto& b : payloads.vbkcontext) {
          BlockIndex<VbkBlock> index;
          if (!this->vbk().acceptBlock(b, state, &index)) {
            return state.addStackFunction("addPayloads");
          }
          if (change) {
            change->putVbkBlock(index);
          }
        }

        /// ADD ALL VTBs
        for (const auto& vtb : payloads.vtbs) {
          if (!this->addVTB(vtb, state, change)) {
            return state.addStackFunction("addPayloads");
          }
        }

        /// ADD ATV
        if (!this->addAltProof(payloads.alt, state, change)) {
          return state.addStackFunction("addPayloads");
        }

        uncommitted_.push_back(payloads);

        return true;
      },
      [&]() { this->removePayloads(payloads, change); });
}

void PopManager::removePayloads(const Payloads& payloads,
                                StateChange* change) noexcept {
  /// first, remove ATV
  removeAltProof(payloads.alt, change);

  /// second, remove VTBs in reverse order
  auto& v = payloads.vtbs;
  std::for_each(v.rbegin(), v.rend(), [this, &change](const VTB& vtb) {
    removeVTB(vtb, change);
  });

  /// remove vbk context
  for (const auto& b : payloads.vbkcontext) {
    this->vbk().invalidateBlockByHash(b.getHash());
    if (change) {
      change->removeVbkBlock(b.getHash());
    }
  }

  /// remove btc context
  for (const auto& b : payloads.btccontext) {
    this->btc().invalidateBlockByHash(b.getHash());
    if (change) {
      change->removeBtcBlock(b.getHash());
    }
  }
}

bool PopManager::addVTB(const VTB& vtb,
                        ValidationState& state,
                        StateChange* change) {
  if (!checkVTB(vtb, state, *vbkparam_, *btcparam_)) {
    return state.addStackFunction("addVTB");
  }

  // firstly, add btc context blocks
  for (const auto& block : vtb.transaction.blockOfProofContext) {
    BlockIndex<BtcBlock> index;
    if (!btc_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addVTB");
    }
    if (change) {
      change->putBtcBlock(index);
    }
  }

  // secondly, add VBK endorsements in BTC
  btce_->put(vtb);
  if (change) {
    change->putBtcEndorsement(vtb);
  }

  // thirdly, add vbk context blocks
  for (const auto& block : vtb.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addVTB");
    }
    if (change) {
      change->putVbkBlock(index);
    }
  }

  return true;
}

void PopManager::removeVTB(const VTB& vtb, StateChange* change) noexcept {
  // remove VBK context in reverse order
  auto& vbkctx = vtb.context;
  std::for_each(
      vbkctx.rbegin(), vbkctx.rend(), [this, &change](const VbkBlock& b) {
        vbk_->invalidateBlockByHash(b.getHash());
        if (change) {
          change->removeVbkBlock(b.getHash());
        }
      });

  // remove endorsement
  btce_->remove(vtb);
  // DO NOT REMOVE ENDORSEMENT FROM DISK

  // remove BTC context in reverse order
  auto& btcctx = vtb.transaction.blockOfProofContext;
  std::for_each(
      btcctx.rbegin(), btcctx.rend(), [this, &change](const BtcBlock& b) {
        btc_->invalidateBlockByHash(b.getHash());
        if (change) {
          change->removeBtcBlock(b.getHash());
        }
      });
}

bool PopManager::addAltProof(const AltProof& payloads,
                             ValidationState& state,
                             StateChange* change) {
  if (payloads.hasAtv && !checkATV(payloads.atv, state, *vbkparam_)) {
    return state.addStackFunction("addPayloads");
  }

  for (const auto& block : payloads.atv.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addPayloads");
    }
    if (change) {
      change->putVbkBlock(index);
    }
  }

  vbke_->put(payloads);
  if (change) {
    change->putVbkEndorsement(payloads);
  }

  return true;
}

void PopManager::removeAltProof(const AltProof& alt,
                                StateChange* change) noexcept {
  auto& vbkctx = alt.atv.context;
  std::for_each(
      vbkctx.rbegin(), vbkctx.rend(), [this, &change](const VbkBlock& b) {
        vbk_->invalidateBlockByHash(b.getHash());
        if (change) {
          change->removeVbkBlock(b.getHash());
        }
      });

  vbke_->remove(alt);
  // DO NOT REMOVE ALT PROOF FROM DISK
}

void PopManager::rollback() noexcept {
  // rollback in reverse order
  std::for_each(uncommitted_.rbegin(),
                uncommitted_.rend(),
                [this](const Payloads& p) { removePayloads(p, nullptr); });

  uncommitted_.clear();
}

void PopManager::commit() { uncommitted_.clear(); }

bool PopManager::hasUncommittedChanges() const noexcept {
  return !uncommitted_.empty();
}

int PopManager::compareTwoBranches(const Chain<AltBlockIndex>& chain1,
                                   const Chain<AltBlockIndex>& chain2) {
  auto pkcChain1 =
      getProtoKeystoneContext(chain1, *vbk_, vbke_, *altChainParams_);
  auto kcChain1 = getKeystoneContext(pkcChain1, *vbk_);

  auto pkcChain2 =
      getProtoKeystoneContext(chain2, *vbk_, vbke_, *altChainParams_);
  auto kcChain2 = getKeystoneContext(pkcChain2, *vbk_);

  return altChainCompare_(kcChain1, kcChain2);
}

}  // namespace altintegration

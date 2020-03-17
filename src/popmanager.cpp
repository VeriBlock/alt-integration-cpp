#include <algorithm>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace VeriBlock {

bool PopManager::addPayloads(const Payloads& payloads,
                             std::shared_ptr<StateChange> stateChange,
                             ValidationState& state) {
  return tryValidateWithResources(
      [&]() {
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

void PopManager::removePayloads(
    const Payloads& payloads,
    std::shared_ptr<StateChange> stateChange) noexcept {
  /// first, remove ATV
  removeAltProof(payloads.alt, stateChange);

  /// second, remove VTBs in reverse order
  auto& v = payloads.vtbs;
  std::for_each(v.rbegin(), v.rend(), [this, &stateChange](const VTB& vtb) {
    removeVTB(vtb, stateChange);
  });
}

bool PopManager::addVTB(const VTB& vtb,
                        std::shared_ptr<StateChange> stateChange,
                        ValidationState& state) {
  if (!checkVTB(vtb, state, *vbkparam_, *btcparam_)) {
    return state.addStackFunction("addVTB");
  }

  // firstly, add btc context blocks
  for (const auto& block : vtb.transaction.blockOfProofContext) {
    BlockIndex<BtcBlock> index;
    if (!btc_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addVTB");
    }
    stateChange->putBtcBlock(index);
  }

  // secondly, add VBK endorsements in BTC
  btce_->put(vtb);
  stateChange->putBtcEndorsement(vtb);

  // thirdly, add vbk context blocks
  for (const auto& block : vtb.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state)) {
      return state.addStackFunction("addVTB");
    }
    stateChange->putVbkBlock(index);
  }

  return true;
}

void PopManager::removeVTB(const VTB& vtb,
                           std::shared_ptr<StateChange> stateChange) noexcept {
  // remove VBK context in reverse order
  auto& vbkctx = vtb.context;
  std::for_each(
      vbkctx.rbegin(), vbkctx.rend(), [this, &stateChange](const VbkBlock& b) {
        vbk_->invalidateBlockByHash(b.getHash());
        stateChange->removeVbkBlock(b.getHash());
      });

  // remove endorsement
  btce_->remove(vtb);
  stateChange->removeBtcEndorsement(vtb);

  // remove BTC context in reverse order
  auto& btcctx = vtb.transaction.blockOfProofContext;
  std::for_each(
      btcctx.rbegin(), btcctx.rend(), [this, &stateChange](const BtcBlock& b) {
        btc_->invalidateBlockByHash(b.getHash());
        stateChange->removeBtcBlock(b.getHash());
      });
}

bool PopManager::addAltProof(const AltProof& payloads,
                             std::shared_ptr<StateChange> stateChange,
                             ValidationState& state) {
  if (!checkATV(payloads.atv, state, *vbkparam_)) {
    return state.addStackFunction("addPayloads");
  }

  for (const auto& block : payloads.atv.context) {
    BlockIndex<VbkBlock> index;
    if (!vbk_->acceptBlock(block, state, &index)) {
      return state.addStackFunction("addPayloads");
    }
    stateChange->putVbkBlock(index);
  }

  vbke_->put(payloads);
  stateChange->putVbkEndorsement(payloads);

  return true;
}

void PopManager::removeAltProof(
    const AltProof& alt, std::shared_ptr<StateChange> stateChange) noexcept {
  auto& vbkctx = alt.atv.context;
  std::for_each(
      vbkctx.rbegin(), vbkctx.rend(), [this, &stateChange](const VbkBlock& b) {
        vbk_->invalidateBlockByHash(b.getHash());
        stateChange->removeVbkBlock(b.getHash());
      });

  vbke_->remove(alt);
  stateChange->removeVbkEndorsement(alt);
}

void PopManager::rollback(std::shared_ptr<StateChange> stateChange) noexcept {
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

}  // namespace VeriBlock

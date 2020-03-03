#include <algorithm>
#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace VeriBlock {

bool PopManager::addPayloads(const Payloads& payloads, ValidationState& state) {
  return tryValidateWithResources(
      [&]() {
        /// ADD ALL VTBs
        for (const auto& vtb : payloads.vtbs) {
          if (!this->addVTB(vtb, state)) {
            return state.addStackFunction("addPayloads");
          }
        }

        /// ADD ATV
        if (!this->addAltProof(payloads.alt, state)) {
          return state.addStackFunction("addPayloads");
        }

        uncommitted_.push_back(payloads);

        return true;
      },
      [&]() { this->removePayloads(payloads); });
}

void PopManager::removePayloads(const Payloads& payloads) noexcept {
  /// first, remove ATV
  removeAltProof(payloads.alt);

  /// second, remove VTBs in reverse order
  auto& v = payloads.vtbs;
  std::for_each(
      v.rbegin(), v.rend(), [this](const VTB& vtb) { removeVTB(vtb); });
}

bool PopManager::addVTB(const VTB& vtb, ValidationState& state) {
  if (!checkVTB(vtb, state, *vbkparam_, *btcparam_)) {
    return state.addStackFunction("addVTB");
  }

  // firstly, add btc context blocks
  for (const auto& block : vtb.transaction.blockOfProofContext) {
    if (!btc_->acceptBlock(block, state)) {
      return state.addStackFunction("addVTB");
    }
  }

  // secondly, add VBK endorsements in BTC
  btce_->put(vtb);

  // thirdly, add vbk context blocks
  for (const auto& block : vtb.context) {
    if (!vbk_->acceptBlock(block, state)) {
      return state.addStackFunction("addVTB");
    }
  }

  return true;
}

void PopManager::removeVTB(const VTB& vtb) noexcept {
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

bool PopManager::addAltProof(const AltProof& payloads, ValidationState& state) {
  if (!checkATV(payloads.atv, state, *vbkparam_)) {
    return state.addStackFunction("addPayloads");
  }

  for (const auto& block : payloads.atv.context) {
    if (!vbk_->acceptBlock(block, state)) {
      return state.addStackFunction("addPayloads");
    }
  }

  vbke_->put(payloads);

  return true;
}

void PopManager::removeAltProof(const AltProof& alt) noexcept {
  auto& vbkctx = alt.atv.context;
  std::for_each(vbkctx.rbegin(), vbkctx.rend(), [this](const VbkBlock& b) {
    vbk_->invalidateBlockByHash(b.getHash());
  });

  vbke_->remove(alt);
}

void PopManager::rollback() noexcept {
  // rollback in reverse order
  std::for_each(uncommitted_.rbegin(),
                uncommitted_.rend(),
                [this](const Payloads& p) { removePayloads(p); });

  uncommitted_.clear();
}

void PopManager::commit() { uncommitted_.clear(); }

bool PopManager::hasUncommittedChanges() const noexcept {
  return !uncommitted_.empty();
}

}  // namespace VeriBlock
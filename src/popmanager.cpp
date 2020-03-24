#include <algorithm>
#include <veriblock/blockchain/alt_block_index.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace altintegration {

bool PopManager::addPayloads(const Payloads& payloads, ValidationState& state) {
  return tryValidateWithResources(
      [&]() {
        /// update btc context
        for (const auto& b : payloads.btccontext) {
          if (!this->btc().acceptBlock(b, state)) {
            return state.addStackFunction("addPayloads");
          }
        }

        /// update vbk context
        for (const auto& b : payloads.vbkcontext) {
          if (!this->vbk().acceptBlock(b, state)) {
            return state.addStackFunction("addPayloads");
          }
        }

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

  /// second, remove VTBs
  for (const auto& b : payloads.vtbs) {
    removeVTB(b);
  }

  /// remove vbk context
  for (const auto& b : payloads.vbkcontext) {
    this->vbk().invalidateBlockByHash(b.getHash());
  }

  /// remove btc context
  for (const auto& b : payloads.btccontext) {
    this->btc().invalidateBlockByHash(b.getHash());
  }
}

bool PopManager::addVTB(const VTB& vtb, ValidationState& state) {
  if (!checkVTB(vtb, state, vbkparam_, btcparam_)) {
    return state.addStackFunction("addVTB");
  }

  // firstly, add btc context blocks
  for (const auto& block : vtb.transaction.blockOfProofContext) {
    if (!btc_.acceptBlock(block, state)) {
      return state.addStackFunction("addVTB");
    }
  }

  // thirdly, add vbk context blocks
  for (const auto& block : vtb.context) {
    if (!vbk_.acceptBlock(block, state)) {
      return state.addStackFunction("addVTB");
    }
  }

  return true;
}

void PopManager::removeVTB(const VTB& vtb) noexcept {
  for (const auto& b : vtb.context) {
    vbk_.invalidateBlockByHash(b.getHash());
  }

  for (const auto& b : vtb.transaction.blockOfProofContext) {
    btc_.invalidateBlockByHash(b.getHash());
  }
}

bool PopManager::addAltProof(const AltProof& payloads, ValidationState& state) {
  if (payloads.hasAtv && !checkATV(payloads.atv, state, vbkparam_)) {
    return state.addStackFunction("addPayloads");
  }

  for (const auto& block : payloads.atv.context) {
    if (!vbk_.acceptBlock(block, state)) {
      return state.addStackFunction("addPayloads");
    }
  }

  return true;
}

void PopManager::removeAltProof(const AltProof& alt) noexcept {
  for (const auto& b : alt.atv.context) {
    vbk_.invalidateBlockByHash(b.getHash());
  }
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

int PopManager::compareTwoBranches(const Chain<BlockIndex<AltBlock>>& chain1,
                                   const Chain<BlockIndex<AltBlock>>& chain2) {
  auto pkcChain1 = getProtoKeystoneContext(
      chain1,
      vbk_,
      altparam_,
      [](const BlockIndex<AltBlock>& index) -> std::vector<VbkEndorsement> {
        (void)index;
        return {};
      });
  auto kcChain1 = getKeystoneContext(pkcChain1, vbk_);

  auto pkcChain2 = getProtoKeystoneContext(chain2, vbk_, altparam_, []() {});
  auto kcChain2 = getKeystoneContext(pkcChain2, vbk_);

  return altChainCompare_(kcChain1, kcChain2);
}

}  // namespace altintegration

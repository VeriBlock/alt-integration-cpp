#include <veriblock/finalizer.hpp>
#include <veriblock/popmanager.hpp>

namespace VeriBlock {

bool PopManager::addATVpayloads(
    const ATV& atv,
    ValidationState& state,
    const std::function<AltBlock(const PublicationData&)>& parse) {
  return tryValidateWithResources(
      [&]() -> bool {
        if (!checkATV(atv, state, *vbkparam_)) {
          return state.addStackFunction("addATVpayloads");
        }

        for (const auto& block : atv.context) {
          if (!vbk_->acceptBlock(block, state)) {
            return state.addStackFunction("addATVpayloads");
          }
        }

        auto altblock = parse(atv.transaction.publicationData);

        // TODO: add altchain endorsements

        return true;
      },
      [&]() { this->removeATVpayloads(atv); });
}
bool PopManager::removeATVpayloads(const ATV& atv) {
  (void)atv;

  // TODO: implement this

  return false;
}

bool PopManager::addVTBpayloads(const VTB& vtb, ValidationState& state) {
  return tryValidateWithResources(
      [&]() {
        if (!checkVTB(vtb, state, *vbkparam_, *btcparam_)) {
          return state.addStackFunction("addVTBpayloads");
        }

        // firstly, add btc context blocks
        for (const auto& block : vtb.transaction.blockOfProofContext) {
          if (!btc_->acceptBlock(block, state)) {
            return state.addStackFunction("addVTBpayloads");
          }
        }

        // secondly, add VBK endorsements in BTC
        erepo_->put(vtb.transaction, vtb.containingBlock.getHash());

        // thirdly, add vbk context blocks
        for (const auto& block : vtb.context) {
          if (!vbk_->acceptBlock(block, state)) {
            return state.addStackFunction("addVTBpayloads");
          }
        }

        return true;
      },
      [&]() { this->removeVTBpayloads(vtb); });
}

bool PopManager::removeVTBpayloads(const VTB& vtb) {
  (void)vtb;
  // TODO: implement
  return false;
}

}  // namespace VeriBlock
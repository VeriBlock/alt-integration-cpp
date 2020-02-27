#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/storage/endorsements_repository_inmem.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

/**
 * @invariant does not modify any on-disk state.
 */
struct PopManager {
  PopManager(std::shared_ptr<BtcChainParams> btcp,
             std::shared_ptr<VbkChainParams> vbkp)
      : btcparam_(std::move(btcp)), vbkparam_(std::move(vbkp)) {
    erepo_ = std::make_shared<EndorsementsRepositoryInmem>();
    btc_ = std::make_shared<BtcTree>(btcparam_);
    vbk_ = std::make_shared<VbkTree>(*btc_, erepo_, vbkparam_);
  }

  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using VbkTree = VbkBlockTree;

  BtcTree& btc() { return *btc_; }
  VbkTree& vbk() { return *vbk_; }
  const BtcTree& btc() const { return *btc_; }
  const VbkTree& vbk() const { return *vbk_; }

  bool addATVpayloads(const ATV& atv,
              ValidationState& state,
              const std::function<AltBlock(const PublicationData&)>& parse);
  bool removeATVpayloads(const ATV& atv);

  bool addVTBpayloads(const VTB& vtb, ValidationState& state);
  bool removeVTBpayloads(const VTB& vtb);

 private:
  std::shared_ptr<BtcChainParams> btcparam_;
  std::shared_ptr<VbkChainParams> vbkparam_;

  std::shared_ptr<BtcTree> btc_;
  std::shared_ptr<VbkTree> vbk_;
  std::shared_ptr<EndorsementsRepository> erepo_;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

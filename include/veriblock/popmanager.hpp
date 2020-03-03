#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/storage/endorsement_repository.hpp>
#include <veriblock/storage/vbk_endorsement_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

/**
 *
 * @warning When you validate single ALT block, you MUST first add all VTBs and
 * then add ATV. The other way, may not pass validation.
 *
 * @invariant does not modify any on-disk state.
 */
struct PopManager {
  PopManager(std::shared_ptr<BtcChainParams> btcp,
             std::shared_ptr<VbkChainParams> vbkp,
             std::shared_ptr<EndorsementRepository<BtcEndorsement>> btce,
             std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbke)
      : btcparam_(std::move(btcp)),
        vbkparam_(std::move(vbkp)),
        btce_(std::move(btce)),
        vbke_(std::move(vbke)) {
    btc_ = std::make_shared<BtcTree>(btcparam_);
    vbk_ = std::make_shared<VbkTree>(*btc_, btce_, vbkparam_);
  }

  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using VbkTree = VbkBlockTree;

  BtcTree& btc() { return *btc_; }
  VbkTree& vbk() { return *vbk_; }
  const BtcTree& btc() const { return *btc_; }
  const VbkTree& vbk() const { return *vbk_; }

  /**
   * Atomically add all payloads from ATV and all VTBs.
   * @param payloads altchain to veriblock publication
   * @param state validation state
   * @param parse publication data -> altblock parsing function
   * @return true if payloads added (and valid), false otherwise
   * @invariant payloads ALWAYS added atomically - meaning that if this function
   * returns true, all payloads are fully valid and state has been changed to
   * include new payloads, and if returns false - no state changes made.
   */
  bool addPayloads(const Payloads& payloads, ValidationState& state);

  /**
   * Atomically revert all payloads from given ATV and VTBs.
   * @param atv altchain to veriblock publication
   */
  void removePayloads(const Payloads& payloads) noexcept;

  bool hasUncommittedChanges() const noexcept;

  /// removes all uncommitted payloads
  void rollback() noexcept;

  /// commit currently uncommitted payloads
  void commit();

 private:
  // vector of payloads that have been added to current state,
  // but not committed
  std::vector<Payloads> uncommitted_;

  std::shared_ptr<BtcChainParams> btcparam_;
  std::shared_ptr<VbkChainParams> vbkparam_;

  std::shared_ptr<BtcTree> btc_;
  std::shared_ptr<VbkTree> vbk_;
  std::shared_ptr<EndorsementRepository<BtcEndorsement>> btce_;
  std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbke_;

  bool addVTB(const VTB& vtb, ValidationState& state);
  bool addAltProof(const AltProof& payloads, ValidationState& state);
  void removeAltProof(const AltProof& alt) noexcept;
  void removeVTB(const VTB& vtb) noexcept;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

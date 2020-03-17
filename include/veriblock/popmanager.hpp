#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {
/**
 * @invariant does not modify any on-disk state.
 */
struct PopManager {
  PopManager(std::shared_ptr<BtcChainParams> btcp,
             std::shared_ptr<VbkChainParams> vbkp,
             std::shared_ptr<EndorsementRepository<BtcEndorsement>> btce,
             std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbke,
             std::shared_ptr<AltChainParams> params)
      : btcparam_(std::move(btcp)),
        vbkparam_(std::move(vbkp)),
        btce_(std::move(btce)),
        vbke_(std::move(vbke)),
        altChainParams_(params),
        altChainCompare_(*params) {
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
   * @param payloads publications
   * @param state validation state
   * @param parse publication data -> altblock parsing function
   * @return true if payloads added (and valid), false otherwise
   * @invariant payloads ALWAYS added atomically - meaning that if this function
   * returns true, all payloads are fully valid and state has been changed to
   * include new payloads, and if returns false - no state changes made.
   * @throws may throw if out of memory. In this case, payloads also will be
   * reverted.
   */
  bool addPayloads(const Payloads& payloads,
                   StateChange& stateChange,
                   ValidationState& state);

  /**
   * Atomically revert all payloads from given ATV and VTBs.
   * @param atv altchain to veriblock publication
   * @note does not throw in any circumstance
   */
  void removePayloads(const Payloads& payloads,
                      StateChange& stateChange) noexcept;

  bool hasUncommittedChanges() const noexcept;

  /// removes all uncommitted payloads
  void rollback(StateChange& stateChange) noexcept;

  /// commit currently uncommitted payloads
  void commit();

  /**
   * Determine the best chain of the AltBlocks in accordance with the VeriBlock
   * forkresolution rules
   * @param AltBlock chain1, AltBlock chain2
   * @return '> 0' number if chain1 is better, '< 0' number if chain2 is better,
   * '0' if they are the same
   * @note chain1 and chain2 are being consindered as forks not a full chains
   * from the genesis block, they should start at the common block
   */
  int compareTwoBranches(const Chain<AltBlock>& chain1,
                         const Chain<AltBlock>& chain2);

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

  std::shared_ptr<AltChainParams> altChainParams_;
  ComparePopScore<AltChainParams> altChainCompare_;

  bool addVTB(const VTB& vtb, StateChange& stateChange, ValidationState& state);
  bool addAltProof(const AltProof& payloads,
                   StateChange& stateChange,
                   ValidationState& state);
  void removeAltProof(const AltProof& alt, StateChange& stateChange) noexcept;
  void removeVTB(const VTB& vtb, StateChange& stateChange) noexcept;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_POPMANAGER_HPP_

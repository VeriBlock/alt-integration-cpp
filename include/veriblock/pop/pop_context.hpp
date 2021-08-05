// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALTINTEGRATION_HPP
#define ALTINTEGRATION_ALTINTEGRATION_HPP

#include <utility>

#include "alt-util.hpp"
#include "blockchain/alt_block_tree.hpp"
#include "bootstraps.hpp"
#include "config.hpp"
#include "mempool.hpp"
#include "pop_stateless_validator.hpp"
#include "rewards/default_poprewards_calculator.hpp"
#include "storage.hpp"

namespace altintegration {

/**
 * @struct PopContext
 *
 * Main entrypoint into the veriblock-pop library.
 */
struct PopContext {
  ~PopContext();

  // non-copyable
  PopContext(const PopContext&) = delete;
  PopContext& operator=(const PopContext&) = delete;

  // movable
  PopContext(PopContext&&) = default;
  PopContext& operator=(PopContext&&) = default;

  /**
   * Factory function for PopContext.
   *
   * @param[in] config
   * @param[in] payloadsProvider
   * @param[in] blockProvider
   * @param[in] validatorWorkers
   *
   * @return
   */
  static std::shared_ptr<PopContext> create(
      std::shared_ptr<Config> config,
      std::shared_ptr<PayloadsStorage> payloadsProvider,
      std::shared_ptr<BlockReader> blockProvider,
      size_t validatorWorkers = 0);

  /**
   * Checks ATV (stateless check).
   *
   * @param[in] payload ATV to be checked
   * @param[out] state
   *
   * @return true if ATV is statelessly valid, false otherwise
   */
  VBK_CHECK_RETURN bool check(const ATV& payload, ValidationState& state);

  /**
   * Checks VTB (stateless check).
   *
   * @param[in] payload VTB to be checked
   * @param[out] state
   *
   * @return true if VTB is statelessly valid, false otherwise
   */
  VBK_CHECK_RETURN bool check(const VTB& payload, ValidationState& state);

  /**
   * Checks VbkBlock (stateless check).
   *
   * @param[in] payload VbkBlock to be checked
   * @param[out] state
   *
   * @return true if VbkBlock is statelessly valid, false otherwise
   */
  VBK_CHECK_RETURN bool check(const VbkBlock& payload, ValidationState& state);

  /**
   * Checks PopData (stateless check).
   *
   * @param[in] payload PopData to be checked
   * @param[out] state
   *
   * @return true if PopData is statelessly valid, false otherwise
   */
  VBK_CHECK_RETURN bool check(const PopData& payload, ValidationState& state);

  /**
   * Save ALT/VBK/BTC trees on disk via adapter BlockBatch.
   *
   * @param[out] batch adaptor for writing blocks on disk.
   */
  void saveAllTrees(BlockBatch& batch) const;

  /**
   * Load ALT/VBK/BTC trees from disk via adapter BlockReader.
   *
   * @param[out] state validation state
   */
  VBK_CHECK_RETURN bool loadAllTrees(ValidationState& state);

  /**
   * Calculates POP rewards that should be paid in the `tip` block.
   *
   * @param[in] tip Altchain has to supply prev block of a block that will
   * contain POP reward.
   * @param[out] rewards a map where key=payoutInfo from PublicationData, value=amount to be
   * paid.
   * @param[in] state validation state.
   *
   * @return true if rewards are obtained; false otherwise.
   */
  VBK_CHECK_RETURN bool getPopPayout(const AltBlockTree::hash_t& tip,
                    PopPayouts& rewards,
                    ValidationState& state);

  /**
   * Generate PopData for the block next to the current tip.
   *
   * @return empty or non-empty PopData instance that must be inserted into next
   * block body.
   */
  VBK_CHECK_RETURN PopData generatePopData();

  /**
   * Create PublicationData given required input parameters.
   *
   * @param[out] output generated publication data output. Valid only if this
   * func returned true.
   * @param[in] endorsedBlockHeader serialized altchain header of endorsed block
   * (for BTC it is 80 bytes).
   * @param[in] txMerkleRoot transaction merkle root (used original merkle root
   * value from the block header).
   * @param[in] popData popData that is stored in endorsed block.
   * @param[in] payoutInfo bytes that will then be interpreted by altchain as
   * payout information (for BTC can be a script...).
   *
   * @return true if endorsed block exists, false otherwise.
   */
  VBK_CHECK_RETURN bool generatePublicationData(
      PublicationData& output,
      const std::vector<uint8_t>& endorsedBlockHeader,
      const std::vector<uint8_t>& txMerkleRoot,
      const PopData& popData,
      const std::vector<uint8_t>& payoutInfo);

  VBK_CHECK_RETURN const Config& getConfig() const;
  VBK_CHECK_RETURN MemPool& getMemPool();
  VBK_CHECK_RETURN AltBlockTree& getAltBlockTree();
  VBK_CHECK_RETURN const AltBlockTree& getAltBlockTree() const;
  VBK_CHECK_RETURN const VbkBlockTree& getVbkBlockTree() const;
  VBK_CHECK_RETURN const BtcBlockTree& getBtcBlockTree() const;

  //! stops PopContext internal thread pool used for stateless validation.
  void shutdown();

 private:
  PopContext() = default;

  std::shared_ptr<Config> config_;
  std::shared_ptr<MemPool> mempool_;
  std::shared_ptr<AltBlockTree> altTree_;
  std::shared_ptr<PopValidator> popValidator_;
  std::shared_ptr<PopRewardsCalculator> popRewardsCalculator_;
  std::shared_ptr<PayloadsStorage> payloadsProvider_;
  std::shared_ptr<BlockReader> blockProvider_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALTINTEGRATION_HPP

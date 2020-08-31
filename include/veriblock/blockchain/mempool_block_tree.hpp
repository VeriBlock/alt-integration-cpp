// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP
#define ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP

#include <unordered_map>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/block_index.hpp"

namespace altintegration {

struct MemPoolBlockTree {
  MemPoolBlockTree(const AltTree& tree) : tree_(tree) { (void)tree_; }
  MemPoolBlockTree(const MemPoolBlockTree& tree) : tree_(tree.tree_) {}

  /**
   * Compares ATVs for the strongly equivalence
   *
   * @param[in] first ATV
   * @param[in] second ATV
   * @return
   * Returns true if ATVs strongly equivalent, otherwise returns false
   */
  bool areStronglyEquivalent(const ATV& atv1, const ATV& atv2);

  /**
   * Compares VTBs for the strongly equivalence
   *
   * @param[in] first VTB
   * @param[in] second VTB
   * @return
   * Returns true if VTB strongly equivalent, otherwise returns false
   */
  bool areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2);

  /**
   * Compares VTBs for the weakly equivalence
   *
   * @param[in] first VTB
   * @param[in] second VTB
   * @return
   * Returns true if VTBs weakly equivalent, otherwise returns false
   */
  bool areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2);

  /**
   * Compare two vtbs that are weakly equivalent.
   * @param[in] first VTB to compare
   * @param[in] second VTB to compare
   * @return:
   * Return positive if vtb1 is better
   * Return negative if vtb2 is better
   * Return 0 if they are strongly equal
   */
  int weaklyCompare(const VTB& vtb1, const VTB& vtb2);

 private:
  const AltTree& tree_;
};

}  // namespace altintegration

#endif
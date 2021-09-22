// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_RELATIONS_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_RELATIONS_HPP

#include <memory>
#include <utility>
#include <vector>

#include "entities/atv.hpp"
#include "entities/popdata.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "blockchain/alt_block_tree.hpp"

namespace altintegration {

//! @private
struct VbkPayloadsRelations {
  using id_t = VbkBlock::id_t;
  using height_t = typename VbkBlock::height_t;

  struct EndorsedAltComparator {
    EndorsedAltComparator(const AltBlockTree& tree) : tree_(tree) {}
    int operator()(const std::shared_ptr<ATV>& a,
                   const std::shared_ptr<ATV>& b) const;

   private:
    const AltBlockTree& tree_;
  };

  struct TxFeeComparator {
    int operator()(const std::shared_ptr<ATV>& a,
                   const std::shared_ptr<ATV>& b) const;
  };

  struct AtvCombinedComparator {
    AtvCombinedComparator(const AltBlockTree& tree) : txFeeComparator(), endorsedAltComparator(tree) {}
    bool operator()(const std::shared_ptr<ATV>& a,
                    const std::shared_ptr<ATV>& b) const;

   private:
    TxFeeComparator txFeeComparator;
    EndorsedAltComparator endorsedAltComparator;
  };

  VbkPayloadsRelations(const AltBlockTree& tree, const VbkBlock& b)
      : tree_(tree),
        header(std::make_shared<VbkBlock>(b)),
        atvComparator(tree),
        atvs(atvComparator) {}

  VbkPayloadsRelations(const AltBlockTree& tree,
                       std::shared_ptr<VbkBlock> ptr_b)
      : tree_(tree),
        header(std::move(ptr_b)),
        atvComparator(tree),
        atvs(atvComparator) {}

  const AltBlockTree& tree_;
  std::shared_ptr<VbkBlock> header;
  std::vector<std::shared_ptr<VTB>> vtbs;
  AtvCombinedComparator atvComparator;
  std::set<std::shared_ptr<ATV>, AtvCombinedComparator> atvs;

  bool empty() const { return atvs.empty() && vtbs.empty(); }
};

}  // namespace altintegration

#endif
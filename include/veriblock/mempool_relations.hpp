// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_RELATIONS_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_RELATIONS_HPP

#include <memory>
#include <vector>

#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"

namespace altintegration {

struct VbkPayloadsRelations {
  using id_t = VbkBlock::id_t;
  using height_t = typename VbkBlock::height_t;

  VbkPayloadsRelations(const VbkBlock& b)
      : header(std::make_shared<VbkBlock>(b)) {}

  VbkPayloadsRelations(const std::shared_ptr<VbkBlock>& ptr_b)
      : header(ptr_b) {}

  std::shared_ptr<VbkBlock> header;
  std::vector<std::shared_ptr<VTB>> vtbs;
  std::vector<std::shared_ptr<ATV>> atvs;

  PopData toPopData() const;

  bool empty() const { return atvs.empty() && vtbs.empty(); }

  void removeVTB(const VTB::id_t& vtb_id);
  void removeATV(const ATV::id_t& atv_id);
};

}  // namespace altintegration

#endif
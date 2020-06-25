// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBKFULLBLOCK_HPP
#define VERIBLOCK_POP_CPP_VBKFULLBLOCK_HPP

#include <utility>

#include "atv.hpp"
#include "vbkblock.hpp"
#include "vtb.hpp"

namespace altintegration {

// entity that represents partially filled VBK full block.
// it is guaranteed that this block contains payloads, which have
// containingBlock == header
struct VbkFullBlock {
  VbkFullBlock(VbkBlock h) : header(std::move(h)) {}

  void addATV(const ATV& atv) {
    VBK_ASSERT(atv.containingBlock == header);
    atvs.push_back(atv);
  }

  void addVTB(const VTB& vtb) {
    VBK_ASSERT(vtb.containingBlock == header);
    vtbs.push_back(vtb);
  }

  const std::vector<VTB>& getVtbs() const { return vtbs; }

  const std::vector<ATV>& getAtvs() const { return atvs; }

 private:
  VbkBlock header;
  std::vector<VTB> vtbs;
  std::vector<ATV> atvs;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBKFULLBLOCK_HPP

// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FACADE_POP_MEMPOOL_HPP
#define VERIBLOCK_POP_CPP_FACADE_POP_MEMPOOL_HPP

#include <veriblock/pop/entities/atv.hpp>

namespace altintegration {

struct PopMempool {
  bool submit(std::shared_ptr<ATV> atv, ValidationState& state);
  bool submit(std::shared_ptr<VTB> vtb, ValidationState& state);
  bool submit(std::shared_ptr<VbkBlock> vbk, ValidationState& state);
};

}

#endif  // VERIBLOCK_POP_CPP_FACADE_POP_MEMPOOL_HPP

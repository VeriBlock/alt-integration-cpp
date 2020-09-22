// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VERIBLOCK_HPP
#define VERIBLOCK_POP_CPP_VERIBLOCK_HPP

#include <veriblock/entities/vbkblock.hpp>

namespace altintegration {

VbkBlock::hash_t getVbkBlockHash(Slice<const uint8_t> vbkblock);

}

#endif  // VERIBLOCK_POP_CPP_VERIBLOCK_HPP

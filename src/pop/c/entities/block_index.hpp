// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ENTITIES_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_C_ENTITIES_BLOCK_INDEX_HPP

#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/c/entities/block_index.h"
#include "veriblock/pop/entities/altblock.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"

struct __pop_alt_block_index {
  const altintegration::BlockIndex<altintegration::AltBlock>& ref;
};

struct __pop_vbk_block_index {
  const altintegration::BlockIndex<altintegration::VbkBlock>& ref;
};

struct __pop_btc_block_index {
  const altintegration::BlockIndex<altintegration::BtcBlock>& ref;
};

#endif

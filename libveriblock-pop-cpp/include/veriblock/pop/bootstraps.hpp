// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP
#define VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP

#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"

namespace altintegration {

//! Getter for `regtest_progpow` VBK network genesis block.
extern VbkBlock GetRegTestVbkBlock();

//! Getter for `regtest` BTC network genesis block.
extern BtcBlock GetRegTestBtcBlock();

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP

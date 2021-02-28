// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP
#define VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP

#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"

namespace altintegration {

// corresponds to "regtest_progpow" VBK network
extern VbkBlock GetRegTestVbkBlock();
extern BtcBlock GetRegTestBtcBlock();

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BOOTSTRAPS_HPP

// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_TXFEES_HPP
#define ALT_INTEGRATION_VERIBLOCK_TXFEES_HPP

#include "entities/coin.hpp"
#include "entities/vbktx.hpp"

namespace altintegration {

Coin calculateTxFee(const VbkTx& tx);

}  // namespace altintegration

#endif //ALT_INTEGRATION_VERIBLOCK_TXFEES_HPP

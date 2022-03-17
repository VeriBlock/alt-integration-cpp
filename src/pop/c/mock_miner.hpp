// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MOCK_MINER_HPP
#define VERIBLOCK_POP_CPP_C_MOCK_MINER_HPP

#include "veriblock/pop/c/mock_miner.h"
#include "veriblock/pop/mock_miner.hpp"

struct __pop_mock_miner {
  static altintegration::AltChainParamsRegTest alt_param;
  static altintegration::VbkChainParamsRegTest vbk_param;
  static altintegration::BtcChainParamsRegTest btc_param;

  altintegration::MockMiner ref{alt_param, vbk_param, btc_param};
};

#endif
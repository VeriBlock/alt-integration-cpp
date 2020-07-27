// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONTEXT_HPP
#define VERIBLOCK_POP_CPP_CONTEXT_HPP

namespace altintegration {

extern void RequestShutdown();

//! warning!!! should be used in tests ONLY. It is NOT guaranteed that state
//! will be correct when shutdown is aborted.
extern void AbortShutdown();

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTEXT_HPP

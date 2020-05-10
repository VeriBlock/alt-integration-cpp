// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_SIGNALS_HPP
#define VERIBLOCK_POP_CPP_SIGNALS_HPP

#include <veriblock/third_party/Signals.hpp>

//! a facade for Signals

namespace altintegration {
namespace signals {

template <typename SignalSignature>
using Signal = Simple::Signal<SignalSignature>;

}  // namespace signals

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_SIGNALS_HPP

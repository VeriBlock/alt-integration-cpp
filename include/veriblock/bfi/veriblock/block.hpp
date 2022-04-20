// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_VERIBLOCK_BLOCK_HPP
#define BFI_VERIBLOCK_BLOCK_HPP

#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace core {
// forward declaration
class RpcBitcoinBlockHeader;
class RpcBlock;
class RpcBlockHeader;
}  // namespace core

namespace altintegration {
namespace vbk {

bool ConvertFromProto(const core::RpcBitcoinBlockHeader& from,
                      BtcBlock& to,
                      ValidationState& state);

bool ConvertFromProto(const core::RpcBlock& from,
                      VbkBlock& to,
                      ValidationState& state);

bool ConvertFromProto(const core::RpcBlockHeader& from,
                      VbkBlock& to,
                      ValidationState& state);

}  // namespace vbk
}  // namespace altintegration

#endif
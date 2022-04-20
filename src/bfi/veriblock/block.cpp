// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock.pb.h>

#include <veriblock/bfi/veriblock/block.hpp>

namespace altintegration {
namespace vbk {

bool ConvertFromProto(const core::RpcBitcoinBlockHeader& from,
                      BtcBlock& to,
                      ValidationState& state) {
  if (!DeserializeFromHex(from.header(), to, state)) {
    return state.Invalid("bad-proto-btc-block-header");
  }

  return true;
}

bool ConvertFromProto(const core::RpcBlock& from,
                      VbkBlock& to,
                      ValidationState& state) {
  // TOOD implement
  return true;
}

bool ConvertFromProto(const core::RpcBlockHeader& from,
                      VbkBlock& to,
                      ValidationState& state) {
  if (!DeserializeFromHex(from.header(), to, state)) {
    return state.Invalid("bad-proto-vbk-block-header");
  }

  return true;
}

}  // namespace vbk
}  // namespace altintegration

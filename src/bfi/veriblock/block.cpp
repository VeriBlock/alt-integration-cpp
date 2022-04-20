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
  if (!DeserializeFromVbkEncoding(from.header(), to, state)) {
    return state.Invalid("bad-proto-bitcoin-block-header");
  }

  return true;
}

}  // namespace vbk
}  // namespace altintegration

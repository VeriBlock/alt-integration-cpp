// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock.pb.h>

#include <veriblock/bfi/veriblock/transaction.hpp>

namespace altintegration {

namespace vbk {

struct ProtoOtput {
  core::RpcOutput val;
};

bool ConvertFromProto(const ProtoOtput& from,
                      Output& to,
                      ValidationState& state) {
  if (!to.address.fromString(from.val.address(), state)) {
    return state.Invalid("bad-proto-otput-address");
  }
  to.coin = Coin(from.val.amount());

  return true;
}

void ConvertToProto(const Output& from, ProtoOtput& to) {
  to.val.set_amount(from.coin.units);
  to.val.set_address(from.address.toString());
}

struct ProtoTransaction {
  core::RpcSignedTransaction val;
};

bool ConvertFromProto(const ProtoTransaction& from,
                      VbkTx& to,
                      ValidationState& state) {
  if (!to.sourceAddress.fromString(from.val.transaction().source_address(),
                                   state)) {
    return state.Invalid("bad-proto-transaction-sourceAddress");
  }

  for (const auto& output : from.val.transaction().outputs()) {
    ProtoOtput proto{output};
    Output val;
    if (!ConvertFromProto(proto, val, state)) {
      return state.Invalid("bad-proto-transaction-output");
    }
    to.outputs.push_back(val);
  }

  to.sourceAmount = Coin(from.val.transaction().source_amount());

  return true;
}

void ConvertToProto(const VbkTx& from, ProtoTransaction& to) {}

}  // namespace vbk

}  // namespace altintegration
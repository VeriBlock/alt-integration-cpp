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

struct ProtoTransaction {
  core::RpcSignedTransaction val;
};

bool ConvertFromProto(const ProtoTransaction& from,
                      VbkTx& to,
                      ValidationState& state) {
  if (from.val.transaction().type() !=
      core::RpcTransaction_Type::RpcTransaction_Type_STANDARD) {
    return state.Invalid("bad-proto-transaction-type");
  }

  // TODO: need to setup VbkTx::networkOrType field

  if (!to.sourceAddress.fromString(from.val.transaction().source_address(),
                                   state)) {
    return state.Invalid("bad-proto-transaction-source_address");
  }

  for (const auto& output : from.val.transaction().outputs()) {
    ProtoOtput proto{output};
    Output val;
    if (!ConvertFromProto(proto, val, state)) {
      return state.Invalid("bad-proto-transaction-output");
    }
    to.outputs.push_back(val);
  }

  if (!IsHex(from.val.signature())) {
    return state.Invalid("bad-proto-transaction-signature",
                         "signature key data is not hex");
  } else {
    to.signature = ParseHex(from.val.signature());
  }

  if (!IsHex(from.val.public_key())) {
    return state.Invalid("bad-proto-transaction-public_key",
                         "Public key data is not hex");
  } else {
    to.publicKey = ParseHex(from.val.public_key());
  }

  if (!DeserializeFromVbkEncoding(
          from.val.transaction().data(), to.publicationData, state)) {
    return state.Invalid("bad-proto-transaction-data");
  }

  to.signatureIndex = from.val.signature_index();

  to.sourceAmount = Coin(from.val.transaction().source_amount());

  return true;
}

bool ConvertFromProto(const ProtoTransaction& from,
                      VbkPopTx& to,
                      ValidationState& state) {
  if (from.val.transaction().type() !=
      core::RpcTransaction_Type::RpcTransaction_Type_PROOF_OF_PROOF) {
    return state.Invalid("bad-proto-transaction-type");
  }

  // TODO: need to setup VbkPopTx::networkOrType field

  if (!to.address.fromString(from.val.transaction().source_address(), state)) {
    return state.Invalid("bad-proto-transaction-source_address");
  }

  if (!DeserializeFromVbkEncoding(from.val.transaction().bitcoin_transaction(),
                                  to.bitcoinTransaction,
                                  state)) {
    return state.Invalid("bad-proto-transaction-bitcoin_transaction");
  }

  if (!DeserializeFromVbkEncoding(
          from.val.transaction().endorsed_block_header(),
          to.publishedBlock,
          state)) {
    return state.Invalid("bad-proto-transaction-endorsed_block_header");
  }

  // TODO: need to setup VbkPopTx::merklePath field

  if (!IsHex(from.val.signature())) {
    return state.Invalid("bad-proto-transaction-signature",
                         "signature key data is not hex");
  } else {
    to.signature = ParseHex(from.val.signature());
  }

  if (!IsHex(from.val.public_key())) {
    return state.Invalid("bad-proto-transaction-public_key",
                         "Public key data is not hex");
  } else {
    to.publicKey = ParseHex(from.val.public_key());
  }

  return true;
}

}  // namespace vbk

}  // namespace altintegration
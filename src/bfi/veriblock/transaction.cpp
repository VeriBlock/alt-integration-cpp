// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock.pb.h>

#include <veriblock/bfi/veriblock/block.hpp>
#include <veriblock/bfi/veriblock/transaction.hpp>

namespace altintegration {

namespace vbk {

bool ConvertFromProto(const core::RpcOutput& from,
                      Output& to,
                      ValidationState& state) {
  if (!to.address.fromString(from.address(), state)) {
    return state.Invalid("bad-proto-otput-address");
  }
  to.coin = Coin(from.amount());

  return true;
}

bool ConvertFromProto(const core::RpcSignedTransaction& from,
                      VbkTx& to,
                      ValidationState& state) {
  if (from.transaction().type() !=
      core::RpcTransaction_Type::RpcTransaction_Type_STANDARD) {
    return state.Invalid("bad-proto-transaction-type");
  }

  // TODO: need to setup VbkTx::networkOrType field

  if (!to.sourceAddress.fromString(from.transaction().source_address(),
                                   state)) {
    return state.Invalid("bad-proto-transaction-source_address");
  }

  for (const auto& output : from.transaction().outputs()) {
    Output val;
    if (!ConvertFromProto(output, val, state)) {
      return state.Invalid("bad-proto-transaction-output");
    }
    to.outputs.push_back(val);
  }

  if (!IsHex(from.signature())) {
    return state.Invalid("bad-proto-transaction-signature",
                         "signature key data is not hex");
  } else {
    to.signature = ParseHex(from.signature());
  }

  if (!IsHex(from.public_key())) {
    return state.Invalid("bad-proto-transaction-public_key",
                         "Public key data is not hex");
  } else {
    to.publicKey = ParseHex(from.public_key());
  }

  if (!DeserializeFromVbkEncoding(
          from.transaction().data(), to.publicationData, state)) {
    return state.Invalid("bad-proto-transaction-data");
  }

  to.signatureIndex = from.signature_index();

  to.sourceAmount = Coin(from.transaction().source_amount());

  return true;
}

bool ConvertFromProto(const core::RpcSignedTransaction& from,
                      VbkPopTx& to,
                      ValidationState& state) {
  if (from.transaction().type() !=
      core::RpcTransaction_Type::RpcTransaction_Type_PROOF_OF_PROOF) {
    return state.Invalid("bad-proto-transaction-type");
  }

  // TODO: need to setup VbkPopTx::networkOrType field

  if (!to.address.fromString(from.transaction().source_address(), state)) {
    return state.Invalid("bad-proto-transaction-source_address");
  }

  if (!DeserializeFromVbkEncoding(from.transaction().bitcoin_transaction(),
                                  to.bitcoinTransaction,
                                  state)) {
    return state.Invalid("bad-proto-transaction-bitcoin_transaction");
  }

  if (!DeserializeFromVbkEncoding(from.transaction().endorsed_block_header(),
                                  to.publishedBlock,
                                  state)) {
    return state.Invalid("bad-proto-transaction-endorsed_block_header");
  }

  if (!ConvertFromProto(from.transaction().bitcoin_block_header_of_proof(),
                        to.blockOfProof,
                        state)) {
    return state.Invalid("bad-proto-transaction-bitcoin_block_header_of_proof");
  }

  for (const auto& output :
       from.transaction().context_bitcoin_block_headers()) {
    BtcBlock val;
    if (!ConvertFromProto(output, val, state)) {
      return state.Invalid("bad-proto-transaction-output");
    }
    to.blockOfProofContext.push_back(val);
  }

  // TODO: need to setup VbkPopTx::merklePath field

  if (!IsHex(from.signature())) {
    return state.Invalid("bad-proto-transaction-signature",
                         "signature key data is not hex");
  } else {
    to.signature = ParseHex(from.signature());
  }

  if (!IsHex(from.public_key())) {
    return state.Invalid("bad-proto-transaction-public_key",
                         "Public key data is not hex");
  } else {
    to.publicKey = ParseHex(from.public_key());
  }

  return true;
}

}  // namespace vbk

}  // namespace altintegration
// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/endorsements.hpp"

#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/serde.hpp"

namespace altintegration {

template <>
VbkEndorsement VbkEndorsement::fromContainer(const VTB& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
  e.payoutInfo = {};
  return e;
}

template <>
AltEndorsement AltEndorsement::fromContainer(
    const ATV& c,
    const std::vector<uint8_t>& containingHash,
    const std::vector<uint8_t>& endorsedHash) {
  AltEndorsement e;
  e.id = AltEndorsement::getId(c);
  e.blockOfProof = c.blockOfProof.getHash();
  e.endorsedHash = endorsedHash;
  e.containingHash = containingHash;
  e.payoutInfo = c.transaction.publicationData.payoutInfo;
  return e;
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const VTB& c) {
  return c.getId();
}

template <>
AltEndorsement::id_t AltEndorsement::getId(const ATV& c) {
  return c.getId();
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AltEndorsement& out,
                                ValidationState& state) {
  if (!readSingleByteLenValue(
          stream, out.id, state, ATV_ID_SIZE, ATV_ID_SIZE)) {
    return state.Invalid("bad-altendorsement-id");
  }

  if (!readSingleByteLenValue(stream,
                              out.endorsedHash,
                              state,
                              MIN_ALT_HASH_SIZE,
                              MAX_ALT_HASH_SIZE)) {
    return state.Invalid("bad-altendorsement-endorsed-hash");
  }

  if (!readSingleByteLenValue(stream,
                              out.containingHash,
                              state,
                              MIN_ALT_HASH_SIZE,
                              MAX_ALT_HASH_SIZE)) {
    return state.Invalid("bad-altendorsement-containing-hash");
  }

  if (!readSingleByteLenValue(stream,
                              out.blockOfProof,
                              state,
                              VBK_BLOCK_HASH_SIZE,
                              VBK_BLOCK_HASH_SIZE)) {
    return state.Invalid("bad-altendorsement-block-of-proof");
  }

  if (!readSingleByteLenValue(stream,
                              out.payoutInfo,
                              state,
                              0,
                              MAX_PAYOUT_INFO_SIZE)) {
    return state.Invalid("bad-altendorsement-payout-info");
  }

  return true;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkEndorsement& out,
                                ValidationState& state) {
  if (!readSingleByteLenValue(
          stream, out.id, state, VTB_ID_SIZE, VTB_ID_SIZE)) {
    return state.Invalid("bad-vbkendorsement-id");
  }
  if (!readSingleByteLenValue(stream,
                              out.endorsedHash,
                              state,
                              VBK_BLOCK_HASH_SIZE,
                              VBK_BLOCK_HASH_SIZE)) {
    return state.Invalid("bad-vbkendorsement-endorsed-hash");
  }
  if (!readSingleByteLenValue(stream,
                              out.containingHash,
                              state,
                              VBK_BLOCK_HASH_SIZE,
                              VBK_BLOCK_HASH_SIZE)) {
    return state.Invalid("bad-vbkendorsement-containing-hash");
  }
  if (!readSingleByteLenValue(stream,
                              out.blockOfProof,
                              state,
                              BTC_BLOCK_HASH_SIZE,
                              BTC_BLOCK_HASH_SIZE)) {
    return state.Invalid("bad-vbkendorsement-block-of-proof");
  }

  // this one should be empty
  if (!readSingleByteLenValue(stream,
                              out.payoutInfo,
                              state,
                              0,
                              0)) {
    return state.Invalid("bad-vbkendorsement-payout-info");
  }
  return true;
}

}  // namespace altintegration

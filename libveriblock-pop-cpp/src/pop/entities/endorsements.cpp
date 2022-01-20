// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/serde.hpp>

namespace altintegration {

template <>
VbkEndorsement VbkEndorsement::fromContainer(const VTB& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
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

  return true;
}

}  // namespace altintegration

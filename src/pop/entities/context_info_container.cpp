// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/entities/context_info_container.hpp>

namespace altintegration {

void ContextInfoContainer::toVbkEncoding(WriteStream& w) const {
  w.writeBE<int32_t>(height);
  keystones.toVbkEncoding(w);
}

size_t ContextInfoContainer::estimateSize() const {
  return sizeof(height) + keystones.estimateSize();
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                ContextInfoContainer& ctx,
                                ValidationState& state) {
  if (!stream.readBE<int32_t>(ctx.height, state)) {
    return state.Invalid("bad-height");
  }

  if (!DeserializeFromVbkEncoding(stream, ctx.keystones, state)) {
    return state.Invalid("bad-keystones");
  }

  return true;
}

ContextInfoContainer ContextInfoContainer::createFromPrevious(
    const BlockIndex<AltBlock>* prev, const AltChainParams& params) {
  ContextInfoContainer c;

  const auto bootstrapHeight = params.getBootstrapBlock().getHeight();
  c.height = (prev == nullptr) ? bootstrapHeight : prev->getHeight() + 1;

  const auto ki = params.getKeystoneInterval();
  c.keystones = KeystoneContainer::createFromPrevious(prev, ki);

  return c;
}

uint256 ContextInfoContainer::getHash() const {
  WriteStream w(100);
  toVbkEncoding(w);
  return sha256twice(w.data());
}

std::string ContextInfoContainer::toPrettyString() const {
  return format("ContextInfo(height={} ks1={} ks2={})",
                height,
                HexStr(keystones.firstPreviousKeystone),
                HexStr(keystones.secondPreviousKeystone));
}

void AuthenticatedContextInfoContainer::toVbkEncoding(WriteStream& w) const {
  ctx.toVbkEncoding(w);
  w.write(stateRoot);
}

size_t AuthenticatedContextInfoContainer::estimateSize() const {
  return ctx.estimateSize() + stateRoot.size();
}

uint256 AuthenticatedContextInfoContainer::getTopLevelMerkleRoot() const {
  auto ctxHash = ctx.getHash();
  return sha256twice(stateRoot, ctxHash);
}

AuthenticatedContextInfoContainer
AuthenticatedContextInfoContainer::createFromPrevious(
    const uint256& stateRoot,
    const BlockIndex<AltBlock>* prev,
    const AltChainParams& p) {
  AuthenticatedContextInfoContainer c;
  c.stateRoot = stateRoot;
  c.ctx = ContextInfoContainer::createFromPrevious(prev, p);
  return c;
}

AuthenticatedContextInfoContainer
AuthenticatedContextInfoContainer::createFromPrevious(
    const std::vector<uint8_t>& txRoot,
    const uint256& popDataRoot,
    const BlockIndex<AltBlock>* prev,
    const AltChainParams& p) {
  auto stateRoot = sha256twice(txRoot, popDataRoot);
  return createFromPrevious(stateRoot, prev, p);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AuthenticatedContextInfoContainer& ctx,
                                ValidationState& state) {
  if (!DeserializeFromVbkEncoding(stream, ctx.ctx, state)) {
    return state.Invalid("bad-ctx");
  }

  if (!stream.read(ctx.stateRoot.size(), ctx.stateRoot.data(), state)) {
    return state.Invalid("bad-stateroot");
  }

  return true;
}

}  // namespace altintegration
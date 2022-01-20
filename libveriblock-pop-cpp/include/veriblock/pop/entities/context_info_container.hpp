// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONTEXT_INFO_CONTAINER_HPP
#define VERIBLOCK_POP_CPP_CONTEXT_INFO_CONTAINER_HPP

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/serde.hpp>

#include "keystone_container.hpp"

namespace altintegration {

/**
 * Container of context info for endorsed block.
 */
struct ContextInfoContainer {
  //! endorsed block height
  int height = 0;

  //! endorsed block previous keystones
  KeystoneContainer keystones{};

  static ContextInfoContainer createFromPrevious(
      const BlockIndex<AltBlock>* prev, const AltChainParams& param);

  uint256 getHash() const;

  void toVbkEncoding(WriteStream& w) const;
  size_t estimateSize() const;

  bool operator==(const ContextInfoContainer& other) const {
    return height == other.height && keystones == other.keystones;
  }

  bool operator!=(const ContextInfoContainer& other) const {
    return !(this->operator==(other));
  }

  std::string toPrettyString() const;
};

/**
 * Contains ContextInfoContainer and merkle path which authenticates hash of
 * ContextInfoContainer to a block header.
 *
 * ```
 * auto tlmr = getTopLevelMerkleRoot(header);
 * auto r = sha256d(stateRoot, context.getHash());
 * if(r == tlmr) {
 *   // valid!
 * } esle {
 *   // invalid!
 * }
 * ```
 */
struct AuthenticatedContextInfoContainer {
  ContextInfoContainer ctx{};
  // state root = sha256d(original merkle root || popdata merkle root)
  // This is a merkle path which consists of 1 node. This merkle path
  // authenticates 'ctx' to 'endorsed block header' in
  // PublicationData::contextInfo
  uint256 stateRoot{};

  //! @param stateRoot equals to sha256d(merkle root || pop data merkle root)
  //! @param prev is a pointer to previous block
  //! @param p params
  static AuthenticatedContextInfoContainer createFromPrevious(
      const uint256& stateRoot,
      const BlockIndex<AltBlock>* prev,
      const AltChainParams& p);

  //! @overload
  static AuthenticatedContextInfoContainer createFromPrevious(
      const std::vector<uint8_t>& txRoot,
      const uint256& popDataRoot,
      const BlockIndex<AltBlock>* prev,
      const AltChainParams& p);

  bool operator==(const AuthenticatedContextInfoContainer& other) const {
    return ctx == other.ctx && stateRoot == other.stateRoot;
  }

  void toVbkEncoding(WriteStream& w) const;

  size_t estimateSize() const;

  uint256 getTopLevelMerkleRoot() const;
};

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                ContextInfoContainer& ctx,
                                ValidationState& state);

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AuthenticatedContextInfoContainer& ctx,
                                ValidationState& state);

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const ContextInfoContainer& a) {
  auto o = json::makeEmptyObject<JsonValue>();
  json::putIntKV(o, "height", a.height);
  json::putStringKV(
      o, "firstPreviousKeystone", HexStr(a.keystones.firstPreviousKeystone));
  json::putStringKV(
      o, "secondPreviousKeystone", HexStr(a.keystones.secondPreviousKeystone));
  return o;
}

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const AuthenticatedContextInfoContainer& a) {
  auto o = json::makeEmptyObject<JsonValue>();

  WriteStream w;
  a.toVbkEncoding(w);

  json::putStringKV(o, "serialized", HexStr(w.data()));
  json::putStringKV(o, "stateRoot", HexStr(a.stateRoot));
  json::putKV(o, "context", ToJSON<JsonValue>(a.ctx));
  return o;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CONTEXT_INFO_CONTAINER_HPP

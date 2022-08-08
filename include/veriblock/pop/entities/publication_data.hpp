// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/strutil.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include "context_info_container.hpp"
#include "veriblock/pop/json.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
struct ReadStream;

/**
 * @struct PublicationData
 *
 * Publication data about ALT block inside VBK blockchain.
 *
 */
struct PublicationData {
  //! altchain network POP ID
  int64_t identifier{};
  //! altchain block header
  std::vector<uint8_t> header{};
  //! bitcoin script or POP payout address
  std::vector<uint8_t> payoutInfo{};
  //! serialized AuthenticatedContextInfoContainer
  std::vector<uint8_t> contextInfo{};

  std::string toPrettyString() const;

  /**
   * Convert PublicationData to data stream using PublicationData byte
   * format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;
};

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const PublicationData& p) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "identifier", p.identifier);
  json::putStringKV(obj, "header", HexStr(p.header));
  json::putStringKV(obj, "payoutInfo", HexStr(p.payoutInfo));
  json::putStringKV(obj, "contextInfo", HexStr(p.contextInfo));
  return obj;
}

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PublicationData& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

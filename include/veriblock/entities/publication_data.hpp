// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

#include <cstdint>
#include <sstream>
#include <vector>

#include "veriblock/fmt.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/strutil.hpp"

namespace altintegration {

/**
 * @struct PublicationData
 *
 * Publication data about ALT block inside VBK blockchain.
 *
 * @ingroup entities
 */
struct PublicationData {
  //! altchain network POP ID
  int64_t identifier{};
  //! altchain block header
  std::vector<uint8_t> header{};
  //! bitcoin script or POP payout address
  std::vector<uint8_t> payoutInfo{};
  //! TBD
  std::vector<uint8_t> contextInfo{};

  std::string toPrettyString() const {
    return fmt::sprintf("PublicationData{id=%lld, header=%s, payoutInfo=%s}",
                        identifier,
                        HexStr(header),
                        HexStr(payoutInfo));
  }

  /**
   * Read data from the vector of bytes and convert it to
   * PublicationData
   * @param stream data stream to read from
   * @return PublicationData
   */
  static PublicationData fromRaw(const std::vector<uint8_t>& bytes);

  /**
   * Read data from the stream and convert it to PublicationData
   * @param stream data stream to read from
   * @return PublicationData
   */
  static PublicationData fromRaw(ReadStream& stream);

  static PublicationData generatePublicationData();

  /**
   * Convert PublicationData to data stream using PublicationData byte
   * format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;
};

//! @private
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
bool Deserialize(ReadStream& stream,
                 PublicationData& out,
                 ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_PUBLICATION_DATA_HPP_

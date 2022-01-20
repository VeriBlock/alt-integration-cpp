// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MEMPOOL_RESULT_HPP
#define VERIBLOCK_POP_CPP_MEMPOOL_RESULT_HPP

#include "entities/atv.hpp"
#include "entities/vbkblock.hpp"
#include "entities/vtb.hpp"
#include "validation_state.hpp"

namespace altintegration {

/**
 * An entity which is returned from MemPool on call to submitAll.
 */
struct MempoolResult {
  std::vector<std::pair<VbkBlock::id_t, ValidationState>> context;
  std::vector<std::pair<VTB::id_t, ValidationState>> vtbs;
  std::vector<std::pair<ATV::id_t, ValidationState>> atvs;
};

//! @private
namespace detail {

//! @overload
template <typename JsonValue, typename ID>
JsonValue putArrayOfPairs(
    JsonValue& obj,
    std::string key,
    const std::vector<std::pair<ID, ValidationState>>& pairs) {
  auto arr = json::makeEmptyArray<JsonValue>();
  for (auto& v : pairs) {
    auto o = json::makeEmptyObject<JsonValue>();
    json::putStringKV(o, "id", HexStr(v.first));
    auto state = ToJSON<JsonValue>(v.second);
    json::putKV(o, "validity", state);
    json::arrayPushBack(arr, o);
  }

  json::putKV(obj, key, arr);
  return obj;
}

}  // namespace detail

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const MempoolResult& r) {
  auto obj = json::makeEmptyObject<JsonValue>();
  detail::putArrayOfPairs(obj, "vbkblocks", r.context);
  detail::putArrayOfPairs(obj, "vtbs", r.vtbs);
  detail::putArrayOfPairs(obj, "atvs", r.atvs);
  return obj;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_MEMPOOL_RESULT_HPP

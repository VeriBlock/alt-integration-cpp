// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_TEST_CASE_ENTITY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_TEST_CASE_ENTITY_HPP_

#include <vector>

#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/config.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/serde.hpp"

namespace altintegration {

struct TestCase {
  using alt_block_with_payloads_t = std::pair<AltBlock, PopData>;

  std::vector<alt_block_with_payloads_t> alt_tree;
  Config config;

  static TestCase fromRaw(ReadStream& stream);

  static TestCase fromRaw(const std::vector<uint8_t>& bytes);

  std::vector<uint8_t> toRaw() const;

  void toRaw(WriteStream& stream) const;
};

}  // namespace altintegration

#endif

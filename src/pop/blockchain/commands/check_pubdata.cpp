// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/blockchain/commands/check_pubdata.hpp"

#include <cstdint>
#include <utility>
#include <vector>

#include "veriblock/pop/blockchain/alt_block_tree.hpp"
#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/entities/altblock.hpp"
#include "veriblock/pop/entities/context_info_container.hpp"
#include "veriblock/pop/entities/publication_data.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/strutil.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {

bool CheckPublicationData::Execute(ValidationState& state) noexcept {
  AuthenticatedContextInfoContainer c;
  if (!DeserializeFromVbkEncoding(pub.contextInfo, c, state)) {
    return state.Invalid("bad-sf-contextinfo");
  }

  auto& params = tree->getParams();
  const auto* endorsed = tree->getBlockIndex(endorsed_hash);
  if (endorsed == nullptr) {
    return state.Invalid("bad-sf-endorsed",
                         format("Can not find endorsed header (hash={})",
                                HexStr(endorsed_hash)));
  }

  auto ctx = ContextInfoContainer::createFromPrevious(endorsed->pprev, params);
  if (c.ctx != ctx) {
    return state.Invalid("bad-sf-context",
                         format("Expected context={}, got={}",
                                ctx.toPrettyString(),
                                c.ctx.toPrettyString()));
  }

  return true;
}

CheckPublicationData::CheckPublicationData(PublicationData pub,
                                           AltBlockTree& tree,
                                           AltBlock::hash_t endorsed_hash)
    : pub(std::move(pub)),
      endorsed_hash(std::move(endorsed_hash)),
      tree(&tree) {}
}  // namespace altintegration

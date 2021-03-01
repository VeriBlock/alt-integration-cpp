// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FACADE_ALT_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_FACADE_ALT_BLOCK_INDEX_HPP

#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>

#include "block_index_facade.hpp"

namespace altintegration {

struct AltBlockIndexFacade final : public BlockIndexFacade<AltBlock> {
  using base = BlockIndexFacade<AltBlock>;
  using impl = BlockIndex<AltBlock>;
  using hash_t = typename impl::hash_t;
  using index_t = AltBlockIndexFacade;

  AltBlockIndexFacade(const impl& ptr);

  std::vector<const AltEndorsement*> getContainingEndorsements() const;

  std::vector<const AltEndorsement*> getEndorsedBy() const noexcept;

  const std::vector<ATV::id_t>& getContainingATVs() const noexcept;

  const std::vector<VTB::id_t>& getContainingVTBs() const noexcept;

  const std::vector<VbkBlock::id_t>& getContainingVbkBlocks() const noexcept;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FACADE_ALT_BLOCK_INDEX_HPP

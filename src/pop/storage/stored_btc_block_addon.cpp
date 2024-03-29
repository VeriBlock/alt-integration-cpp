// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/storage/stored_btc_block_addon.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/pop/algorithm.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/entities/endorsement.hpp"
#include "veriblock/pop/entities/endorsements.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

namespace altintegration {

StoredBtcBlockAddon::StoredBtcBlockAddon(const addon_t& other) {
  blockOfProofEndorsementIds =
      map_get_id_from_pointers<VbkEndorsement::id_t, const VbkEndorsement>(
          other.getBlockOfProofEndorsement());
  refs = other.getRefs();
}

void StoredBtcBlockAddon::toVbkEncoding(WriteStream& w) const {
  writeContainer(w, blockOfProofEndorsementIds, writeSingleByteLenValue);
  writeContainer(w, refs, [&](WriteStream& /*ignore*/, ref_height_t value) {
    w.writeBE<ref_height_t>(value);
  });
}

void StoredBtcBlockAddon::toInmem(StoredBtcBlockAddon::addon_t& to) const {
  to.clearRefs();
  for (const auto& r : refs) {
    to.addRef(r);
  }
}

std::string StoredBtcBlockAddon::toPrettyString() const {
  return altintegration::format(
      "refs={}[{}]", refs.size(), fmt::join(refs, ","));
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredBtcBlockAddon& out,
                                ValidationState& state) {
  if (!readArrayOf<VbkEndorsement::id_t>(stream,
                                         out.blockOfProofEndorsementIds,
                                         state,
                                         0,
                                         MAX_POPDATA_VBK,
                                         [](ReadStream& stream,
                                            VbkEndorsement::id_t& o,
                                            ValidationState& state) -> bool {
                                           return readSingleByteLenValue(
                                               stream,
                                               o,
                                               state,
                                               VbkEndorsement::id_t::size(),
                                               VbkEndorsement::id_t::size());
                                         })) {
    return state.Invalid("stored-btc-block-addon-bad-block-of-proof-hash");
  }

  if (!readArrayOf<StoredBtcBlockAddon::ref_height_t>(
          stream,
          out.refs,
          state,
          0,
          MAX_BTCADDON_REFS,
          [](ReadStream& stream, int32_t& out, ValidationState& state) -> bool {
            return stream.readBE<StoredBtcBlockAddon::ref_height_t>(out, state);
          })) {
    return state.Invalid("bad-refs");
  }

  return true;
}

}  // namespace altintegration

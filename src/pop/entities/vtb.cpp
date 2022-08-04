// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/fmt.hpp>
#include <algorithm>
#include <cstdint>

#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/entities/btctx.hpp"
#include "veriblock/pop/entities/vbk_merkle_path.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"
#include "veriblock/pop/entities/vbkpoptx.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/read_stream.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

using namespace altintegration;

// NOLINTNEXTLINE(cert-err58-cpp)
const std::string VTB::_name = "VTB";

void VTB::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint32_t>(version);
  if (version == 1) {
    transaction.toVbkEncoding(stream);
    merklePath.toVbkEncoding(stream);
    containingBlock.toVbkEncoding(stream);
  } else {
    VBK_ASSERT_MSG(
        false, "VTB serialization version=%d is not implemented", version);
  }
}

std::vector<uint8_t> VTB::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

size_t VTB::estimateSize() const {
  size_t size = 0;
  size += sizeof(version);
  if (version == 1) {
    size += transaction.estimateSize();
    size += merklePath.estimateSize();
    size += containingBlock.estimateSize();
  } else {
    VBK_ASSERT_MSG(
        false, "VTB estimate size version=%d is not implemented", version);
  }
  return size;
}

VTB::id_t VTB::getId() const {
  auto btcTx = transaction.bitcoinTransaction.getHash();
  auto blockOfProof = transaction.blockOfProof.getHash();
  auto containingVbkBlock = uint256(containingBlock.getHash());
  auto temp = sha256(blockOfProof, containingVbkBlock);
  return sha256(btcTx, temp);
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                VTB& vtb,
                                                ValidationState& state) {
  if (!stream.readBE<uint32_t>(vtb.version, state)) {
    return state.Invalid("vtb-version");
  }
  if (vtb.version != 1) {
    return state.Invalid("vtb-bad-version");
  }

  if (!DeserializeFromVbkEncoding(stream, vtb.transaction, state)) {
    return state.Invalid("vtb-transaction");
  }
  if (!DeserializeFromVbkEncoding(stream, vtb.merklePath, state)) {
    return state.Invalid("vtb-merkle-path");
  }
  if (!DeserializeFromVbkEncoding(stream, vtb.containingBlock, state)) {
    return state.Invalid("vtb-containing-block");
  }
  return true;
}

std::string VTB::toShortPrettyString() const {
  return format("VTB(id={}, containing={}:{})",
                getId().toHex(),
                containingBlock.getHeight(),
                containingBlock.getHash().toHex());
}

std::string VTB::toPrettyString() const {
  return format("VTB(version={},containingTx={}({}), containingBlock={}({}))",
                version,
                transaction.getHash().toHex(),
                transaction.toPrettyString(),
                containingBlock.getHash().toHex(),
                containingBlock.toPrettyString());
}

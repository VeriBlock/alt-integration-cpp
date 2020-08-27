// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/vtb.hpp"

#include "veriblock/hashutil.hpp"

using namespace altintegration;

const std::string VTB::_name = "VTB";

VTB VTB::fromVbkEncoding(ReadStream& stream) {
  VTB vtb{};
  vtb.transaction = VbkPopTx::fromVbkEncoding(stream);
  vtb.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  vtb.containingBlock = VbkBlock::fromVbkEncoding(stream);
  vtb.context = readArrayOf<VbkBlock>(
      stream,
      0,
      MAX_CONTEXT_COUNT,
      (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding);

  return vtb;
}

VTB VTB::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

VTB VTB::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void VTB::toVbkEncoding(WriteStream& stream) const {
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  writeSingleBEValue(stream, context.size());
  for (const auto& block : context) {
    block.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> VTB::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

VTB::id_t VTB::getId() const {
  auto btcTx = transaction.bitcoinTransaction.getHash();
  auto blockOfProof = transaction.blockOfProof.getHash();
  auto containingVbkBlock = uint256(containingBlock.getHash());
  auto temp = sha256(blockOfProof, containingVbkBlock);
  return sha256(btcTx, temp);
}

VTB VTB::fromHex(const std::string& hex) {
  auto data = ParseHex(hex);
  return fromVbkEncoding(data);
}

bool altintegration::Deserialize(ReadStream& stream,
  VTB& out,
  ValidationState& state) {
  VTB vtb{};
  typedef bool (*vbkde)(ReadStream&, VbkBlock&, ValidationState&);

  if (!Deserialize(stream, vtb.transaction, state)) {
    return state.Invalid("vtb-transaction");
  }
  if (!Deserialize(stream, vtb.merklePath, state)) {
    return state.Invalid("vtb-merkle-path");
  }
  if (!Deserialize(stream, vtb.containingBlock, state)) {
    return state.Invalid("vtb-containing-block");
  }

  if (!readArrayOf<VbkBlock>(stream,
                             vtb.context,
                             state,
                             0,
                             MAX_CONTEXT_COUNT,
                             static_cast<vbkde>(Deserialize))) {
    return state.Invalid("vtb-context");
  }

  out = vtb;
  return true;
}

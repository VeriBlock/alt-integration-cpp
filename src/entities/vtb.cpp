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
  vtb.version = stream.readBE<uint32_t>();
  if (vtb.version == 1) {
    vtb.transaction = VbkPopTx::fromVbkEncoding(stream);
    vtb.merklePath = VbkMerklePath::fromVbkEncoding(stream);
    vtb.containingBlock = VbkBlock::fromVbkEncoding(stream);
  } else {
    throw std::domain_error(
        fmt::format("VTB version={} is not implemented", vtb.version));
  }

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
  if (!stream.readBE<uint32_t>(vtb.version, state)) {
    return state.Invalid("vtb-version");
  }
  if (vtb.version != 1) {
    return state.Invalid("vtb-bad-version");
  }

  if (!Deserialize(stream, vtb.transaction, state)) {
    return state.Invalid("vtb-transaction");
  }
  if (!Deserialize(stream, vtb.merklePath, state)) {
    return state.Invalid("vtb-merkle-path");
  }
  if (!Deserialize(stream, vtb.containingBlock, state)) {
    return state.Invalid("vtb-containing-block");
  }

  out = vtb;
  return true;
}

std::string VTB::toPrettyString() const {
  return fmt::sprintf("VTB(version=%d,containingTx=%s(%s), containingBlock=%s)",
                      version,
                      transaction.getHash().toHex(),
                      transaction.toPrettyString(),
                      containingBlock.getHash().toHex());
}

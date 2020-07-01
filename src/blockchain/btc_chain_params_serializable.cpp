// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/btc_chain_params_serializable.hpp>

namespace altintegration {

std::vector<uint8_t> BtcChainParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void BtcChainParams::toRaw(WriteStream& stream) const {
  writeVarLenValue(stream,
                   Slice<const uint8_t>(this->getPowLimit().data(),
                                        this->getPowLimit().size()));
  stream.writeBE<uint32_t>(this->getPowTargetTimespan());
  stream.writeBE<uint32_t>(this->getPowTargetSpacing());
  stream.writeBE<uint8_t>((uint8_t)this->getAllowMinDifficultyBlocks());
  stream.writeBE<uint8_t>((uint8_t)this->getPowNoRetargeting());
  stream.writeBE<uint8_t>((uint8_t)this->EnableTimeAdjustment());
  this->getGenesisBlock().toVbkEncoding(stream);
  stream.writeBE<uint32_t>(this->numBlocksForBootstrap());

  writeSingleBEValue(stream, this->networkName().size());
  for (const auto& el : this->networkName()) {
    stream.writeBE<char>(el);
  }
}

BtcChainParamsSerializable BtcChainParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

BtcChainParamsSerializable BtcChainParamsSerializable::fromRaw(
    ReadStream& stream) {
  BtcChainParamsSerializable param;
  param.powLimit_ = readVarLenValue(stream, 0, MAX_CONTEXT_COUNT);
  param.powTargetTimespan_ = stream.readBE<uint32_t>();
  param.powTargetSpacing_ = stream.readBE<uint32_t>();
  param.allowMinDifficultyBlocks_ = (bool)stream.readBE<uint8_t>();
  param.powNoRetargeting_ = (bool)stream.readBE<uint8_t>();
  param.EnableTimeAdjustment_ = (bool)stream.readBE<uint8_t>();
  param.genesisBlock_ = BtcBlock::fromVbkEncoding(stream);
  param.numBlocksForBootstrap_ = stream.readBE<uint32_t>();

  param.networkName_ = readString(stream);

  return param;
}

}  // namespace altintegration

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/vbk_chain_params_serializable.hpp>

namespace altintegration {

std::vector<uint8_t> VbkNetworkType::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void VbkNetworkType::toRaw(WriteStream& stream) const {
  stream.writeBE<uint8_t>((uint8_t)this->hasValue);
  if (this->hasValue) {
    stream.writeBE<uint8_t>(this->value);
  }
}

VbkNetworkType VbkNetworkType::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkNetworkType VbkNetworkType::fromRaw(ReadStream& stream) {
  bool hasValue = (bool)stream.readBE<uint8_t>();
  if (hasValue) {
    return VbkNetworkType(hasValue, stream.readBE<uint8_t>());
  }
  return VbkNetworkType(hasValue, 0);
}

std::vector<uint8_t> VbkChainParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void VbkChainParams::toRaw(WriteStream& stream) const {
  writeSingleBEValue(stream, this->networkName().size());
  for (const auto& el : this->networkName()) {
    stream.writeBE<char>(el);
  }
  stream.writeBE<uint8_t>((uint8_t)this->EnableTimeAdjustment());

  writeVarLenValue(stream,
                   Slice<const uint8_t>(this->getMinimumDifficulty().data(),
                                        this->getMinimumDifficulty().size()));

  this->getTransactionMagicByte().toRaw(stream);
  stream.writeBE<uint8_t>((uint8_t)this->getPowNoRetargeting());
  this->getGenesisBlock().toVbkEncoding(stream);
  stream.writeBE<uint32_t>(this->getRetargetPeriod());
  stream.writeBE<uint32_t>(this->getTargetBlockTime());
  stream.writeBE<uint32_t>(this->numBlocksForBootstrap());
  stream.writeBE<uint32_t>(this->getKeystoneInterval());
  stream.writeBE<uint32_t>(this->getFinalityDelay());

  writeSingleBEValue(stream, this->getForkResolutionLookUpTable().size());
  for (const auto& el : this->getForkResolutionLookUpTable()) {
    stream.writeBE<uint32_t>(el);
  }

  stream.writeBE<int32_t>(this->getEndorsementSettlementInterval());
}

VbkChainParamsSerializable VbkChainParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

VbkChainParamsSerializable VbkChainParamsSerializable::fromRaw(
    ReadStream& stream) {
  VbkChainParamsSerializable param;
  param.networkName_ = readString(stream);
  param.EnableTimeAdjustment_ =  (bool)stream.readBE<uint8_t>();
  param.minimumDifficulty_ = readVarLenValue(stream, 0, MAX_CONTEXT_COUNT);
  param.transactionMagicByte_ = VbkNetworkType::fromRaw(stream);
  param.powNoRetargeting_ = (bool)stream.readBE<uint8_t>();
  param.genesisBlock_ = VbkBlock::fromVbkEncoding(stream);
  param.retargetPeriod_ = stream.readBE<uint32_t>();
  param.targetBlockTime_ = stream.readBE<uint32_t>();
  param.numBlocksForBootstrap_ = stream.readBE<uint32_t>();
  param.keystoneInterval_ = stream.readBE<uint32_t>();
  param.finalityDelay_ = stream.readBE<uint32_t>();

  param.forkResolutionLookUpTable_ = readArrayOf<uint32_t>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) -> uint32_t {
        return stream.readBE<uint32_t>();
      });

  param.endorsementSettlementInterval_ = stream.readBE<int32_t>();

  return param;
}

}  // namespace altintegration

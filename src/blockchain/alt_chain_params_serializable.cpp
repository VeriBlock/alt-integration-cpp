#include <veriblock/blockchain/alt_chain_params_serializable.hpp>

namespace altintegration {


PopRewardsParamsSerializable PopRewardsParamsSerializable::fromRaw(
    ReadStream& stream) {

}

PopRewardsParamsSerializable PopRewardsParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

std::vector<uint8_t> AltChainParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void AltChainParams::toRaw(WriteStream& stream) const {
  stream.writeBE<uint32_t>(this->getKeystoneInterval());
  stream.writeBE<uint32_t>(this->getFinalityDelay());

  writeSingleBEValue(stream, this->getForkResolutionLookUpTable().size());
  for (const auto& val : this->getForkResolutionLookUpTable()) {
    stream.writeBE<uint32_t>(val);
  }

  stream.writeBE<int32_t>(this->getEndorsementSettlementInterval());
  stream.writeBE<size_t>(this->getMaxPopDataSize());
  stream.writeBE<uint32_t>(this->getIdentifier());

  this->getBootstrapBlock().toVbkEncoding(stream);
  this->getRewardParams().toRaw(stream);
}

AltChainParamsSerializable AltChainParamsSerializable::fromRaw(
    ReadStream& stream) {
  AltChainParamsSerializable param;
  param.keystoneInterval_ = stream.readBE<uint32_t>();
  param.finalityDelay_ = stream.readBE<uint32_t>();

  param.forkResolutionLookUpTable_ = readArrayOf<uint32_t>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) -> uint32_t {
        return stream.readBE<uint32_t>();
      });

  param.endorsementSettlementInterval_ = stream.readBE<int32_t>();
  param.maxPopDataSize = stream.readBE<size_t>();
  param.indentifier_ = stream.readBE<uint32_t>();

  param.bootstrapBlock_ = AltBlock::fromVbkEncoding(stream);
  param.popRewardsParams = std::make_shared<PopRewardsParamsSerializable>(
      PopRewardsParamsSerializable::fromRaw(stream));

  return param;
}

AltChainParamsSerializable AltChainParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

}  // namespace altintegration

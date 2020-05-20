#include <veriblock/blockchain/alt_chain_params_serializable.hpp>

namespace altintegration {

void PopRewardsCurveParams::toRaw(WriteStream& stream) const {
  writeDouble(stream, this->startOfSlope());
  writeDouble(stream, this->slopeNormal());
  writeDouble(stream, this->slopeKeystone());
}

std::vector<uint8_t> PopRewardsCurveParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

PopRewardsCurveParamsSerializable PopRewardsCurveParamsSerializable::fromRaw(
    ReadStream& stream) {
  PopRewardsCurveParamsSerializable param;

  param.startOfSlope_ = readDouble(stream);
  param.slopeNormal_ = readDouble(stream);
  param.slopeKeystone_ = readDouble(stream);

  return param;
}

PopRewardsCurveParamsSerializable PopRewardsCurveParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

void PopRewardsParams::toRaw(WriteStream& stream) const {
  stream.writeBE<uint32_t>(this->keystoneRound());
  stream.writeBE<uint32_t>(this->payoutRounds());
  stream.writeBE<uint32_t>(this->flatScoreRound());
  stream.writeBE<uint8_t>(this->flatScoreRoundUse());

  writeSingleBEValue(stream, this->roundRatios().size());
  for (const auto& el : this->roundRatios()) {
    writeDouble(stream, el);
  }

  writeDouble(stream, this->maxScoreThresholdNormal());
  writeDouble(stream, this->maxScoreThresholdKeystone());

  stream.writeBE<uint32_t>(this->difficultyAveragingInterval());
  writeSingleBEValue(stream, this->relativeScoreLookupTable().size());
  for (const auto& el : this->relativeScoreLookupTable()) {
    writeDouble(stream, el);
  }

  this->getCurveParams().toRaw(stream);
}

std::vector<uint8_t> PopRewardsParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

PopRewardsParamsSerializable PopRewardsParamsSerializable::fromRaw(
    ReadStream& stream) {
  PopRewardsParamsSerializable param;

  param.keystoneRound_ = stream.readBE<uint32_t>();
  param.payoutRounds_ = stream.readBE<uint32_t>();
  param.flatScoreRound_ = stream.readBE<uint32_t>();
  param.flatScoreRoundUse_ = (bool)stream.readBE<uint8_t>();

  param.roundRatios_ =
      readArrayOf<double>(stream, 0, MAX_CONTEXT_COUNT, readDouble);

  param.maxScoreThresholdNormal_ = readDouble(stream);
  param.maxScoreThresholdKeystone_ = readDouble(stream);
  param.difficultyAveragingInterval_ = stream.readBE<uint32_t>();

  param.lookupTable_ =
      readArrayOf<double>(stream, 0, MAX_CONTEXT_COUNT, readDouble);

  param.curveParams = std::make_shared<PopRewardsCurveParamsSerializable>(
      PopRewardsCurveParamsSerializable::fromRaw(stream));

  return param;
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
  stream.writeBE<uint32_t>(this->getMaxPopDataPerBlock());
  stream.writeBE<uint32_t>(this->getMaxPopDataWeight());
  stream.writeBE<uint32_t>(this->getSuperMaxPopDataWeight());
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
  param.maxPopDataPerBlock_ = stream.readBE<uint32_t>();
  param.maxPopDataWeight_ = stream.readBE<uint32_t>();
  param.superMaxPopDataWeight_ = stream.readBE<uint32_t>();
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

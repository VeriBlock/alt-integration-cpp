#include <veriblock/blockchain/alt_chain_params_serializable.hpp>

namespace altintegration {

void PopRewardsCurveParams::toRaw(WriteStream& stream) const {
  std::string temp;

  temp = std::to_string(this->startOfSlope());
  writeSingleBEValue(stream, temp.size());
  for (const char& el : temp) {
    stream.writeBE<char>(el);
  }

  temp = std::to_string(this->slopeNormal());
  writeSingleBEValue(stream, temp.size());
  for (const char& el : temp) {
    stream.writeBE<char>(el);
  }

  temp = std::to_string(this->slopeKeystone());
  writeSingleBEValue(stream, temp.size());
  for (const char& el : temp) {
    stream.writeBE<char>(el);
  }
}

std::vector<uint8_t> PopRewardsCurveParams::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

PopRewardsCurveParamsSerializable PopRewardsCurveParamsSerializable::fromRaw(
    ReadStream& stream) {
  PopRewardsCurveParamsSerializable param;

  param.startOfSlope_ = std::atof(readString(stream).data());
  param.slopeNormal_ = std::atof(readString(stream).data());
  param.slopeKeystone_ = std::atof(readString(stream).data());

  return param;
}

PopRewardsCurveParamsSerializable PopRewardsCurveParamsSerializable::fromRaw(
    const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

}  // namespace altintegration

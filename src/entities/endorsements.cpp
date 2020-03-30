#include <veriblock/entities/endorsements.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

template <>
BtcEndorsement BtcEndorsement::fromVbkEncoding(ReadStream& stream) {
  BtcEndorsement endorsement;
  endorsement.id = stream.readSlice(sizeof(id_t));
  endorsement.endorsedHash = stream.readSlice(sizeof(endorsed_hash_t));
  endorsement.containingHash = stream.readSlice(sizeof(endorsed_hash_t));
  endorsement.blockOfProof = stream.readSlice(sizeof(containing_hash_t));
  endorsement.payoutInfo = {};

  return endorsement;
}

template <>
BtcEndorsement BtcEndorsement::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

template <>
void BtcEndorsement::toVbkEncoding(WriteStream& stream) const {
  stream.write(id);
  stream.write(endorsedHash);
  stream.write(containingHash);
  stream.write(blockOfProof);
}

template <>
std::vector<uint8_t> BtcEndorsement::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

template <>
VbkEndorsement VbkEndorsement::fromVbkEncoding(ReadStream& stream) {
  VbkEndorsement endorsement;
  endorsement.id = stream.readSlice(sizeof(id_t));
  endorsement.endorsedHash = readVarLenValue(stream).asVector();
  endorsement.containingHash = readVarLenValue(stream).asVector();
  /*uint32_t hash_size = stream.readBE<uint32_t>();
  endorsement.endorsedHash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    endorsement.endorsedHash[i] = stream.readBE<uint8_t>();
  }*/
  /*hash_size = stream.readBE<uint32_t>();
  endorsement.containingHash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    endorsement.containingHash[i] = stream.readBE<uint8_t>();
  }*/
  endorsement.blockOfProof = stream.readSlice(sizeof(containing_hash_t));

  /*uint32_t payout_size = stream.readBE<uint32_t>();
  endorsement.payoutInfo.resize(payout_size);
  for (uint32_t i = 0; i < payout_size; ++i) {
    endorsement.payoutInfo[i] = stream.readBE<uint8_t>();
  }*/
  endorsement.payoutInfo = readVarLenValue(stream).asVector();

  return endorsement;
}

template <>
VbkEndorsement VbkEndorsement::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

template <>
void VbkEndorsement::toVbkEncoding(WriteStream& stream) const {
  stream.write(id);
  writeVarLenValue(stream, endorsedHash);
  writeVarLenValue(stream, containingHash);
  /*stream.writeBE<uint32_t>((uint32_t)endorsedHash.size());
  for (unsigned char i : endorsedHash) {
    stream.writeBE<uint8_t>(i);
  }*/
  /*stream.writeBE<uint32_t>((uint32_t)containingHash.size());
  for (unsigned char i : containingHash) {
    stream.writeBE<uint8_t>(i);
  }*/
  stream.write(blockOfProof);

  /*stream.writeBE<uint32_t>((uint32_t)payoutInfo.size());
  for (size_t i = 0; i < payoutInfo.size(); ++i) {
    stream.writeBE<uint8_t>(payoutInfo[i]);
  }*/
  writeVarLenValue(stream, payoutInfo);
}

template <>
std::vector<uint8_t> VbkEndorsement::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

template <>
BtcEndorsement BtcEndorsement::fromContainer(const VTB& c) {
  BtcEndorsement e;
  e.id = BtcEndorsement::getId(c);
  e.blockOfProof = c.transaction.blockOfProof.getHash();
  e.containingHash = c.containingBlock.getHash();
  e.endorsedHash = c.transaction.publishedBlock.getHash();
  e.payoutInfo = {};
  return e;
}

template <>
VbkEndorsement VbkEndorsement::fromContainer(const AltProof& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.containingHash = c.containing.hash;
  e.payoutInfo = c.atv.transaction.publicationData.payoutInfo;
  return e;
}

template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c) {
  WriteStream stream;
  c.transaction.toRaw(stream);
  c.containingBlock.toRaw(stream);
  return sha256(stream.data());
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltProof& c) {
  WriteStream stream;
  c.atv.transaction.toRaw(stream);
  c.containing.toVbkEncoding(stream);
  return sha256(stream.data());
}

}  // namespace altintegration

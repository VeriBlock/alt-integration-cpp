#include <veriblock/entities/endorsement.hpp>

namespace AltIntegrationLib {

template <>
BtcEndorsement BtcEndorsement::fromVbkEncoding(ReadStream& stream) {
  BtcEndorsement endorsement;
  endorsement.id = stream.readSlice(sizeof(id_t));
  endorsement.endorsedHash = stream.readSlice(sizeof(endorsed_hash_t));
  endorsement.containingHash = stream.readSlice(sizeof(endorsed_hash_t));
  endorsement.blockOfProof = stream.readSlice(sizeof(containing_hash_t));

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
  uint32_t hash_size = stream.readBE<uint32_t>();
  endorsement.endorsedHash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    endorsement.endorsedHash[i] = stream.readBE<uint8_t>();
  }
  hash_size = stream.readBE<uint32_t>();
  endorsement.containingHash.resize(hash_size);
  for (uint32_t i = 0; i < hash_size; ++i) {
    endorsement.containingHash[i] = stream.readBE<uint8_t>();
  }
  endorsement.blockOfProof = stream.readSlice(sizeof(containing_hash_t));

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
  stream.writeBE<uint32_t>((uint32_t)endorsedHash.size());
  for (size_t i = 0; i < endorsedHash.size(); ++i) {
    stream.writeBE<uint8_t>(endorsedHash[i]);
  }
  stream.writeBE<uint32_t>((uint32_t)containingHash.size());
  for (size_t i = 0; i < containingHash.size(); ++i) {
    stream.writeBE<uint8_t>(containingHash[i]);
  }
  stream.write(blockOfProof);
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
  return e;
}

template <>
VbkEndorsement VbkEndorsement::fromContainer(const AltProof& c) {
  VbkEndorsement e;
  e.id = VbkEndorsement::getId(c);
  e.blockOfProof = c.atv.containingBlock.getHash();
  e.endorsedHash = c.endorsed.hash;
  e.containingHash = c.containing.hash;
  return e;
}

template <>
BtcEndorsement::id_t BtcEndorsement::getId(const VTB& c) {
  return c.transaction.getHash();
}

template <>
VbkEndorsement::id_t VbkEndorsement::getId(const AltProof& c) {
  return c.atv.transaction.getHash();
}

}  // namespace AltIntegrationLib

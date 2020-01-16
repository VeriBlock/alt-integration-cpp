#ifndef ALT_INTEGRATION_ADDRESS_HPP
#define ALT_INTEGRATION_ADDRESS_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "veriblock/base58.hpp"
#include "veriblock/base59.hpp"
#include "veriblock/serde.hpp"

namespace VeriBlock {

struct Address {
  // TODO: add autodetection of address type
  Address(AddressType type, std::string addr)
      : m_Type(type), m_Address(std::move(addr)) {}

  bool operator==(const Address& other) const noexcept {
    return m_Address == other.m_Address;
  }

  bool operator==(const std::string& other) const noexcept {
    return m_Address == other;
  }

  const std::string& data() const noexcept { return m_Address; }

  void toVbkEncoding(WriteStream& stream) const {
    stream.writeBE<uint8_t>((uint8_t)m_Type);
    std::vector<uint8_t> decoded;
    switch (m_Type) {
      case AddressType::STANDARD:
        decoded = DecodeBase58(m_Address);
        break;
      case AddressType ::MULTISIG:
        decoded = DecodeBase59(m_Address);
        break;
      default:
        throw std::invalid_argument("unexpected address type to encode");
    }

    writeSingleByteLenValue(stream, decoded);
  }

  static Address fromVbkEncoding(ReadStream& stream) {
    auto addressType = (AddressType)stream.readLE<uint8_t>();
    auto addressBytes =
        readSingleByteLenValue(stream, 0, VeriBlock::ADDRESS_SIZE);

    std::string address;
    switch (addressType) {
      case AddressType::STANDARD:
        address = EncodeBase58(addressBytes);
        break;
      case AddressType::MULTISIG:
        address = EncodeBase59(addressBytes);
        break;
      default:
        throw std::invalid_argument(
            "invalid address type: neither standard, nor multisig");
    }

    return Address(addressType, address);
  }

 private:
  AddressType m_Type;
  std::string m_Address;
};

}  // namespace VeriBlock

#endif
#ifndef ALT_INTEGRATION_ADDRESS_HPP
#define ALT_INTEGRATION_ADDRESS_HPP

#include <cstdint>
#include <cassert>
#include <string>
#include <vector>

#include "veriblock/base58.hpp"
#include "veriblock/base59.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/checks.hpp"
#include "veriblock/hashutil.hpp"

namespace VeriBlock {

constexpr const auto STARTING_CHAR = 'V';
constexpr const auto MULTISIG_ENDING_CHAR = '0';

constexpr const auto MULTISIG_ADDRESS_M_VALUE = 1;
constexpr const auto MULTISIG_ADDRESS_N_VALUE = 2;
constexpr const auto MULTISIG_ADDRESS_MIN_N_VALUE = 2;
constexpr const auto MULTISIG_ADDRESS_MAX_N_VALUE = 58;
constexpr const auto MULTISIG_ADDRESS_MAX_M_VALUE = 58;
constexpr const auto MULTISIG_ADDRESS_DATA_START = 0;
constexpr const auto MULTISIG_ADDRESS_DATA_END = 24;
constexpr const auto MULTISIG_ADDRESS_CHECKSUM_END = 28;

// TODO: add autodetection of address type
struct Address {
  Address() = default;

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
    AddressType m_Type{};
    std::string m_Address{};
};

static std::string getDataPortionFromAddress(std::string address) {
  assert(address.length() == ADDRESS_SIZE);
  return address.substr(0, 24 + 1);
}

static bool isMultisig(std::string address) {
  assert(address.length() == ADDRESS_SIZE);
  return (address[ADDRESS_SIZE - 1] == MULTISIG_ENDING_CHAR);
}

static std::string getChecksumPortionFromAddress(std::string address,
                                                 bool multisig) {
  assert(address.length() == ADDRESS_SIZE);
  if (multisig) {
    return address.substr(25, 28 + 1);
  }
  return address.substr(25);
}

static bool isBase58String(std::string input) {
  try {
    auto decoded = DecodeBase58(input);
    (void) decoded;
    return true;
  } catch (std::invalid_argument e) {
    // do not throw - return status instead
  }
  return false;
}

static bool isBase59String(std::string input) {
  try {
    auto decoded = DecodeBase59(input);
    (void)decoded;
    return true;
  } catch (std::invalid_argument e) {
    // do not throw - return status instead
  }
  return false;
}

static std::string calculateChecksum(std::string data, bool multisig) {
  std::vector<uint8_t> dataBytes = toBytes(data);
  auto hash = sha256(dataBytes);
  std::string checksum = EncodeBase58(hash);
  if (multisig) {
    return checksum.substr(0, 3 + 1);
  }
  return checksum.substr(0, 4 + 1);
}

void isValidAddress(std::string input) {
  if (input.size() != ADDRESS_SIZE) {
    throw std::invalid_argument("isValidAddress(): invalid address length");
  }
  if (input[0] != STARTING_CHAR) {
    throw std::invalid_argument("isValidAddress(): not a valid VBK address");
  }

  std::string data = getDataPortionFromAddress(input);
  bool multisig = isMultisig(input);
  std::string checksum = getChecksumPortionFromAddress(input, multisig);

  if (multisig) {
    if (!isBase59String(input)) {
      throw std::invalid_argument("isValidAddress(): not a base59 string");
    }

    /* To make the addresses 'human-readable' we add 1 to the decoded value (1
     * in Base58 is 0, but we want an address with a '1' in the m slot to
     * represent m=1, for example). this allows addresses with m and n both <= 9
     * to be easily recognized. Additionally, an m or n value of 0 makes no
     * sense, so this allows multisig to range from 1 to 58, rather than what
     * would have otherwise been 0 to 57. */
    int m =
        DecodeBase58(std::string(1, input[MULTISIG_ADDRESS_M_VALUE]))[0] + 1;
    int n =
        DecodeBase58(std::string(1, input[MULTISIG_ADDRESS_N_VALUE]))[0] + 1;

    if (n < MULTISIG_ADDRESS_MIN_N_VALUE) {
      throw std::invalid_argument("isValidAddress(): not enough addresses to be multisig");
    }
    if (m > n) {
      throw std::invalid_argument(
          "isValidAddress(): address has more signatures than addresses");
    }
    if ((n > MULTISIG_ADDRESS_MAX_N_VALUE) || (m > MULTISIG_ADDRESS_MAX_M_VALUE)) {
      throw std::invalid_argument(
          "isValidAddress(): too many addresses/signatures");
    }

    if (!isBase58String(input.substr(0, ADDRESS_SIZE - 1))) {
      throw std::invalid_argument(
          "isValidAddress(): remainder is not a base58 string");
    }
  } else {
    if (!isBase58String(input)) {
      throw std::invalid_argument(
          "isValidAddress(): address is not a base58 string");
    }
  }

  auto expectedChecksum = calculateChecksum(data, multisig);
  if (expectedChecksum != checksum) {
    throw std::invalid_argument(
        "isValidAddress(): checksum does not match");
  }
}

Address fromPublicKey(Slice<const uint8_t> publicKey) {
  auto keyHash = sha256(publicKey);
  auto keyHashEncoded = EncodeBase58(keyHash);
  auto data = std::string{"V"} + keyHashEncoded.substr(0, 24);
  auto checksum = calculateChecksum(data, false);
  return Address(AddressType::STANDARD, data + checksum);
}

bool isDerivedFromPublicKey(Address address, Slice<const uint8_t> publicKey, bool multisig) {
  auto keyHash = sha256(publicKey);
  auto keyHashEncoded = EncodeBase58(keyHash);
  auto data = std::string{"V"} + keyHashEncoded.substr(0, 24);
  auto checksum = calculateChecksum(data, multisig);

  if (address.data != data) return false;
  if (calculateChecksum(address.data, multisig) != checksum) return false;
  return true;
}

}  // namespace VeriBlock

#endif
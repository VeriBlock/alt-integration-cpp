// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/address.hpp"

#include "veriblock/assert.hpp"
#include "veriblock/base58.hpp"
#include "veriblock/base59.hpp"
#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"

namespace altintegration {

constexpr const auto STARTING_CHAR = 'V';
constexpr const auto MULTISIG_ENDING_CHAR = '0';

constexpr const auto MULTISIG_ADDRESS_M_VALUE = 1;
constexpr const auto MULTISIG_ADDRESS_N_VALUE = 2;
constexpr const auto MULTISIG_ADDRESS_MIN_N_VALUE = 2;
constexpr const auto MULTISIG_ADDRESS_MAX_N_VALUE = 58;
constexpr const auto MULTISIG_ADDRESS_MAX_M_VALUE = 58;
constexpr const auto MULTISIG_ADDRESS_DATA_END = 24;
constexpr const auto MULTISIG_ADDRESS_CHECKSUM_END = 28;

static std::string getDataPortionFromAddress(const std::string& address) {
  VBK_ASSERT(address.length() == ADDRESS_SIZE);
  return address.substr(0, MULTISIG_ADDRESS_DATA_END + 1);
}

static bool isMultisig(const std::string& address) {
  VBK_ASSERT(address.length() == ADDRESS_SIZE);
  return (address[ADDRESS_SIZE - 1] == MULTISIG_ENDING_CHAR);
}

static std::string getChecksumPortionFromAddress(const std::string& address,
                                                 bool multisig) {
  VBK_ASSERT(address.length() == ADDRESS_SIZE);
  if (multisig) {
    return address.substr(
        MULTISIG_ADDRESS_DATA_END + 1,
        MULTISIG_ADDRESS_CHECKSUM_END - MULTISIG_ADDRESS_DATA_END);
  }
  return address.substr(MULTISIG_ADDRESS_DATA_END + 1);
}

static bool isBase58String(const std::string& input) {
  std::vector<uint8_t> out;
  return DecodeBase58(input, out);
}

static bool isBase59String(const std::string& input) {
  std::vector<uint8_t> out;
  return DecodeBase59(input, out);
}

static std::string calculateChecksum(const std::string& data, bool multisig) {
  auto hash = sha256(data);
  std::string checksum = EncodeBase58(hash);
  if (multisig) {
    return checksum.substr(0, 3 + 1);
  }
  return checksum.substr(0, 4 + 1);
}

static std::string addressChecksum(const Address& address) {
  return calculateChecksum(address.toString(),
                           address.getType() == AddressType::MULTISIG);
}

Address Address::fromPublicKey(Slice<const uint8_t> publicKey) {
  auto keyHash = sha256(publicKey);
  auto keyHashEncoded = EncodeBase58(keyHash);
  auto data = std::string{STARTING_CHAR} +
              keyHashEncoded.substr(0, MULTISIG_ADDRESS_DATA_END);
  auto checksum = calculateChecksum(data, false);
  return Address(AddressType::STANDARD, data + checksum);
}

bool Address::isDerivedFromPublicKey(Slice<const uint8_t> publicKey) const {
  auto expectedAddress = fromPublicKey(publicKey);
  if (*this != expectedAddress) {
    return false;
  }
  if (addressChecksum(*this) != addressChecksum(expectedAddress)) {
    return false;
  }
  return true;
}

bool Address::fromString(const std::string& input, ValidationState& state) {
  if (input.size() != ADDRESS_SIZE) {
    return state.Invalid(
        "addr-bad-length",
        fmt::format("Invalid address length. Expected={}, got={}.",
                    ADDRESS_SIZE,
                    input.size()));
  }

  if (input[0] != STARTING_CHAR) {
    return state.Invalid(
        "addr-bad-starting-char",
        fmt::format("Address should start with {}", STARTING_CHAR));
  }

  std::string data = getDataPortionFromAddress(input);
  bool multisig = isMultisig(input);
  std::string checksum = getChecksumPortionFromAddress(input, multisig);

  if (multisig) {
    if (!isBase59String(input)) {
      return state.Invalid("addr-bad-multisig-content", "Not a base59 string.");
    }

    /* To make the addresses 'human-readable' we add 1 to the decoded value (1
     * in Base58 is 0, but we want an address with a '1' in the m slot to
     * represent m=1, for example). this allows addresses with m and n both <=
     * 9 to be easily recognized. Additionally, an m or n value of 0 makes no
     * sense, so this allows multisig to range from 1 to 58, rather than what
     * would have otherwise been 0 to 57. */
    auto decodeNumber = [](const std::string& in, int& num) -> bool {
      std::vector<uint8_t> ret(in.size(), 0);
      if (!DecodeBase58(in, ret)) {
        return false;
      }
      num = ret[0] + 1;
      return true;
    };

    int n = 0, m = 0;
    if (!decodeNumber(std::string(1, input[MULTISIG_ADDRESS_M_VALUE]), m)) {
      return state.Invalid("addr-multisig-bad-m");
    }
    if (!decodeNumber(std::string(1, input[MULTISIG_ADDRESS_N_VALUE]), n)) {
      return state.Invalid("addr-multisig-bad-n");
    }

    if (n < MULTISIG_ADDRESS_MIN_N_VALUE) {
      return state.Invalid(
          "addr-multisig-bad-n-too-small",
          fmt::format("Expected N to be at least {}, but got {}",
                      MULTISIG_ADDRESS_MIN_N_VALUE,
                      n));
    }
    if (m > n) {
      return state.Invalid("addr-multisig-bad-mn",
                           "Address has more signatures than addresses");
    }
    if ((n > MULTISIG_ADDRESS_MAX_N_VALUE) ||
        (m > MULTISIG_ADDRESS_MAX_M_VALUE)) {
      return state.Invalid("addr-multisig-bad-mn-too-many",
                           "Too many addresses or signatures");
    }

    if (!isBase58String(input.substr(0, ADDRESS_SIZE - 1))) {
      return state.Invalid("addr-multisig-bad-remainder",
                           "Remainder is not a base58 string");
    }
  } else {
    if (!isBase58String(input)) {
      return state.Invalid("addr-bad", "Address is not a base58 string");
    }
  }

  auto expectedChecksum = calculateChecksum(data, multisig);
  if (expectedChecksum != checksum) {
    return state.Invalid(
        "addr-bad-checksum",
        fmt::format("Checksum does not match. Expected={}, got={}.",
                    expectedChecksum,
                    checksum));
  }

  m_Type = multisig ? AddressType::MULTISIG : AddressType::STANDARD;
  m_Address = input;

  return true;
}

std::string Address::toString() const noexcept { return m_Address; }

void Address::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint8_t>((uint8_t)getType());
  std::vector<uint8_t> decoded;
  switch (getType()) {
    case AddressType::STANDARD:
      decoded = AssertDecodeBase58(toString());
      break;
    case AddressType ::MULTISIG:
      decoded = AssertDecodeBase59(toString());
      break;
    default:
      // if we don't know address type, do not encode anything
      return;
  }

  writeSingleByteLenValue(stream, decoded);
}

size_t Address::estimateSize() const {
  size_t size = 0;
  size += sizeof((uint8_t)getType());
  std::vector<uint8_t> decoded;
  switch (getType()) {
    case AddressType::STANDARD:
      decoded = AssertDecodeBase58(toString());
      break;
    case AddressType ::MULTISIG:
      decoded = AssertDecodeBase58(toString());
      break;
    default:
      // if we don't know address type, do not encode anything
      return size;
  }

  size += singleByteLenValueSize(decoded);
  return size;
}

void Address::getPopBytes(WriteStream& stream) const {
  std::vector<uint8_t> bytes = AssertDecodeBase58(m_Address.substr(1));
  stream.write(std::vector<uint8_t>(
      bytes.begin(), bytes.begin() + ADDRESS_POP_DATA_SIZE_PROGPOW));
}

bool Address::operator==(const Address& other) const noexcept {
  return m_Address == other.m_Address;
}

bool Address::operator!=(const Address& other) const noexcept {
  return !(this->operator==(other));
}
bool Address::operator==(const std::string& other) const noexcept {
  return m_Address == other;
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                Address& out,
                                ValidationState& state) {
  uint8_t addressType = 0;
  if (!stream.readBE<uint8_t>(addressType, state)) {
    return state.Invalid("address-type");
  }

  Slice<const uint8_t> addressBytes;
  if (!readSingleByteLenValue(
          stream, addressBytes, state, 0, altintegration::ADDRESS_SIZE)) {
    return state.Invalid("address-bytes");
  }

  std::string addressText;
  switch ((AddressType)addressType) {
    case AddressType::STANDARD:
      addressText = EncodeBase58(addressBytes);
      break;
    case AddressType::MULTISIG:
      addressText = EncodeBase59(addressBytes);
      break;
    default:
      return state.Invalid("invalid-address-type");
  }

  if (!out.fromString(addressText, state)) {
    return state.Invalid("bad-addr");
  }

  return true;
}

Address Address::assertFromString(const std::string& input) {
  Address addr;
  ValidationState state;
  bool success = addr.fromString(input, state);
  VBK_ASSERT_MSG(success, state.toString());
  return addr;
}

// this code verifies that default address is valid
// NOLINTNEXTLINE(cert-err58-cpp)
static Address defaultAddress = [] {
  Address addr;
  ValidationState state;
  bool success = addr.fromString("V111111111111111111111111G3LuZ", state);
  VBK_ASSERT_MSG(success, state.toString());
  return addr;
}();

Address::Address() {
  this->m_Address = defaultAddress.m_Address;
  this->m_Type = defaultAddress.m_Type;
}

}  // namespace altintegration

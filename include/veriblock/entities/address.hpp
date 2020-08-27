// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_ADDRESS_HPP
#define ALT_INTEGRATION_ADDRESS_HPP

#include <cstdint>
#include <string>

#include "veriblock/read_stream.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/write_stream.hpp"

namespace altintegration {

enum class AddressType {
  ZERO_UNUSED = 0,
  STANDARD = 1,
  PROOF_OF_PROOF = 2,
  MULTISIG = 3,
};

struct Address {
  Address() = default;
  explicit Address(const std::string& input);

  bool operator==(const Address& other) const noexcept {
    return m_Address == other.m_Address;
  }
  bool operator!=(const Address& other) const noexcept {
    return !(this->operator==(other));
  }

  bool operator==(const std::string& other) const noexcept {
    return m_Address == other;
  }

  AddressType getType() const noexcept { return m_Type; }

  /**
   * Return a Pop bytes from the address
   * @param stream data stream to write into
   */
  void getPopBytes(WriteStream& stream) const;

  /**
   * Convert public key to VBK standard address.
   * @param publicKey byte array containing public key
   * @return Address containing VBK address
   */
  static Address fromPublicKey(Slice<const uint8_t> publicKey);

  /**
   * Check if given address is generated with provided public key
   * @param publicKey byte array containing public key
   * @return true if address is derived from public key
   */
  bool isDerivedFromPublicKey(Slice<const uint8_t> publicKey) const;

  /**
   * Parse provided string and convert it to VBK address
   * @param input should contain text representation of an address
   * @throws std::invalid_argument if provided string is not valid
   * @return Address containing VBK address
   */
  static Address fromString(const std::string& input);

  /**
   * Convert VBK address to text representation
   * @return string with VBK address
   */
  std::string toString() const noexcept;

  /**
   * Read data from the stream and convert it to VBK address
   * @param stream data stream to read from
   * @throws std::invalid_argument if stream data cannot be converted
   * to the VBK address
   * @return Address containing VBK address
   */
  static Address fromVbkEncoding(ReadStream& stream);

  /**
   * Convert VBK address to data stream using VBK byte format
   * @param stream data stream to write into
   * @throws std::invalid_argument if address has unsupported type
   */
  void toVbkEncoding(WriteStream& stream) const;

 private:
  Address(AddressType type, std::string addr)
      : m_Type(type), m_Address(std::move(addr)) {}

  AddressType m_Type{};
  std::string m_Address{};
};

template <typename Value>
inline Value ToJSON(const Address& addr) {
  return ToJSON<Value>(addr.toString());
}

bool Deserialize(ReadStream& stream,
                 Address& out,
                 ValidationState& state);

bool Deserialize(Slice<const uint8_t> data,
                 Address& out,
                 ValidationState& state);

}  // namespace altintegration

#endif

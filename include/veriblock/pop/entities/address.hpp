// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_ADDRESS_HPP
#define ALT_INTEGRATION_ADDRESS_HPP

#include <cstdint>
#include <string>
#include <veriblock/pop/read_stream.hpp>
#include <veriblock/pop/slice.hpp>
#include <veriblock/pop/write_stream.hpp>

namespace altintegration {

//! VBK Address type.
enum class AddressType {
  ZERO_UNUSED = 0,
  STANDARD = 1,
  PROOF_OF_PROOF = 2,
  MULTISIG = 3,
};

/**
 * @class Address
 *
 * Represents address on VBK chain.
 */
struct Address {
  Address();

  bool operator==(const Address& other) const noexcept;
  bool operator!=(const Address& other) const noexcept;
  bool operator==(const std::string& other) const noexcept;

  /**
   * Return address type.
   * @return
   */
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
   * @param[in] input should contain text representation of an address
   * @param[out] state validation state
   * @return Address containing VBK address
   */
  bool fromString(const std::string& input, ValidationState& state);

  static Address assertFromString(const std::string& input);

  /**
   * Convert VBK address to text representation
   * @return string with VBK address
   */
  std::string toString() const noexcept;

  /**
   * Convert VBK address to data stream using VBK byte format
   * @param stream data stream to write into
   * @throws std::invalid_argument if address has unsupported type
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

 private:
  Address(AddressType type, std::string addr)
      : m_Type(type), m_Address(std::move(addr)) {}

  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         Address& out,
                                         ValidationState& state);

  AddressType m_Type{};
  std::string m_Address{};
};

//! @overload
template <typename Value>
inline Value ToJSON(const Address& addr) {
  return ToJSON<Value>(addr.toString());
}

/**
 * Read data from the stream and convert it to VBK address
 * @param stream data stream to read from
 * @param[out] out output address
 * @param[out] state
 * @return Address containing VBK address
 */
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                Address& out,
                                ValidationState& state);

}  // namespace altintegration

#endif

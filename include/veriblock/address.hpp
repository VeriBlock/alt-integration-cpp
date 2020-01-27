#ifndef ALT_INTEGRATION_ADDRESS_HPP
#define ALT_INTEGRATION_ADDRESS_HPP

#include <cstdint>
#include <string>

#include "veriblock/entities/address_entity.hpp"
#include "veriblock/read_stream.hpp"
#include "veriblock/write_stream.hpp"
#include "veriblock/slice.hpp"

namespace VeriBlock {

/**
 * Convert public key to VBK standard address.
 * @param publicKey byte array containing public key
 * @return AddressEntity containing VBK address
 */
AddressEntity addressFromPublicKey(Slice<const uint8_t> publicKey);

/**
 * Check if given address is generated with provided public key
 * @param address VBK address
 * @param publicKey byte array containing public key
 * @return true if address is derived from public key
 */
bool addressIsDerivedFromPublicKey(const AddressEntity& address,
                                   Slice<const uint8_t> publicKey);

/**
 * Parse provided string and convert it to VBK address
 * @param input should contain text representation of an address
 * @throws std::invalid_argument if provided string is not valid
 * @return AddressEntity containing VBK address
 */
AddressEntity addressFromString(const std::string& input);

/**
 * Convert VBK address to text representation
 * @param address VBK address
 * @return string with VBK address
 */
std::string addressToString(const AddressEntity& address);

/**
 * Read data from the stream and convert it to VBK address
 * @param stream data stream to read from
 * @throws std::invalid_argument if stream data cannot be converted
 * to the VBK address
 * @return AddressEntity containing VBK address
 */
AddressEntity addressFromVbkEncoding(ReadStream& stream);

/**
 * Convert VBK address to data stream using VBK byte format
 * @param address VBK address
 * @param stream data stream to write into
 * @throws std::invalid_argument if address has unsupported type
 */
void addressToVbkEncoding(const AddressEntity& address, WriteStream& stream);

}  // namespace VeriBlock

#endif
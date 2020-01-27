#ifndef ALT_INTEGRATION_ADDRESS_ENTITY_HPP
#define ALT_INTEGRATION_ADDRESS_ENTITY_HPP

#include <cstdint>
#include <string>

#include "veriblock/consts.hpp"

namespace VeriBlock {

struct AddressEntity {
  AddressEntity() = default;

  AddressEntity(AddressType type, std::string addr)
      : m_Type(type), m_Address(std::move(addr)) {}

  bool operator==(const AddressEntity& other) const noexcept {
    return m_Address == other.m_Address;
  }

  bool operator==(const std::string& other) const noexcept {
    return m_Address == other;
  }

  const std::string& getAddr() const noexcept { return m_Address; }
  const AddressType getType() const noexcept { return m_Type; }

  private:
    AddressType m_Type{};
    std::string m_Address{};
};

}  // namespace VeriBlock

#endif
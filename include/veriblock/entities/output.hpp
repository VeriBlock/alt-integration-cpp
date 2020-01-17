#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_

#include "veriblock/entities/address.hpp"
#include "veriblock/entities/coin.hpp"

namespace VeriBlock {

struct Output {
  Address address;
  Coin coin;

  Output(Address _address, Coin _coin)
      : address(std::move(_address)), coin(_coin) {}

  bool operator==(const Output& other) const noexcept {
    return address == other.address && coin == other.coin;
  }

  static Output fromVbkEncoding(ReadStream& stream) {
    auto addr = Address::fromVbkEncoding(stream);
    auto amount = Coin::fromVbkEncoding(stream);
    return Output(addr, amount);
  }

  void toVbkEncoding(WriteStream& stream) const {
    address.toVbkEncoding(stream);
    coin.toVbkEncoding(stream);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_

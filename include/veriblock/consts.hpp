#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

namespace VeriBlock {

enum TxType {
  VBK_TX = 0x01,
  VBK_POP_TX = 0x02,
};

constexpr auto BTC_HEADER_SIZE = 80;
constexpr auto VBK_HEADER_SIZE = 64;

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONSTS_HPP_

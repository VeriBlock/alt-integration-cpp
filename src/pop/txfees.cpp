#include <veriblock/pop/txfees.hpp>

namespace altintegration {

Coin calculateTxFee(const VbkTx& tx) {
  decltype(Coin::units) output{0};
  for (const auto& o : tx.outputs) {
    output += o.coin.units;
  }
  VBK_ASSERT_MSG(tx.sourceAmount.units >= output,
                 "Output amount should be less or equal to the input amount");
  return Coin{tx.sourceAmount.units - output};
}

}  // namespace altintegration

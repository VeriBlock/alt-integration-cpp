#include <veriblock/checks.hpp>

namespace VeriBlock {

void checkRange(int64_t num, int64_t min, int64_t max) {
  if (num < min) {
    throw std::out_of_range("value is less than minimal");
  }

  if (num > max) {
    throw std::out_of_range("value is greater than maximum");
  }
}

}  // namespace VeriBlock
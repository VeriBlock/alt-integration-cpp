#ifndef ALTINTEGRATION_CONTEXT_HPP
#define ALTINTEGRATION_CONTEXT_HPP

#include <vector>

namespace altintegration {

struct BtcBlock;
struct VbkBlock;

struct VbkContext {
  std::vector<BtcBlock> btc;
};

struct AltContext {
  std::vector<BtcBlock> btc;
  std::vector<VbkBlock> vbk;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

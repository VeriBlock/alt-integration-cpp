#ifndef ALTINTEGRATION_CONTEXT_HPP
#define ALTINTEGRATION_CONTEXT_HPP

#include <tuple>
#include <vector>

#include "endorsements.hpp"

namespace altintegration {

struct BtcBlock;
struct VbkBlock;
struct VTB;
struct AltPayloads;

struct VbkContext {
  std::vector<BtcBlock> btc;
};

struct AltContext {
  std::vector<VbkBlock> vbk;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

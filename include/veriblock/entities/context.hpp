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
  // corresponds to the ATV
  std::vector<VbkBlock> vbk;

  std::vector<VTB> vtbs;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

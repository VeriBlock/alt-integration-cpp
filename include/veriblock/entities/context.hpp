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

  bool empty() const noexcept { return btc.empty(); }
};

struct AltContext {
  // corresponds to the ATV
  std::vector<VbkBlock> vbk;

  std::vector<VTB> vtbs;

  bool empty() const noexcept { return vbk.empty() && vtbs.empty(); }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

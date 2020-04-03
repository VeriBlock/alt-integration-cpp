#ifndef ALTINTEGRATION_CONTEXT_HPP
#define ALTINTEGRATION_CONTEXT_HPP

#include <vector>

#include "endorsements.hpp"

namespace altintegration {

struct BtcBlock;
struct VbkBlock;
struct VTB;

struct VbkContext {
  std::vector<BtcBlock> btc;
  BtcEndorsement endorsement;

  static VbkContext fromContainer(const VTB& vtb);
};

struct AltContext {
  std::vector<BtcBlock> btc;
  std::vector<VbkBlock> vbk;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

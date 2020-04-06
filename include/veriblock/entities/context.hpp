#ifndef ALTINTEGRATION_CONTEXT_HPP
#define ALTINTEGRATION_CONTEXT_HPP

#include <vector>

#include "endorsements.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"

namespace altintegration {

struct VTB;
struct AltPayloads;

struct VbkContext {
  std::vector<BtcBlock> btc;
  BtcEndorsement endorsement;

  static VbkContext fromContainer(const VTB& vtb);
};

struct VbkPayloads {
  VbkBlock containing;
  VbkContext context;
  std::vector<VbkBlock> contextVbkBlocks;
};

struct AltContext {
  // corresponds to the ATV
  std::vector<VbkBlock> vbk;
  VbkEndorsement endorsement;
  // corresponds to the VBTs
  std::vector<VbkPayloads> vbkContext;

  static AltContext fromContainer(const AltPayloads& altPayloads);
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

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
  BtcEndorsement endorsement;

  static VbkContext fromContainer(const VTB& vtb);
};

struct AltContext {
  // corresponds to the ATV
  std::vector<VbkBlock> vbk;
  VbkEndorsement endorsement;
  // corresponds to the VBTs
  std::vector<std::tuple<VbkBlock, VbkContext, std::vector<VbkBlock>>>
      vbkContext;
  // corresponds to the update context fields
  std::vector<BtcBlock> updateContextBtc;
  std::vector<VbkBlock> updateContextVbk;

  static AltContext fromContainer(const AltPayloads& altPayloads);
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

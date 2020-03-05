#ifndef ALT_INTEGRATION_PAYLOADS_HPP
#define ALT_INTEGRATION_PAYLOADS_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>

namespace VeriBlock {

struct AltProof {
  AltBlock endorsed;
  AltBlock containing;
  ATV atv;
};

struct Payloads {
  AltProof alt;
  std::vector<VTB> vtbs;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_PAYLOADS_HPP

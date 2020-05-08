// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_CONTEXT_HPP
#define ALTINTEGRATION_CONTEXT_HPP

#include <memory>
#include <vector>

#include "endorsements.hpp"

namespace altintegration {

struct BtcBlock;
struct VbkBlock;
struct VTB;

struct VbkContext {
  using eid_t = typename BtcEndorsement::id_t;

  std::vector<std::pair<eid_t, std::vector<std::shared_ptr<BtcBlock>>>>
      btc_context{};

  bool empty() const noexcept { return btc_context.empty(); }
};

struct PartialVTB {
  std::vector<std::shared_ptr<BtcBlock>> btc{};
  std::shared_ptr<VbkBlock> containing{};
  BtcEndorsement endorsement{};

  /**
   * Return a containing VbkBlock
   * @return containing block
   */
  VbkBlock getContainingBlock() const;

  /**
   * Return true if contains endorsement data
   * @return true if contains endorsement data
   */
  bool containsEndorsements() const;

  /**
   * Return BtcEndorsement of the PartialVbk
   * @return endorsement BtcEndorsement
   */
  BtcEndorsement getEndorsement() const;

  /**
   * Return BtcEndorsement id of the endorsement
   * @return endorsement.id BtcEndorsement::id_t
   */
  typename BtcEndorsement::id_t getEndorsementId() const;

  // generate without VBK context blocks, only BtcEndorsement VbkBlock
  // containing and btc blocks
  static PartialVTB fromVTB(const VTB& vtb);

  static std::vector<PartialVTB> fromVTB(const std::vector<VTB>& vtbs);

  friend bool operator==(const PartialVTB& a, const PartialVTB& b);
};

struct AltContext {
  // corresponds to the ATV
  std::vector<std::shared_ptr<VbkBlock>> vbk{};

  std::vector<PartialVTB> vtbs{};

  bool empty() const noexcept { return vbk.empty() && vtbs.empty(); }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXT_HPP

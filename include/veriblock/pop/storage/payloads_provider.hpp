// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP

#include <veriblock/pop/blockchain/blockchain_util.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/popdata.hpp>

namespace altintegration {

struct AltBlockTree;
struct VbkBlockTree;

/**
 * Accessor for ATV/VTB/VbkBlock bodies given hash. Technically, a facade over
 * on-disk storage.
 *
 * Altchains are expected to implement this interface.
 */
struct PayloadsStorage {
  virtual ~PayloadsStorage() = default;

  //! should write ATV identified by `id` into `out`, or return false
  //! @pure
  virtual bool getATV(const ATV::id_t& ids,
                      ATV& out,
                      ValidationState& state) = 0;
  //! should write VTB identified by `id` into `out`, or return false
  //! @pure
  virtual bool getVTB(const VTB::id_t& ids,
                      VTB& out,
                      ValidationState& state) = 0;
  //! should write VbkBlock identified by `id` into `out`, or return false
  //! @pure
  virtual bool getVBK(const VbkBlock::id_t& id,
                      VbkBlock& out,
                      ValidationState& state) = 0;

  //! @pure
  virtual void writePayloads(const PopData& payloads) = 0;

  //! alias for getVBK
  bool getPayload(const VbkBlock::id_t& id,
                  VbkBlock& out,
                  ValidationState& state) {
    return getVBK(id, out, state);
  }
  //! alias for getVTB
  bool getPayload(const VTB::id_t& id, VTB& out, ValidationState& state) {
    return getVTB(id, out, state);
  }
  //! alias for getATV
  bool getPayload(const ATV::id_t& id, ATV& out, ValidationState& state) {
    return getATV(id, out, state);
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP

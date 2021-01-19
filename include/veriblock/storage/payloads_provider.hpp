// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP
#define VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP

#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/popdata.hpp>

/**
 * @defgroup interfaces Interfaces to be implemented
 * These interfaces must be implemented by Altchain developers for integration
 * of POP protocol.
 */

namespace altintegration {

struct AltBlockTree;
struct VbkBlockTree;

namespace details {
struct PayloadsReader {
  virtual ~PayloadsReader() = default;

  //! should write ATV identified by `id` into `out`, or return false
  virtual bool getATV(const ATV::id_t& ids,
                      ATV& out,
                      ValidationState& state) = 0;
  //! should write VTB identified by `id` into `out`, or return false
  virtual bool getVTB(const VTB::id_t& ids,
                      VTB& out,
                      ValidationState& state) = 0;
  //! should write VbkBlock identified by `id` into `out`, or return false
  virtual bool getVBK(const VbkBlock::id_t& id,
                      VbkBlock& out,
                      ValidationState& state) = 0;

  /**
   * Load commands from a particular block.
   * @param[in] tree load from this tree
   * @param[in] block load from this block
   * @param[out] out output vector of commands
   * @param[out] state if commands can't be loaded, will be set to Error
   */
  virtual void getCommands(AltBlockTree& tree,
                           const BlockIndex<AltBlock>& block,
                           std::vector<CommandGroup>& out,
                           ValidationState& state);

  /**
   * Load commands from a particular block.
   * @param[in] tree load from this tree
   * @param[in] block load from this block
   * @param[out] out output vector of commands
   * @param[out] state if commands can't be loaded, will be set to Error
   */
  virtual void getCommands(VbkBlockTree& tree,
                           const BlockIndex<VbkBlock>& block,
                           std::vector<CommandGroup>& out,
                           ValidationState& state);
};

struct PayloadsWriter {
  virtual ~PayloadsWriter() = default;

  virtual void writePayloads(const std::vector<ATV>& atvs) = 0;

  virtual void writePayloads(const std::vector<VTB>& vtbs) = 0;

  virtual void writePayloads(const std::vector<VbkBlock>& vbks) = 0;

  virtual void writePayloads(const PopData& payloads);
};

}  // namespace details

/**
 * @struct PayloadsProvider
 *
 * An abstraction over on-disk storage.
 *
 * veriblock-pop-cpp does not dictate how to store payloads on-disk. Altchains
 * must create derived class and provide it to AltBlockTree, so that it can
 * fetch payloads from disk during state changes.
 *
 * @ingroup interfaces
 */
struct PayloadsProvider {
  virtual ~PayloadsProvider() = default;

  virtual details::PayloadsReader& getPayloadsReader() = 0;

  virtual details::PayloadsWriter& getPayloadsWriter() = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PAYLOADS_PROVIDER_HPP

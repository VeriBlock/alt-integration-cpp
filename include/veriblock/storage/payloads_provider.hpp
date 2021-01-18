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

  //! should write ALL ATVs identified by `id` into `out`, or return false
  virtual bool getATVs(const std::vector<ATV::id_t>& ids,
                       std::vector<ATV>& out,
                       ValidationState& state) = 0;
  //! should write ALL VTBs identified by `id` into `out`, or return false
  virtual bool getVTBs(const std::vector<VTB::id_t>& ids,
                       std::vector<VTB>& out,
                       ValidationState& state) = 0;
  //! should write ALL VbkBlocks identified by `id` into `out`, or return false
  virtual bool getVBKs(const std::vector<VbkBlock::id_t>& ids,
                       std::vector<VbkBlock>& out,
                       ValidationState& state) = 0;

  virtual bool getATV(const ATV::id_t& id,
                      ATV& out,
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

  virtual bool writePayloads(const std::vector<ATV>& atvs) = 0;

  virtual bool writePayloads(const std::vector<VTB>& vtbs) = 0;

  virtual bool writePayloads(const std::vector<VbkBlock>& vbks) = 0;

  bool writePayloads(const PopData& payloads);
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

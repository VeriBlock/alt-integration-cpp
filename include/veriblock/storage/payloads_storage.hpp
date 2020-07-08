// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/db_error.hpp>
#include <veriblock/storage/payloads_repository.hpp>
#include <veriblock/command_group_cache.hpp>

namespace altintegration {

struct VbkBlockTree;

class PayloadsStorage {
 public:
  virtual ~PayloadsStorage() = default;

  template <typename Payloads>
  PayloadsRepository<Payloads>& getRepo();

  template <typename Payloads>
  const PayloadsRepository<Payloads>& getRepo() const;

  template <typename Payloads>
  Payloads loadPayloads(const typename Payloads::id_t& pid) {
    Payloads payloads;
    auto& repo = getRepo<Payloads>();
    bool ret = repo.get(pid, &payloads);
    if (!ret) {
      throw db::StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    return payloads;
  }

  template <typename Payloads>
  void savePayloads(const Payloads& payloads) {
    auto& repo = getRepo<Payloads>();
    repo.put(payloads);
  }

  template <typename Payloads>
  void savePayloadsMany(const std::vector<Payloads>& payloads) {
    auto& repo = getRepo<Payloads>();
    auto batch = repo.newBatch();
    for (const auto& p : payloads) {
      batch->put(p);
    }
    batch->commit();
  }

  void savePayloads(const PopData& pop) {
    savePayloadsMany(pop.context);
    savePayloadsMany(pop.vtbs);
    savePayloadsMany(pop.atvs);
  }

  // realisation in the alt_block_tree, vbK_block_tree
  template <typename BlockTree>
  std::vector<CommandGroup> loadCommands(
      const typename BlockTree::index_t& index, BlockTree& tree);

  template <typename Payloads>
  void setValidity(const typename Payloads::id_t& pid, bool valid) {
    auto payloads = loadPayloads<Payloads>(pid);
    payloads.valid = valid;
    savePayloads(payloads);
  }

  template <typename Payloads>
  bool getValidity(const typename Payloads::id_t& pid) {
    auto payloads = loadPayloads<Payloads>(pid);
    return payloads.valid;
  }

  void setValidity(const CommandGroup& cg, bool valid) {
    if (cg.getPayloadsTypeName() == altintegration::VbkBlock::name()) {
      return setValidity<altintegration::VbkBlock>(
          altintegration::VbkBlock::id_t(cg.id), valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::VTB::name()) {
      return setValidity<altintegration::VTB>(altintegration::VTB::id_t(cg.id),
                                              valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::ATV::name()) {
      return setValidity<altintegration::ATV>(altintegration::ATV::id_t(cg.id),
                                              valid);
    }

    VBK_ASSERT(false && "should not get here");
  }

  template <typename Tree, typename Payloads>
  void payloadsToCommands_(Tree& tree,
                           const Payloads& payloads,
                           const typename Tree::block_t&,
                           std::vector<CommandPtr>& commands) {
    tree.payloadsToCommands(payloads, commands);
  }

  template <typename Tree>
  void payloadsToCommands_(Tree& tree,
                           const ATV& payloads,
                           const typename Tree::block_t& containing,
                           std::vector<CommandPtr>& commands) {
    tree.payloadsToCommands(payloads, containing, commands);
  }

  template <typename Tree, typename Payloads>
  std::vector<CommandGroup> loadCommandsStorage(
      const typename Tree::index_t& index, Tree& tree) {
    auto& pids = index.template getPayloadIds<Payloads, typename Payloads::id_t>();
    std::vector<CommandGroup> out{};
    for (const auto& pid : pids) {
      CommandGroup cg(pid.asVector(), true, Payloads::name());
      auto& cache = getCache<Tree, Payloads>();
      if (!cache.get(pid.asVector(), &cg)) {
        Payloads payloads;
        if (!getRepo<Payloads>().get(pid, &payloads)) {
          throw db::StateCorruptedException(
              fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
        }
        cg.valid = payloads.valid;
        payloadsToCommands_(tree, payloads, *index.header, cg.commands);
        cache.put(cg);
      }
      out.push_back(cg);
    }
    return out;
  }

 protected:
  std::shared_ptr<PayloadsRepository<ATV>> _repoAtv;
  std::shared_ptr<PayloadsRepository<VTB>> _repoVtb;
  std::shared_ptr<PayloadsRepository<VbkBlock>> _repoBlocks;
  CommandGroupCache _cacheAlt;
  CommandGroupCache _cacheVbk;

  template <typename Tree, typename Payloads>
  CommandGroupCache& getCache();
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
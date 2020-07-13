// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/command_group_cache.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/db_error.hpp>
#include <veriblock/storage/payloads_repository.hpp>

namespace altintegration {

struct VbkBlockTree;

class PayloadsStorage {
  using id_t = typename CommandGroupCache::id_t;

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
    using index_t = typename Tree::index_t;
    auto& pids =
        index.template getPayloadIds<Payloads, typename Payloads::id_t>();
    std::vector<CommandGroup> out{};
    for (const auto& pid : pids) {
      auto& cache = getCache<Tree, Payloads>();
      auto cid = getCommandGroupId<index_t, Payloads>(index, pid);
      CommandGroup cg(pid.asVector(), true, Payloads::name());
      if (!cache.get(cid, &cg)) {
        Payloads payloads;
        if (!getRepo<Payloads>().get(pid, &payloads)) {
          throw db::StateCorruptedException(
              fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
        }
        payloadsToCommands_(tree, payloads, *index.header, cg.commands);
        cache.put(cid, cg);
      }
      // create new validity record if does not exist
      auto it = _cgValidity.find(cid);
      if (it == _cgValidity.end()) {
        _cgValidity[cid] = true;
      }
      cg.valid = _cgValidity[cid];
      out.push_back(cg);
    }
    return out;
  }

  template <typename BlockIndex>
  void setValidity(const CommandGroup& cg,
                   const BlockIndex& index,
                   bool valid) {
    if (cg.getPayloadsTypeName() == altintegration::VbkBlock::name()) {
      return setValidity<altintegration::VbkBlock, BlockIndex>(
          altintegration::VbkBlock::id_t(cg.id), index, valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::VTB::name()) {
      return setValidity<altintegration::VTB, BlockIndex>(
          altintegration::VTB::id_t(cg.id), index, valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::ATV::name()) {
      return setValidity<altintegration::ATV, BlockIndex>(
          altintegration::ATV::id_t(cg.id), index, valid);
    }

    VBK_ASSERT(false && "should not get here");
  }

  template <typename Payloads, typename BlockIndex>
  void setValidity(const typename Payloads::id_t& pid,
                   const BlockIndex& index,
                   bool valid) {
    auto cid = getCommandGroupId<BlockIndex, Payloads>(index, pid);
    _cgValidity[cid] = valid;
  }

  template <typename Payloads, typename BlockIndex>
  bool isValid(const typename Payloads::id_t& pid, const BlockIndex& index) {
    auto cid = getCommandGroupId<BlockIndex, Payloads>(index, pid);
    auto it = _cgValidity.find(cid);
    if (it == _cgValidity.end()) {
      throw db::StateCorruptedException(fmt::sprintf(
          "CommandGroup id={%s} validity is not set", HexStr(cid)));
    }
    return it->second;
  }

  template <typename Payloads, typename BlockIndex>
  bool isExisting(const typename Payloads::id_t& pid, const BlockIndex& index) {
    auto cid = getCommandGroupId<BlockIndex, Payloads>(index, pid);
    return _cgValidity.find(cid) != _cgValidity.end();
  }

 protected:
  std::shared_ptr<PayloadsRepository<ATV>> _repoAtv;
  std::shared_ptr<PayloadsRepository<VTB>> _repoVtb;
  std::shared_ptr<PayloadsRepository<VbkBlock>> _repoBlocks;
  CommandGroupCache _cacheAlt;
  CommandGroupCache _cacheVbk;
  std::unordered_map<id_t, bool> _cgValidity;

  template <typename BlockIndex, typename Payloads>
  id_t getCommandGroupId(const BlockIndex& index,
                         const typename Payloads::id_t& pid) {
    auto out = pid.asVector();
    auto hash = index.getHash();
    out.insert(out.end(), hash.begin(), hash.end());
    return id_t(out);
  }

  template <typename Tree, typename Payloads>
  CommandGroupCache& getCache();
};

template <typename Tree, typename Payloads>
inline CommandGroupCache& PayloadsStorage::getCache() {
  return _cacheAlt;
}

template <>
inline CommandGroupCache& PayloadsStorage::getCache<VbkBlockTree, VTB>() {
  return _cacheVbk;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
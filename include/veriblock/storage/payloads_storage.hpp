// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/command_group_cache.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/db_error.hpp>
#include <veriblock/storage/payloads_repository.hpp>

#include "repository.hpp"

namespace altintegration {

struct AltTree;

constexpr const char DB_VBK_PREFIX = '^';
constexpr const char DB_VTB_PREFIX = '<';
constexpr const char DB_ATV_PREFIX = '>';

class PayloadsStorage {
  using id_t = typename CommandGroupCache::id_t;

 public:
  virtual ~PayloadsStorage() = default;

  PayloadsStorage(std::shared_ptr<Repository> repo);

  //! getter for cached payload validity
  bool getValidity(Slice<const uint8_t> containingBlock,
                   Slice<const uint8_t> payloadId);
  //! setter for payload validity
  void setValidity(Slice<const uint8_t> containingBlock,
                   Slice<const uint8_t> payloadId,
                   bool validity);

  void reindex(const AltTree& tree);

  void savePayloads(const PopData& pop);
  void savePayloads(const std::vector<VTB>& vtbs);

  // get a list of ALT containing blocks for given payload
  const std::set<AltBlock::hash_t>& getContainingAltBlocks(
      const std::vector<uint8_t>& payloadId);
  // get a list of VBK containing blocks for given payload
  const std::set<VbkBlock::hash_t>& getContainingVbkBlocks(
      const std::vector<uint8_t>& payloadId);
  void addBlockToIndex(const BlockIndex<AltBlock>& block);
  void addBlockToIndex(const BlockIndex<VbkBlock>& block);
  // add ALT payload to index
  void addAltPayloadIndex(const AltBlock::hash_t& containing,
                          const std::vector<uint8_t>& payloadId);
  // add VBK payload to index
  void addVbkPayloadIndex(const VbkBlock::hash_t& containing,
                          const std::vector<uint8_t>& payloadId);
  // remove ALT payload from index
  void removeAltPayloadIndex(const AltBlock::hash_t& containing,
                             const std::vector<uint8_t>& payloadId);
  // remove VBK payload from index
  void removeVbkPayloadIndex(const VbkBlock::hash_t& containing,
                             const std::vector<uint8_t>& payloadId);

  void removePayloadsIndex(const BlockIndex<AltBlock>& block);
  void removePayloadsIndex(const BlockIndex<VbkBlock>& block);

  Repository& getRepo();

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
      char prefix, const typename Tree::index_t& index, Tree& tree) {
    using id_t = typename Payloads::id_t;
    auto& pids = index.template getPayloadIds<Payloads, id_t>();
    std::vector<CommandGroup> out{};
    auto containingHash = index.getHash();
    for (const auto& pid : pids) {
      auto& cache = getCache<Tree, Payloads>();
      auto cid = makeGlobalPid(containingHash, pid);
      CommandGroup cg(pid.asVector(), true, Payloads::name());
      if (!cache.get(cid, &cg)) {
        Payloads payloads;
        if (!repo_->getObject(std::make_pair(prefix, pid), &payloads)) {
          throw db::StateCorruptedException(
              fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
        }
        payloadsToCommands_(tree, payloads, index.getHeader(), cg.commands);
        cache.put(cid, cg);
      }
      cg.valid = getValidity(containingHash, pid);
      out.push_back(cg);
    }
    return out;
  }

 private:
  template <typename Tree, typename Payloads>
  CommandGroupCache& getCache();

  std::vector<uint8_t> makeGlobalPid(Slice<const uint8_t> a,
                                     Slice<const uint8_t> b);

 protected:
  std::shared_ptr<Repository> repo_;
  CommandGroupCache _cacheAlt;
  CommandGroupCache _cacheVbk;

  // reverse index. stores invalid payloads only.
  // key = <containing hash + payload id>
  // value =
  //  true  -> payload is valid in containing block
  //  false -> payload is invalid in containing block
  // if key is missing in this map, assume payload is valid
  std::unordered_map<std::vector<uint8_t>, bool> _cgValidity;

  // reverse index
  // key = id of payload
  // value = set of ALT/VBK blocks containing that payload
  std::map<std::vector<uint8_t>, std::set<AltBlock::hash_t>> payload_in_alt;
  std::map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>> payload_in_vbk;
};

template <typename Tree, typename Payloads>
inline CommandGroupCache& PayloadsStorage::getCache() {
  return _cacheAlt;
}

struct VbkBlockTree;
template <>
inline CommandGroupCache& PayloadsStorage::getCache<VbkBlockTree, VTB>() {
  return _cacheVbk;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
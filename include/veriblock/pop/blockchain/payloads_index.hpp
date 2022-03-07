#ifndef DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0
#define DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <veriblock/pop/assert.hpp>

#include "payloads_index_detail.hpp"

namespace altintegration {

//! Stores a mapping "payload id -> containing block" hash of payloads that are
//! stored in **finalized** blocks.
//! @invariant KV are never removed from this index.
template <typename ContainerIndex>
struct FinalizedPayloadsIndex {
  using index_t = ContainerIndex;
  using hash_t = typename ContainerIndex::hash_t;
  using payload_id = std::vector<uint8_t>;

  const hash_t* find(const payload_id& pid) const {
    auto it = map_.find(pid);
    if (it == map_.end()) {
      return nullptr;
    }

    return &it->second;
  }

  bool empty() const { return map_.empty(); }

  size_t size() const { return map_.size(); }

  void add(const payload_id& pid, const hash_t& hash) {
    auto it = map_.find(pid);
    VBK_ASSERT(it == map_.end());
    map_.insert(std::make_pair(pid, hash));
  }

  void remove(const payload_id& pid) {
    auto it = map_.find(pid);
    VBK_ASSERT(it != map_.end());
    map_.erase(pid);
  }

  void addBlock(const index_t& block) {
    // in FinalizedPayloadsIndex we can add only finalized payloads!
    VBK_ASSERT_MSG(block.finalized, block.toPrettyString());
    detail::PLIAddBlock(*this, block);
  }

  const std::unordered_map<payload_id, hash_t>& getAll() const { return map_; }

 private:
  std::unordered_map<payload_id, hash_t> map_;
};

//! Payloads index that stores mapping "payload id -> set of containing blocks"
//! from all NON-FINALIZED blocks.
//! @note KV in this index should be removed BEFORE block is finalized.
template <typename IndexT>
struct PayloadsIndex {
  using index_t = IndexT;
  using hash_t = typename index_t::hash_t;
  using payload_id = std::vector<uint8_t>;

  void addBlock(const index_t& block) {
    // in PayloadsIndex we can add only NON-FINALIZED blocks!
    VBK_ASSERT_MSG(!block.finalized, block.toPrettyString());
    detail::PLIAddBlock(*this, block);
  }

  void removeBlock(const index_t& block) {
    // when blocks are removed from PayloadsIndex they are mostly non finalized.
    // but there's an edge case: when we deallocate entire Tree (in destructor)
    // we can remove finalized blocks.
    detail::PLIRemoveBlock(*this, block);
  }

  void add(const payload_id& id, const hash_t& block) {
    map_[id].insert(block);
  }

  void remove(const payload_id& id, const hash_t& block) {
    auto it = map_.find(id);
    VBK_ASSERT_MSG(it != map_.end(),
                   "Can not remove payload %s from %s block %s",
                   HexStr(id),
                   index_t::block_t::name(),
                   HexStr(block));
    auto& set = it->second;
    size_t erased = set.erase(block);
    VBK_ASSERT(erased == 1);

    if (set.empty()) {
      // "value" set is empty, we can cleanup key
      erased = map_.erase(id);
      VBK_ASSERT(erased == 1);
    }
  }

  const std::set<hash_t>& find(const payload_id& id) const {
    static std::set<hash_t> empty;
    auto it = map_.find(id);
    if (it == map_.end()) {
      return empty;
    }

    return it->second;
  }

  const std::unordered_map<payload_id, std::set<hash_t>>& getAll() const {
    return map_;
  }

 private:
  std::unordered_map<payload_id, std::set<hash_t>> map_;
};

}  // namespace altintegration

#endif /* DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0 */

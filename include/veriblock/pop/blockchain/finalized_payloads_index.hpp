#ifndef DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0
#define DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>

namespace altintegration {

//! Stores a mapping payload id -> containing block hash of payloads that are
//! stored in **finalized** blocks.
template <typename ContainerIndex>
struct FinalizedPayloadsIndex {
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

 private:
  std::unordered_map<payload_id, hash_t> map_;
};

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<AltBlock>>& pl,
                               const BlockIndex<AltBlock>& index);

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<VbkBlock>>& pl,
                               const BlockIndex<VbkBlock>& index);

void FinalizedPayloadsAddBlock(FinalizedPayloadsIndex<BlockIndex<BtcBlock>>& pl,
                               const BlockIndex<BtcBlock>& index);

}  // namespace altintegration

#endif /* DE6BAACA_91D4_4C99_B0C5_F1AB318E58B0 */

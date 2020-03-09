#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbktx.hpp"
#include "veriblock/storage/endorsement_repository.hpp"

namespace std {

template <>
struct hash<std::vector<uint8_t>> {
  size_t operator()(const std::vector<uint8_t>& v) const {
    return std::hash<std::string>{}({v.begin(), v.end()});
  }
};

}  // namespace std

namespace VeriBlock {

template <typename Endorsement>
struct EndorsementRepositoryInmem : public EndorsementRepository<Endorsement> {
  using endorsement_t = Endorsement;
  using eid_t = typename Endorsement::id_t;
  using endorsed_hash_t = typename Endorsement::endorsed_hash_t;
  using containing_hash_t = typename Endorsement::containing_hash_t;
  using container_t = typename Endorsement::container_t;

  ~EndorsementRepositoryInmem() override = default;

  void remove(const eid_t& id) override {
    for (auto& el : e_) {
      auto& set = el.second;
      auto end = set.end();
      auto it = set.find(id);
      if (it != end) {
        set.erase(it);
        return;
      }
    }
  }

  void remove(const container_t& container) override {
    remove(Endorsement::getId(container));
  }

  void put(const container_t& container) override {
    auto e = Endorsement::fromContainer(container);

    e_[e.endorsedHash][e.id] = e;
  }

  std::vector<endorsement_t> get(
      const endorsed_hash_t& endorsedBlockHash) const override {
    auto it = e_.find(endorsedBlockHash);
    if (it == e_.end()) {
      return {};
    }

    auto& m = it->second;
    std::vector<endorsement_t> ret;
    ret.reserve(m.size());

    std::transform(
        m.begin(),
        m.end(),
        std::back_inserter(ret),
        [](const std::pair<eid_t, endorsement_t>& p) { return p.second; });

    return ret;
  }

 private:
  // [endorsed block hash] => [endorsement id] => endorsement
  // clang-format off
  std::unordered_map<
    endorsed_hash_t,
    std::unordered_map<eid_t, endorsement_t>
  > e_;
  // clang-format on
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_BTC_ENDORSEMENTS_REPOSITORY_INMEM_HPP_

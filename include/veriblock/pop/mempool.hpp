// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "blockchain/alt_block_tree.hpp"
#include "blockchain/mempool_block_tree.hpp"
#include "entities/popdata.hpp"
#include "mempool_result.hpp"
#include "serde.hpp"
#include "signals.hpp"
#include "value_sorted_map.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/json.hpp"
#include "veriblock/pop/validation_state.hpp"

namespace altintegration {

/**
 * @struct MemPool
 *
 * Data structure, that stores in memory all valid payloads (ATV, VTB, VbkBlock)
 * submitted by other peers through p2p or by users through rpc.
 *
 * @invariant All payloads in MemPool are at least statelessly valid.
 * @invariant Those payloads that are not contextually valid *given current
 * view* may become valid or invalid later. Example: VTB's VBK containing block
 * is 100, currently last known VBK block is 90. ATM we can't connect VTB to
 * current VBK chain, so this VTB remains in MemPool until we explicitly remove
 * it, or connect containing VBK block with this VTB.
 *
 */
struct MemPool {
  enum Status { VALID = 0, FAILED_STATEFUL = 1, FAILED_STATELESS = 2 };

  //! Result of submission to mempool.
  struct SubmitResult {
    Status status = VALID;

    // by default created in valid state
    SubmitResult() = default;

    SubmitResult(bool state) {
      VBK_ASSERT_MSG(
          state, "SubmitResult can be implicitly constructed from bool=true");
    }

    SubmitResult(Status status, bool /* ignore */) : status(status) {}

    operator bool() const { return isAccepted(); }

    bool isAccepted() const {
      return status == VALID || status == FAILED_STATEFUL;
    }

    bool isValid() const { return status == VALID; }

    bool isFailedStateless() const { return status == FAILED_STATELESS; }

    bool isFailedStateful() const { return status == FAILED_STATEFUL; }
  };

  using vbk_hash_t = typename VbkBlock::prev_hash_t;

  template <typename Payload>
  using payload_map =
      std::unordered_map<typename Payload::id_t, std::shared_ptr<Payload>>;

  template <typename Payload>
  using payload_value_sorted_map =
      ValueSortedMap<typename Payload::id_t, std::shared_ptr<Payload>>;

  using vbk_map_t = payload_map<VbkBlock>;
  using atv_map_t = payload_map<ATV>;
  using vtb_map_t = payload_map<VTB>;
  using relations_map_t = payload_map<VbkPayloadsRelations>;

  using vbk_value_sorted_map_t = payload_value_sorted_map<VbkBlock>;
  using vtb_value_sorted_map_t = payload_value_sorted_map<VTB>;
  using atv_value_sorted_map_t = payload_value_sorted_map<ATV>;

  ~MemPool() = default;
  MemPool(AltBlockTree& tree);

  /**
   * Use this method to determine if payload of type T with id `id` is already
   * known to active chain.
   * @tparam T ATV or VTB or VbkBlock
   * @param[in] id payload id
   * @param[in] onlyInMempool if true, blockchain won't be searched for "known"
   * payloads.
   * @return true if payload exists in mempool or active chain, false otherwise.
   */
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  VBK_CHECK_RETURN bool isKnown(const typename T::id_t& id,
                                const bool onlyInMempool = false) const {
    // is `id` in mempool?
    auto* inmempool = get<T>(id);
    if (inmempool != nullptr) {
      return true;
    }

    if (onlyInMempool) {
      // we did not find in mempool, so exit early
      return false;
    }

    const std::vector<uint8_t> v(id.begin(), id.end());

    auto& tree = mempool_tree_.alt();
    auto& fpl = tree.getFinalizedPayloadsIndex();
    const auto* blockhash = fpl.find(v);
    if (blockhash != nullptr) {
      // finalized payloads index stores only payloads from finalized blocks.
      // all finalized blocks are on active chain.
      return true;
    }

    auto& pl = tree.getPayloadsIndex();
    // is `id` in payloads index?
    const auto& set = pl.find(v);
    if (set.empty()) {
      return false;
    }

    // check if any of candidates is on active chain
    for (const auto& hash : set) {
      const auto* candidate = tree.getBlockIndex(hash);
      // all candidates must exist in a tree after split on
      // PayloadsIndex+FinalizedPayloadsIndex. only finalized blocks may not
      // exist in a tree.
      VBK_ASSERT_MSG(candidate != nullptr, candidate->toPrettyString());
      if (tree.getBestChain().contains(candidate)) {
        // candidate is on main chain
        return true;
      }
    }

    // none of candidates are on main chain
    return false;
  }

  //! getter for payloads stored in mempool
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  VBK_CHECK_RETURN const T* get(const typename T::id_t& id) const {
    const auto& map = getMap<T>();
    auto it = map.find(id);
    if (it != map.end()) {
      return it->second.get();
    }

    const auto& inflight = getInFlightMap<T>();
    auto it2 = inflight.find(id);
    if (it2 != inflight.end()) {
      return it2->second.get();
    }

    return nullptr;
  }

  /**
   * Add new payload to mempool.
   *
   * Does stateless validation, and conditionally contextual validation.
   *
   *
   * @note When payloads are received by rpc, you can do contextual validation
   * immediately. If payloads are added during reorg/p2p sync, you can disable
   * contextual validation, as otherwise it may ban valid payloads from remote
   * peers.
   *
   * @tparam Raw represantation one of VTB, ATV, VbkBlock
   * @param[in] bytes payload`s bytes
   * @param[out] state validation state
   * validation
   * @return true if payload is accepted to mempool, false otherwise
   */
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  VBK_CHECK_RETURN SubmitResult submit(Slice<const uint8_t> bytes,
                                       bool doIsBlockOldCheck,
                                       ValidationState& state) {
    ReadStream stream(bytes);
    T payload;
    if (!DeserializeFromVbkEncoding(stream, payload, state)) {
      return {FAILED_STATELESS,
              state.Invalid("pop-mempool-submit-deserialize")};
    }

    return submit<T>(payload, doIsBlockOldCheck, state);
  }

  /**
   * Add new payload to mempool.
   *
   * Does stateless validation, and conditionally contextual validation.
   *
   *
   * @note When payloads are received by rpc, you can do contextual validation
   * immediately. If payloads are added during reorg/p2p sync, you can disable
   * contextual validation, as otherwise it may ban valid payloads from remote
   * peers.
   *
   * @tparam T one of VTB, ATV, VbkBlock
   * @param[in] pl payload
   * @param[out] state validation state
   * validation
   * @return true if payload is accepted to mempool, false otherwise
   */
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  VBK_CHECK_RETURN SubmitResult submit(const T& pl,
                                       bool doIsBlockOldCheck,
                                       ValidationState& state) {
    return submit<T>(std::make_shared<T>(pl), doIsBlockOldCheck, state);
  }

  /**
   * Add new payload to mempool.
   *
   * Does stateless validation, and conditionally contextual validation.
   *
   *
   * @note When payloads are received by rpc, you can do contextual validation
   * immediately. If payloads are added during reorg/p2p sync, you can disable
   * contextual validation, as otherwise it may ban valid payloads from remote
   * peers.
   *
   * @tparam shared_ptr<T> one of VTB, ATV, VbkBlock
   * @param[in] pl payload
   * @param[out] state validation state
   * @return true if payload is accepted to mempool, false otherwise
   */
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  VBK_CHECK_RETURN SubmitResult submit(const std::shared_ptr<T>& pl,
                                       bool doIsBlockOldCheck,
                                       ValidationState& state) {
    (void)pl;
    (void)state;
    (void)doIsBlockOldCheck;
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::submit");
    return {};
  }

  //! @private
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  const payload_map<T>& getMap() const {
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::getMap");
  }

  //! @private
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  const payload_value_sorted_map<T>& getInFlightMap() const {
    static_assert(sizeof(T) == 0,
                  "Undefined type used in MemPool::getInFlightMap");
  }

  std::vector<BtcBlock::hash_t> getMissingBtcBlocks() const;

  /**
   * (POW) Miners should execute this to get POP content for "next block".
   *
   * @warning Expensive operation. It builds virtual VBK Block Tree with
   * payloads stored in mempool, applies them to current AltBlockTree tip. All
   * payloads that can not be connected will remain in mempool. As a result,
   * this method returns altintegration::PopData which contains fully valid and
   * connected payloads. This should be inserted into AltBlock as is.
   *
   * @return statefully valid altintegration::PopData that can be connected to
   * current tip.
   */
  VBK_CHECK_RETURN PopData generatePopData();
  //! @overload
  //! @param[in] onATV a callback that is executed when ATV have been considered
  //! @param[in] onVTB a callback that is executed when VTB have been considered
  //! @param[in] onVBK a callback that is executed when VbkBlock have been
  //! considered
  VBK_CHECK_RETURN PopData generatePopData(
      const std::function<void(const ATV&, const ValidationState&)>& onATV,
      const std::function<void(const VTB&, const ValidationState&)>& onVTB,
      const std::function<void(const VbkBlock&, const ValidationState&)>&
          onVBK);

  /**
   * Remove payloads from mempool by their IDs.
   *
   * Use it when new block arrives and it contains PopData. Doing this, mempool
   * also executes clean() method.
   * @param[in] popData altintegration::PopData
   */
  void removeAll(const PopData& popData);

  /**
   * Remove paylaods that are statefully invalid anymore (duplicated, staled
   * payloads)
   */
  void cleanUp();

  /**
   * Clear mempool from all payloads.
   */
  void clear();

  /**
   * Subscribe on "accepted" event - fires whenever new payload is added into
   * mempool.
   * @tparam Pop ATV, VTB or VbkBlock
   * @param f callback
   * @return subscription id
   */
  template <typename T,
            typename = typename std::enable_if<IsPopPayload<T>::value>::type>
  size_t onAccepted(std::function<void(const T& p)> f) {
    auto& sig = getSignal<T>();
    return sig.connect(f);
  }

  //! fires when new valid ATV is accepted to mempool
  signals::Signal<void(const ATV& atv)> on_atv_accepted;
  //! fires when new valid VTB is accepted to mempool
  signals::Signal<void(const VTB& atv)> on_vtb_accepted;
  //! fires when new valid VbkBlock is accepted to mempool
  signals::Signal<void(const VbkBlock& atv)> on_vbkblock_accepted;

 private:
  MemPoolBlockTree mempool_tree_;
  // relations between VBK block and payloads
  relations_map_t relations_;
  vbk_map_t vbkblocks_;
  atv_map_t stored_atvs_;
  vtb_map_t stored_vtbs_;

  atv_value_sorted_map_t atvs_in_flight_{
      [](const std::shared_ptr<ATV>& v1,
         const std::shared_ptr<ATV>& v2) -> bool {
        return v1->blockOfProof.getHeight() < v2->blockOfProof.getHeight();
      }};
  vtb_value_sorted_map_t vtbs_in_flight_{
      [](const std::shared_ptr<VTB>& v1,
         const std::shared_ptr<VTB>& v2) -> bool {
        return v1->containingBlock.getHeight() <
               v2->containingBlock.getHeight();
      }};
  vbk_value_sorted_map_t vbkblocks_in_flight_{
      [](const std::shared_ptr<VbkBlock>& v1,
         const std::shared_ptr<VbkBlock>& v2) -> bool {
        return v1->getHeight() < v2->getHeight();
      }};

  VbkPayloadsRelations& getOrPutVbkRelation(
      const std::shared_ptr<VbkBlock>& block);

  void tryConnectPayloads();

  template <typename T>
  void makePayloadConnected(const std::shared_ptr<T>& t) {
    auto& signal = getSignal<T>();
    auto& inflight = getInFlightMapMut<T>();
    auto& connected = getMapMut<T>();

    auto id = t->getId();
    connected[id] = t;
    inflight.erase(id);
    signal.emit(*t);
  }

  template <typename POP>
  void cleanupStale(std::set<std::shared_ptr<POP>,
                             VbkPayloadsRelations::AtvCombinedComparator>& c,
                    std::function<void(POP&)> remove) {
    for (auto it = c.begin(); it != c.end();) {
      auto& pl = **it;
      ValidationState state;
      auto valid = mempool_tree_.checkContextually(pl, state);
      it = !valid ? (remove(pl), c.erase(it)) : std::next(it);
    }
  }

  template <typename POP>
  void cleanupStale(std::vector<std::shared_ptr<POP>>& c,
                    std::function<void(POP&)> remove) {
    for (auto it = c.begin(); it != c.end();) {
      auto& pl = **it;
      ValidationState state;
      auto valid = mempool_tree_.checkContextually(pl, state);
      it = !valid ? (remove(pl), c.erase(it)) : std::next(it);
    }
  }

  template <typename POP>
  void cleanupStale(payload_value_sorted_map<POP>& c) {
    for (auto it = c.begin(); it != c.end();) {
      auto& pl = *it->second;
      ValidationState state;
      auto valid = mempool_tree_.checkContextually(pl, state);
      it = !valid ? c.erase(it) : std::next(it);
    }
  }

  template <typename Pop>
  signals::Signal<void(const Pop&)>& getSignal() {
    static_assert(sizeof(Pop) == 0, "Unknown type in getSignal");
  }

  //! @private
  template <typename T>
  payload_map<T>& getMapMut() {
    return const_cast<payload_map<T>&>(this->getMap<T>());
  }

  //! @private
  template <typename T>
  payload_value_sorted_map<T>& getInFlightMapMut() {
    return const_cast<payload_value_sorted_map<T>&>(this->getInFlightMap<T>());
  }
};

// clang-format off
//! @overload
template <> MemPool::SubmitResult MemPool::submit<ATV>(const std::shared_ptr<ATV>& atv, bool doIsBlockOldCheck, ValidationState& state);
//! @overload
template <> MemPool::SubmitResult MemPool::submit<VTB>(const std::shared_ptr<VTB>& vtb, bool doIsBlockOldCheck, ValidationState& state);
//! @overload
template <> MemPool::SubmitResult MemPool::submit<VbkBlock>(const std::shared_ptr<VbkBlock>& block, bool doIsBlockOldCheck, ValidationState& state);
//! @overload
template <> const MemPool::payload_map<VbkBlock>& MemPool::getMap() const;
//! @overload
template <> const MemPool::payload_map<ATV>& MemPool::getMap() const;
//! @overload
template <> const MemPool::payload_map<VTB>& MemPool::getMap() const;
//! @overload
template<> const MemPool::payload_value_sorted_map<VbkBlock>& MemPool::getInFlightMap() const;
//! @overload
template<> const MemPool::payload_value_sorted_map<ATV>& MemPool::getInFlightMap() const;
//! @overload
template<> const MemPool::payload_value_sorted_map<VTB>& MemPool::getInFlightMap() const;
//! @overload
template <> signals::Signal<void(const ATV&)>& MemPool::getSignal();
//! @overload
template <> signals::Signal<void(const VTB&)>& MemPool::getSignal();
//! @overload
template <> signals::Signal<void(const VbkBlock&)>& MemPool::getSignal();
// clang-format on

//! @private
namespace detail {

template <typename Value, typename T>
inline void mapToJson(Value& obj,
                      const MemPool& mp,
                      const std::string& key,
                      bool verbose = false) {
  auto arr = json::makeEmptyArray<Value>();
  for (auto& p : mp.getMap<T>()) {
    if (verbose) {
      json::arrayPushBack(arr, ToJSON<Value>(*p.second));
    } else {
      json::arrayPushBack(arr, ToJSON<Value>(p.first));
    }
  }
  for (auto& p : mp.getInFlightMap<T>()) {
    if (verbose) {
      json::arrayPushBack(arr, ToJSON<Value>(*p.second));
    } else {
      json::arrayPushBack(arr, ToJSON<Value>(p.first));
    }
  }
  json::putKV(obj, key, arr);
}
}  // namespace detail

//! @private
template <typename Value>
Value ToJSON(const MemPool& mp, bool verbose = false) {
  auto obj = json::makeEmptyObject<Value>();

  detail::mapToJson<Value, VbkBlock>(obj, mp, "vbkblocks", verbose);
  detail::mapToJson<Value, ATV>(obj, mp, "atvs", verbose);
  detail::mapToJson<Value, VTB>(obj, mp, "vtbs", verbose);

  return obj;
}

}  // namespace altintegration

#endif  // !

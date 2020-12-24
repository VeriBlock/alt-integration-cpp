// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP
#define ALT_INTEGRATION_VERIBLOCK_MEMPOOL_HPP

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/mempool_block_tree.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/mempool_result.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/signals.hpp"

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
 * @ingroup api
 */
struct MemPool {
  enum Status { ACCEPTED, FAILED_STATELESS, FAILED_STATEFUL };

  struct SubmitResult {
    Status status = ACCEPTED;

    // by default created in valid state
    SubmitResult() = default;

    SubmitResult(bool state) {
      VBK_ASSERT_MSG(
          state, "SubmitResult can be implicitly constructed from bool=true");
    }

    SubmitResult(Status status, bool /* ignore */) : status(status) {}

    operator bool() const { return status == ACCEPTED; }

    bool isFailedStateful() const { return status == FAILED_STATEFUL; }
  };

  using vbk_hash_t = typename VbkBlock::prev_hash_t;

  template <typename Payload>
  using payload_map =
      std::unordered_map<typename Payload::id_t, std::shared_ptr<Payload>>;

  using vbkblock_map_t = payload_map<VbkBlock>;
  using atv_map_t = payload_map<ATV>;
  using vtb_map_t = payload_map<VTB>;
  using relations_map_t = payload_map<VbkPayloadsRelations>;

  ~MemPool() = default;
  MemPool(AltBlockTree& tree) : mempool_tree_(tree) {}

  //! getter for payloads stored in mempool
  //! @ingroup api
  template <typename T>
  const T* get(const typename T::id_t& id) const {
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
   * @return true if payload is valid, false otherwise
   * @ingroup api
   */
  template <typename T>
  SubmitResult submit(Slice<const uint8_t> bytes, ValidationState& state) {
    ReadStream stream(bytes);
    T payload;
    if (!DeserializeFromVbkEncoding(stream, payload, state)) {
      return {FAILED_STATELESS,
              state.Invalid("pop-mempool-submit-deserialize")};
    }

    return submit<T>(payload, state);
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
   * @return true if payload is valid, false otherwise
   * @ingroup api
   */
  template <typename T>
  SubmitResult submit(const T& pl, ValidationState& state) {
    return submit<T>(std::make_shared<T>(pl), state);
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
   * @return true if payload is valid, false otherwise
   * @ingroup api
   */
  template <typename T>
  SubmitResult submit(const std::shared_ptr<T>& pl, ValidationState& state) {
    (void)pl;
    (void)state;
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::submit");
    return {};
  }

  /**
   * Shortcut to submit altintegration::PopData as whole thing.
   *
   * @param pop altintegration::PopData
   * @return MempoolResult - an entity that can be serialized ToJSON.
   * @ingroup api
   */
  MempoolResult submitAll(const PopData& pop);

  //! @private
  template <typename T>
  const payload_map<T>& getMap() const {
    static_assert(sizeof(T) == 0, "Undefined type used in MemPool::getMap");
  }

  //! @private
  template <typename T>
  const payload_map<T>& getInFlightMap() const {
    static_assert(sizeof(T) == 0,
                  "Undefined type used in MemPool::getInFlightMap");
  }

  /**
   * (POW) Miners should execute this to get POP content for "next block".
   *
   * @warning Expensive operation. It builds virtual VBK Block Tree with
   * payloads stored in mempool, applies them to current AltBlockTree tip. All
   * payloads that can not be connected will remain in mempool. As a result,
   * this method returns altintegration::PopData which contains fully valid and
   * connected payloads. This should be inserted into AltBlock as is.
   *
   * @ingroup api
   * @return statefully valid altintegration::PopData that can be connected to
   * current tip.
   */
  PopData getPop();

  /**
   * Remove payloads from mempool by their IDs.
   *
   * Use it when new block arrives and it contains PopData. Doing this, mempool
   * also executes clean() method.
   * @ingroup api
   * @param[in] popData altintegration::PopData
   */
  void removeAll(const PopData& popData);

  /**
   * Remove paylaods that are statefully invalid anymore (duplicated, staled
   * payloads)
   *@ingroup api
   */
  void cleanUp();

  /**
   * Clear mempool from all payloads.
   * @ingroup api
   */
  void clear();

  /**
   * Subscribe on "accepted" event - fires whenever new payload is added into
   * mempool.
   * @tparam Pop ATV, VTB or VbkBlock
   * @param f callback
   * @return subscription id
   */
  template <typename Pop>
  size_t onAccepted(std::function<void(const Pop& p)> f) {
    auto& sig = getSignal<Pop>();
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
  vbkblock_map_t vbkblocks_;
  atv_map_t stored_atvs_;
  vtb_map_t stored_vtbs_;

  atv_map_t atvs_in_flight_;
  vtb_map_t vtbs_in_flight_;
  vbkblock_map_t vbkblocks_in_flight_;

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
  void cleanupStale(std::vector<std::shared_ptr<POP>>& c,
                    std::function<void(POP&)> remove) {
    for (auto it = c.begin(); it != c.end();) {
      auto& pl = **it;
      ValidationState state;
      auto valid = mempool_tree_.checkContextually(pl, state);
      if (!valid) {
        remove(pl);
        it = c.erase(it);
      } else {
        ++it;
      }
    }
  }

  template <typename POP>
  void cleanupStale(payload_map<POP>& c) {
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
  payload_map<T>& getInFlightMapMut() {
    return const_cast<payload_map<T>&>(this->getInFlightMap<T>());
  }
};

// clang-format off
//! @overload
template <> MemPool::SubmitResult MemPool::submit<ATV>(const std::shared_ptr<ATV>& atv, ValidationState& state);
//! @overload
template <> MemPool::SubmitResult MemPool::submit<VTB>(const std::shared_ptr<VTB>& vtb, ValidationState& state);
//! @overload
template <> MemPool::SubmitResult MemPool::submit<VbkBlock>(const std::shared_ptr<VbkBlock>& block, ValidationState& state);
//! @overload
template <> const MemPool::payload_map<VbkBlock>& MemPool::getMap() const;
//! @overload
template <> const MemPool::payload_map<ATV>& MemPool::getMap() const;
//! @overload
template <> const MemPool::payload_map<VTB>& MemPool::getMap() const;
//! @overload
template<> const MemPool::payload_map<VbkBlock>& MemPool::getInFlightMap() const;
//! @overload
template<> const MemPool::payload_map<ATV>& MemPool::getInFlightMap() const;
//! @overload
template<> const MemPool::payload_map<VTB>& MemPool::getInFlightMap() const;
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
inline void mapToJson(Value& obj, const MemPool& mp, const std::string& key) {
  auto arr = json::makeEmptyArray<Value>();
  for (auto& p : mp.getMap<T>()) {
    json::arrayPushBack(arr, ToJSON<Value>(p.first));
  }
  for (auto& p : mp.getInFlightMap<T>()) {
    json::arrayPushBack(arr, ToJSON<Value>(p.first));
  }
  json::putKV(obj, key, arr);
}
}  // namespace detail

//! @private
template <typename Value>
Value ToJSON(const MemPool& mp) {
  auto obj = json::makeEmptyObject<Value>();

  detail::mapToJson<Value, VbkBlock>(obj, mp, "vbkblocks");
  detail::mapToJson<Value, ATV>(obj, mp, "atvs");
  detail::mapToJson<Value, VTB>(obj, mp, "vtbs");

  return obj;
}

}  // namespace altintegration

#endif  // !

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/storage/blockchain_storage_util.hpp>

namespace altintegration {

template <>
bool loadBlockEndorsements(
    const PopStorage&,
    const typename BlockTree<BtcBlock, BtcChainParams>::index_t& blockIn,
    typename BlockTree<BtcBlock, BtcChainParams>::index_t& blockOut,
    BlockTree<BtcBlock, BtcChainParams>&,
    ValidationState& state) {
  blockOut.refCounter = blockIn.refCounter;
  return state.IsValid();
}

template <>
bool loadBlockEndorsements(const PopStorage& storage,
                           const typename VbkBlockTree::index_t& blockIn,
                           typename VbkBlockTree::index_t& blockOut,
                           VbkBlockTree& tree,
                           ValidationState& state) {
  using endorsement_t = typename VbkBlock::addon_t::endorsement_t;

  blockOut.vtbids = blockIn.vtbids;
  blockOut.refCounter = blockIn.refCounter;

  // load endorsements
  for (const auto& e : blockIn.containingEndorsements) {
    auto endorsement = storage.loadEndorsements<endorsement_t>(e.first);
    auto* endorsed = tree.template getBlockIndex<typename VbkBlock::hash_t>(
        endorsement.endorsedHash);
    if (endorsed == nullptr) {
      return state.Invalid(
          VbkBlock::name() + "-bad-endorsed",
          "Can not find endorsed block: " + HexStr(endorsement.endorsedHash));
    }
    auto endorsementPtr =
        std::make_shared<endorsement_t>(std::move(endorsement));
    blockOut.containingEndorsements.insert(
        std::make_pair(endorsementPtr->getId(), endorsementPtr));
    endorsed->endorsedBy.push_back(endorsementPtr.get());
  }
  return state.IsValid();
}

template <>
bool loadBlockEndorsements(const PopStorage& storage,
                           const typename AltTree::index_t& blockIn,
                           typename AltTree::index_t& blockOut,
                           AltTree& tree,
                           ValidationState& state) {
  using endorsement_t = typename AltBlock::addon_t::endorsement_t;

  blockOut.atvids = blockIn.atvids;
  blockOut.vtbids = blockIn.vtbids;
  blockOut.vbkblockids = blockIn.vbkblockids;

  // load endorsements
  for (const auto& e : blockIn.containingEndorsements) {
    auto endorsement = storage.loadEndorsements<endorsement_t>(e.first);
    auto* endorsed = tree.template getBlockIndex<typename AltBlock::hash_t>(
        endorsement.endorsedHash);
    if (endorsed == nullptr) {
      return state.Invalid(
          AltBlock::name() + "-bad-endorsed",
          "Can not find endorsed block: " + HexStr(endorsement.endorsedHash));
    }
    auto endorsementPtr =
        std::make_shared<endorsement_t>(std::move(endorsement));
    blockOut.containingEndorsements.insert(
        std::make_pair(endorsementPtr->getId(), endorsementPtr));
    endorsed->endorsedBy.push_back(endorsementPtr.get());
  }
  return state.IsValid();
}

}  // namespace altintegration

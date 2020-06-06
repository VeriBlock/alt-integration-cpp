#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <map>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/endorsement_storage.hpp>

namespace altintegration {

class PopStorage {
  using block_btc_t = BlockIndex<BtcBlock>;
  using block_vbk_t = BlockIndex<VbkBlock>;

 public:
  virtual ~PopStorage() = default;
  PopStorage()
      : repoVbk_(std::make_shared<BlockRepositoryInmem<block_vbk_t>>()),
        repoBtc_(std::make_shared<BlockRepositoryInmem<block_btc_t>>()),
        endorsementsAtv_(
            std::make_shared<EndorsementStorage<AltPayloads>>()),
        endorsementsVtb_(
            std::make_shared<EndorsementStorage<VTB>>()) {}

  BlockRepository<block_vbk_t>& vbkIndex() { return *repoVbk_; }
  const BlockRepository<block_vbk_t>& vbkIndex() const { return *repoVbk_; }

  EndorsementStorage<AltPayloads>& altEndorsements() {
    return *endorsementsAtv_;
  }
  const EndorsementStorage<AltPayloads>& altEndorsements() const {
    return *endorsementsAtv_;
  }

    EndorsementStorage<VTB>& vbkEndorsements() {
    return *endorsementsVtb_;
  }
  const EndorsementStorage<VTB>& vbkEndorsements() const {
      return *endorsementsVtb_;
  }

  void saveVbkTree(const VbkBlockTree& tree) {
    vbkTipHeight_ = tree.getBestChain().tip()->height;
    vbkTipHash_ = tree.getBestChain().tip()->getHash();

    auto blocks = tree.getBlocks();
    for (const auto& block : blocks) {
      repoVbk_->put(*(block.second));
    }

    endorsementsVtb_ = std::make_shared<EndorsementStorage<VTB>>(tree.getStorage());
  }

  void saveBtcTree(const BlockTree<BtcBlock, BtcChainParams>& tree) {
    btcTipHeight_ = tree.getBestChain().tip()->height;
    btcTipHash_ = tree.getBestChain().tip()->getHash();

    auto blocks = tree.getBlocks();
    for (const auto& block : blocks) {
      repoBtc_->put(*(block.second));
    }
  }

  void loadVbkTree(VbkBlockTree& tree) {
    loadBtcTree(tree.btc());
    auto cursor = repoVbk_->newCursor();
    cursor->seekToFirst();
    std::multimap<int32_t, std::shared_ptr<block_vbk_t>> blocks;
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.height, std::make_shared<block_vbk_t>(block)});
      cursor->next();
    }

    for (const auto& blockPair : blocks) {
      auto *bi = tree.insertBlock(blockPair.second->header);
      bi->payloadIds = blockPair.second->payloadIds;
      bi->refCounter = blockPair.second->refCounter;

      // restore endorsedBy pointers
      for (const auto& pid : bi->payloadIds) {
        VTB payloads{};
        bool ret = endorsementsVtb_->payloads().get(pid, &payloads);
        if (!ret) continue;

        auto e = BtcEndorsement::fromContainerPtr(payloads);
        auto *endorsed = tree.getBlockIndex(payloads.getEndorsedBlock().getHash());
        endorsed->endorsedBy.push_back(e);
      }
    }

    ValidationState state{};
    auto* tip = tree.getBlockIndex(vbkTipHash_);
    tree.setState(*tip, state, true);
  }

  void loadBtcTree(BlockTree<BtcBlock, BtcChainParams>& tree) {
    auto cursor = repoBtc_->newCursor();
    cursor->seekToFirst();
    std::multimap<int32_t, std::shared_ptr<block_btc_t>> blocks;
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.height, std::make_shared<block_btc_t>(block)});
      cursor->next();
    }

    for (const auto& blockPair : blocks) {
      auto* bi = tree.insertBlock(blockPair.second->header);
      bi->payloadIds = blockPair.second->payloadIds;
      bi->refCounter = blockPair.second->refCounter;
    }

    ValidationState state{};
    auto* tip = tree.getBlockIndex(btcTipHash_);
    tree.setState(*tip, state);
  }

 private:
  std::shared_ptr<BlockRepository<block_vbk_t>> repoVbk_;
  int vbkTipHeight_;
  typename VbkBlock::hash_t vbkTipHash_;
  std::shared_ptr<BlockRepository<block_btc_t>> repoBtc_;
  int btcTipHeight_;
  typename BtcBlock::hash_t btcTipHash_;
  std::shared_ptr<EndorsementStorage<AltPayloads>> endorsementsAtv_;
  std::shared_ptr<EndorsementStorage<VTB>> endorsementsVtb_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
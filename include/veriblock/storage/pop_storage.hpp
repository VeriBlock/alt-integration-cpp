#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <map>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/endorsement_storage.hpp>

namespace altintegration {

struct StoredBtcBlock : BtcBlock {
  StoredBtcBlock() = default;
  StoredBtcBlock(const BtcBlock& b) : BtcBlock(b) {}

  int32_t height;
};

class PopStorage {
 public:
  virtual ~PopStorage() = default;
  PopStorage()
      : repoVbk_(std::move(std::make_shared<BlockRepositoryInmem<VbkBlock>>())),
        repoBtc_(std::move(
            std::make_shared<BlockRepositoryInmem<StoredBtcBlock>>())),
        endorsementsAtv_(
            std::move(std::make_shared<EndorsementStorage<AltPayloads, AltTree>>())),
        endorsementsVtb_(
            std::move(std::make_shared<EndorsementStorage<VTB, VbkBlockTree>>())) {}

  BlockRepository<VbkBlock>& vbkIndex() { return *repoVbk_; }
  const BlockRepository<VbkBlock>& vbkIndex() const { return *repoVbk_; }

  EndorsementStorage<AltPayloads, AltTree>& altEndorsements() {
    return *endorsementsAtv_;
  }
  const EndorsementStorage<AltPayloads, AltTree>& altEndorsements() const {
    return *endorsementsAtv_;
  }

  void saveVbkTree(const VbkBlockTree& tree) {
    vbkTipHeight_ = tree.getBestChain().tip()->height;
    vbkTipHash_ = tree.getBestChain().tip()->getHash();

    auto blocks = tree.getBlocks();
    for (const auto& block : blocks) {
      repoVbk_->put(*(block.second->header));
    }
  }

  void saveBtcTree(const BlockTree<BtcBlock, BtcChainParams>& tree) {
    btcTipHeight_ = tree.getBestChain().tip()->height;
    btcTipHash_ = tree.getBestChain().tip()->getHash();

    auto blocks = tree.getBlocks();
    for (const auto& block : blocks) {
      StoredBtcBlock blockWithHeight{*(block.second->header)};
      blockWithHeight.height = block.second->height;
      repoBtc_->put(blockWithHeight);
    }
  }

  void loadVbkTree(VbkBlockTree& tree) {
    auto cursor = repoVbk_->newCursor();
    cursor->seekToFirst();
    std::multimap<int32_t, std::shared_ptr<VbkBlock>> blocks;
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.height, std::make_shared<VbkBlock>(block)});
      cursor->next();
    }

    for (const auto& blockPair : blocks) {
      BlockIndex<VbkBlock> bi;
      bi.header = blockPair.second;
      bi.height = blockPair.first;
      bi.pprev = tree.getBlockIndex(bi.header->previousBlock);
      tree.blocks_[blockPair.second->getShortHash()] =
          std::make_shared<BlockIndex<VbkBlock>>(std::move(bi));
    }
  }

  void loadBtcTree(BlockTree<BtcBlock, BtcChainParams>& tree) {
    auto cursor = repoBtc_->newCursor();
    cursor->seekToFirst();
    std::multimap<int32_t, std::shared_ptr<BtcBlock>> blocks;
    while (cursor->isValid()) {
      auto block = cursor->value();
      blocks.insert({block.height, std::make_shared<BtcBlock>(block)});
      cursor->next();
    }

    BlockIndex<BtcBlock> bi;
    for (const auto& blockPair : blocks) {
      tree.doInsertBlockHeader(blockPair.second);
    }

    ValidationState state{};
    auto* tip = tree.getBlockIndex(btcTipHash_);
    tree.setTip(*tip, state, false);
  }

 private:
  std::shared_ptr<BlockRepository<VbkBlock>> repoVbk_;
  int vbkTipHeight_;
  typename VbkBlock::hash_t vbkTipHash_;
  std::shared_ptr<BlockRepository<StoredBtcBlock>> repoBtc_;
  int btcTipHeight_;
  typename BtcBlock::hash_t btcTipHash_;
  std::shared_ptr<EndorsementStorage<AltPayloads, AltTree>> endorsementsAtv_;
  std::shared_ptr<EndorsementStorage<VTB, VbkBlockTree>> endorsementsVtb_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
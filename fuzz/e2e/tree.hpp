// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_TREE_HPP
#define VERIBLOCK_POP_CPP_TREE_HPP

#include <veriblock/pop.hpp>

namespace fuzz {

struct Block {
  std::vector<uint8_t> hash;
  std::vector<uint8_t> prevhash;
  int height = 0;
  int timestamp = altintegration::currentTimestamp4();
  altintegration::PopData popdata;

  altintegration::AltBlock toAltBlock() const {
    altintegration::AltBlock block;
    block.hash = hash;
    block.previousBlock = prevhash;
    block.timestamp = timestamp;
    block.height = height;
    return block;
  }
};

extern Block genesisBlock;

struct FuzzAltChainParams : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept override { return 0x1337; }

  altintegration::AltBlock getBootstrapBlock() const noexcept override {
    return genesisBlock.toAltBlock();
  }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    // endorsed header == endorsed hash
    return bytes;
  }

  bool checkBlockHeader(
      const std::vector<uint8_t>& bytes,
      const std::vector<uint8_t>& root,
      altintegration::ValidationState& state) const noexcept override {
    (void)state;
    (void)bytes;
    (void)root;
    return true;
  }
};

class Tree {
 public:
  Tree();

  bool acceptBlock(const Block& block);

  Block* getBlock(const std::vector<uint8_t>& hash);

  altintegration::BtcBlock lastBtc() const {
    return popcontext->getBtcBlockTree().getBestChain().tip()->getHeader();
  }

  altintegration::VbkBlock lastVbk() const {
    return popcontext->getVbkBlockTree().getBestChain().tip()->getHeader();
  }

  std::string toPrettyString() const {
    std::string s;
    std::vector<Block*> b;
    for (auto& it : blocks) {
      b.push_back(it.second.get());
    }
    std::sort(b.begin(), b.end(), [](const Block* a, const Block* b) {
      return a->height < b->height;
    });

    for (auto& p : b) {
      s += fmt::format("Block(height={} hash={})\n",
                       p->height,
                       altintegration::HexStr(p->hash));
    }

    return s;
  }

  std::vector<uint8_t> bestBlock;
  altintegration::adaptors::InmemStorageImpl storage{};
  std::shared_ptr<altintegration::adaptors::PayloadsStorageImpl> pp = nullptr;
  std::shared_ptr<altintegration::AltChainParams> params = nullptr;
  std::shared_ptr<altintegration::PopContext> popcontext = nullptr;
  std::unordered_map<std::vector<uint8_t>, std::shared_ptr<Block>> blocks;
};

}  // namespace fuzz

#endif  // VERIBLOCK_POP_CPP_TREE_HPP

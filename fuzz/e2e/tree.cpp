// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "tree.hpp"

namespace fuzz {

Block genesisBlock = []() -> Block {
  Block b;
  b.timestamp = 0;
  b.hash = {1, 2, 3, 4};
  b.height = 0;
  return b;
}();

bool Tree::acceptBlock(const Block& block) {
  if (block.prevhash == block.hash) {
    // bad hash
    return false;
  }

  auto* prev = getBlock(block.prevhash);
  if (prev == nullptr) {
    // can't connect
    return false;
  }

  auto altheader = block.toAltBlock();
  using altintegration::ValidationState;
  ValidationState state;
  if (!popcontext->getAltBlockTree().acceptBlockHeader(altheader, state)) {
    return false;
  }

  auto* bestIndex = popcontext->getAltBlockTree().getBlockIndex(bestBlock);
  VBK_ASSERT(bestIndex);
  VBK_ASSERT(bestIndex->isConnected());

  popcontext->getAltBlockTree().acceptBlock(altheader.hash, block.popdata);

  blocks[block.hash] = std::make_shared<Block>(block);

  auto* index = popcontext->getAltBlockTree().getBlockIndex(altheader.hash);
  VBK_ASSERT(index);
  if (!index->isConnected()) {
    // do not invoke POP FR if index is not connected
    return true;
  }

  // try POP FR
  bool success = popcontext->getAltBlockTree().setState(*bestIndex, state);
  VBK_ASSERT(success);

  int result = popcontext->getAltBlockTree().activateBestChain(
      bestIndex->getHash(), index->getHash());
  if (result < 0) {
    bestBlock = index->getHash();
  } else if (result == 0) {
    if (bestIndex->getHeight() < index->getHeight()) {
      bestBlock = index->getHash();
    }
  } else {
    // bestIndex is still best
  }

  success = popcontext->getAltBlockTree().setState(*bestIndex, state);
  VBK_ASSERT(success);

  return true;
}

Block* Tree::getBlock(const std::vector<uint8_t>& hash) {
  auto it = blocks.find(hash);
  if (it == blocks.end()) {
    return nullptr;
  }

  return it->second.get();
}

Tree::Tree() {
  // bootstrap with genesis block
  blocks[genesisBlock.hash] = std::make_shared<Block>(genesisBlock);
  bestBlock = genesisBlock.hash;
  params = std::make_shared<FuzzAltChainParams>();
  pp = std::make_shared<altintegration::adaptors::PayloadsStorageImpl>(storage);
  bp = std::make_shared<altintegration::adaptors::BlockReaderImpl>(storage, *params);

  auto config = std::make_shared<altintegration::Config>();
  config->SelectBtcParams("regtest", 0, {});
  config->SelectVbkParams("regtest", 0, {});
  config->SelectAltParams(params);

  popcontext = altintegration::PopContext::create(config, pp, bp);
}
}  // namespace fuzz
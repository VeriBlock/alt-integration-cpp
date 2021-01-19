// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "tree.hpp"

namespace fuzz {

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
  if (!popcontext->altTree->acceptBlockHeader(altheader, state)) {
    return false;
  }

  auto* bestIndex = popcontext->altTree->getBlockIndex(bestBlock);
  VBK_ASSERT(bestIndex);
  VBK_ASSERT(bestIndex->isConnected());

  popcontext->altTree->acceptBlock(altheader.hash, block.popdata);

  auto* index = popcontext->altTree->getBlockIndex(altheader.hash);
  VBK_ASSERT(index);
  if (!index->isConnected()) {
    // do not invoke POP FR if index is not connected
    return true;
  }

  // try POP FR
  bool success = popcontext->altTree->setState(*bestIndex, state);
  VBK_ASSERT(success);

  int result = popcontext->altTree->comparePopScore(bestIndex->getHash(),
                                                    index->getHash());
  if (result < 0) {
    bestBlock = index->getHash();
  } else if (result == 0) {
    if (bestIndex->getHeight() < index->getHeight()) {
      bestBlock = index->getHash();
    }
  } else {
    // bestIndex is still best
  }

  success = popcontext->altTree->setState(*bestIndex, state);
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
  pp = std::make_shared<altintegration::InmemPayloadsProvider>();

  auto config = std::make_shared<altintegration::Config>();
  config->SelectBtcParams("regtest", 0, {});
  config->SelectVbkParams("regtest", 0, {});
  config->SelectAltParams(params);

  popcontext = altintegration::PopContext::create(config, pp);
}
}  // namespace fuzz
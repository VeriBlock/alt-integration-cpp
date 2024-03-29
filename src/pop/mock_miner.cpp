// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/mock_miner.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "veriblock/pop/arith_uint256.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/blob.hpp"
#include "veriblock/pop/blockchain/block_index.hpp"
#include "veriblock/pop/blockchain/chain.hpp"
#include "veriblock/pop/blockchain/miner.hpp"
#include "veriblock/pop/blockchain/vbk_chain_params.hpp"
#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/crypto/secp256k1.hpp"
#include "veriblock/pop/entities/address.hpp"
#include "veriblock/pop/entities/atv.hpp"
#include "veriblock/pop/entities/btcblock.hpp"
#include "veriblock/pop/entities/btctx.hpp"
#include "veriblock/pop/entities/coin.hpp"
#include "veriblock/pop/entities/merkle_path.hpp"
#include "veriblock/pop/entities/merkle_tree.hpp"
#include "veriblock/pop/entities/network_byte_pair.hpp"
#include "veriblock/pop/entities/popdata.hpp"
#include "veriblock/pop/entities/publication_data.hpp"
#include "veriblock/pop/entities/vbk_merkle_path.hpp"
#include "veriblock/pop/entities/vbkblock.hpp"
#include "veriblock/pop/entities/vbkpoptx.hpp"
#include "veriblock/pop/entities/vbktx.hpp"
#include "veriblock/pop/entities/vtb.hpp"
#include "veriblock/pop/storage/adaptors/payloads_provider_impl.hpp"
#include "veriblock/pop/strutil.hpp"
#include "veriblock/pop/third_party/Signals.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

namespace altintegration {

// NOLINTNEXTLINE(cert-err58-cpp)
static auto defaultPrivateKeyVbk = ParseHex(
    "303e020100301006072a8648ce3d020106052b8104000a0427302502010104203abf83fa47"
    "0423d4788a760ef6b7aae1dacf98784b0646057a0adca24e522acb");

// NOLINTNEXTLINE(cert-err58-cpp)
static auto defaultPublicKeyVbk = ParseHex(
    "3056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099"
    "ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a8061"
    "10fa05d9014a102a0513dd354ec5");

PopData MockMiner::endorseAltBlock(
    const PublicationData& publicationData,
    const VbkBlock::hash_t& lastKnownVbkBlockHash) {
  const auto& tx = createVbkTxEndorsingAltBlock(publicationData);
  const auto& block = mineVbkBlocks(1, {tx})->getHeader();
  return createPopDataEndorsingAltBlock(block, tx, lastKnownVbkBlockHash);
}

VTB MockMiner::endorseVbkBlock(const VbkBlock& publishedBlock,
                               const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  const auto& tx =
      createVbkPopTxEndorsingVbkBlock(publishedBlock, lastKnownBtcBlockHash);
  const auto& block = mineVbkBlocks(1, {tx});
  return createVTB(block->getHeader(), tx);
}

VbkPopTx MockMiner::createVbkPopTxEndorsingVbkBlock(
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  const auto& tx = createBtcTxEndorsingVbkBlock(publishedBlock);
  const auto& block = mineBtcBlocks(1, {tx})->getHeader();
  return createVbkPopTxEndorsingVbkBlock(
      block, tx, publishedBlock, lastKnownBtcBlockHash);
}

PopData MockMiner::createPopDataEndorsingAltBlock(
    const VbkBlock& blockOfProof,
    const VbkTx& transaction,
    const VbkBlock::hash_t& lastKnownVbkBlockHash) const {
  PopData data;
  data.atvs = {createATV(blockOfProof, transaction)};
  data.context = getBlocks(vbk_tree_, blockOfProof, lastKnownVbkBlockHash);
  for (const VbkBlock& b : data.context) {
    const auto& vtbs = getVTBs(b);
    data.vtbs.insert(data.vtbs.end(), vtbs.begin(), vtbs.end());
  }
  return data;
}

ATV MockMiner::createATV(const VbkBlock& blockOfProof,
                         const VbkTx& transaction) const {
  ATV atv;
  atv.transaction = transaction;
  atv.merklePath = getMerklePath(
      blockOfProof, transaction.getHash(), VbkMerkleTree::TreeIndex::NORMAL);
  atv.blockOfProof = blockOfProof;
  return atv;
}

VbkTx MockMiner::createVbkTxEndorsingAltBlockWithSourceAmount(
    const PublicationData& publicationData, const Coin& sourceAmount) const {
  VbkTx tx;
  tx.signatureIndex = 7;
  tx.networkOrType.networkType = vbk_params_.getTransactionMagicByte();
  tx.networkOrType.typeId = (uint8_t)TxType::VBK_TX;
  tx.sourceAmount = sourceAmount;
  tx.sourceAddress = Address::fromPublicKey(defaultPublicKeyVbk);
  tx.publicKey = defaultPublicKeyVbk;
  tx.publicationData = publicationData;

  auto hash = tx.getHash();
  tx.signature =
      secp256k1::sign(hash, secp256k1::privateKeyFromVbk(defaultPrivateKeyVbk));

  return tx;
}

VbkTx MockMiner::createVbkTxEndorsingAltBlock(
    const PublicationData& publicationData) const {
  return createVbkTxEndorsingAltBlockWithSourceAmount(publicationData,
                                                      Coin(1000));
}

VTB MockMiner::createVTB(const VbkBlock& containingBlock,
                         const VbkPopTx& transaction) const {
  VTB vtb;
  vtb.transaction = transaction;
  vtb.merklePath = getMerklePath(
      containingBlock, transaction.getHash(), VbkMerkleTree::TreeIndex::POP);
  vtb.containingBlock = containingBlock;
  return vtb;
}

VbkPopTx MockMiner::createVbkPopTxEndorsingVbkBlock(
    const BtcBlock& blockOfProof,
    const BtcTx& transaction,
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) const {
  VbkPopTx popTx;
  popTx.networkOrType.networkType = vbk_params_.getTransactionMagicByte();
  popTx.networkOrType.typeId = (uint8_t)TxType::VBK_POP_TX;
  popTx.address = Address::fromPublicKey(defaultPublicKeyVbk);
  popTx.publishedBlock = publishedBlock;
  popTx.blockOfProof = blockOfProof;
  popTx.publicKey = defaultPublicKeyVbk;
  popTx.bitcoinTransaction = transaction;
  popTx.merklePath = getMerklePath(blockOfProof, transaction.getHash());
  popTx.blockOfProofContext =
      getBlocks(btc_tree_, blockOfProof, lastKnownBtcBlockHash);

  auto hash = popTx.getHash();
  popTx.signature =
      secp256k1::sign(hash, secp256k1::privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

BtcTx MockMiner::createBtcTxEndorsingVbkBlock(
    const VbkBlock& publishedBlock) const {
  WriteStream stream;
  publishedBlock.toRaw(stream);
  const auto addr = Address::fromPublicKey(defaultPublicKeyVbk);
  addr.getPopBytes(stream);
  auto tx = BtcTx(stream.data());
  return tx;
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(const size_t amount) {
  const auto* tip = vbkTip();
  VBK_ASSERT(tip != nullptr);
  return mineBlocks(amount, *tip, {}, {});
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(const size_t amount,
                                               const std::vector<VbkTx>& txs) {
  const auto* tip = vbkTip();
  VBK_ASSERT(tip != nullptr);
  return mineBlocks(amount, *tip, txs, {});
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(
    const size_t amount, const std::vector<VbkPopTx>& popTxs) {
  const auto* tip = vbkTip();
  VBK_ASSERT(tip != nullptr);
  return mineBlocks(amount, *tip, {}, popTxs);
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(
    const size_t amount, const BlockIndex<VbkBlock>& tip) {
  return mineBlocks(amount, tip, {}, {});
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(const size_t amount,
                                               const BlockIndex<VbkBlock>& tip,
                                               const std::vector<VbkTx>& txs) {
  return mineBlocks(amount, tip, txs, {});
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(
    const size_t amount,
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkPopTx>& popTxs) {
  return mineBlocks(amount, tip, {}, popTxs);
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(
    const size_t amount,
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkTx>& txs,
    const std::vector<VbkPopTx>& popTxs) {
  return mineBlocks(amount, tip, txs, popTxs);
}

BlockIndex<BtcBlock>* MockMiner::mineBtcBlocks(const size_t amount,
                                               const std::vector<BtcTx>& txs) {
  return mineBlocks(amount, *btcTip(), txs);
}

BlockIndex<BtcBlock>* MockMiner::mineBtcBlocks(const size_t amount,
                                               const BlockIndex<BtcBlock>& tip,
                                               const std::vector<BtcTx>& txs) {
  return mineBlocks(amount, tip, txs);
}

const BlockIndex<VbkBlock>* MockMiner::vbkTip() const {
  const auto* tip = vbk_tree_.getBestChain().tip();
  VBK_ASSERT_MSG(tip != nullptr,
                 "VBK tip is undefined (blockchain may be not bootstrapped)");
  return tip;
}

const BlockIndex<BtcBlock>* MockMiner::btcTip() const {
  const auto* tip = btc_tree_.getBestChain().tip();
  VBK_ASSERT_MSG(tip != nullptr,
                 "BTC tip is undefined (blockchain may be not bootstrapped)");
  return tip;
}

const BlockIndex<VbkBlock>* MockMiner::getVbkBlockIndex(
    const VbkBlock::hash_t& hash) const {
  const auto* index = vbk_tree_.getBlockIndex(hash);
  VBK_ASSERT_MSG(
      index != nullptr,
      "MockMiner: can't find prev block for mining: " + hash.toHex());
  return index;
}

const BlockIndex<BtcBlock>* MockMiner::getBtcBlockIndex(
    const BtcBlock::hash_t& hash) const {
  const auto* index = btc_tree_.getBlockIndex(hash);
  VBK_ASSERT_MSG(
      index != nullptr,
      "MockMiner: can't find prev block for mining: " + hash.toHex());
  return index;
}

std::vector<VTB> MockMiner::getVTBs(const VbkBlock& block) const {
  const auto it = vtbs_.find(block.getHash());
  if (it == vtbs_.end()) {
    return {};
  }
  return it->second;
}

// private

// (lastKnown, tip]
template <typename BlockTree, typename Block>
std::vector<Block> MockMiner::getBlocks(
    BlockTree& tree,
    const Block& tip,
    const typename Block::hash_t& lastKnownHash) {
  auto* tipIndex = tree.getBlockIndex(tip.getHash());
  VBK_ASSERT(tipIndex != nullptr);
  auto lastKnownIndex = tree.getBlockIndex(lastKnownHash);
  VBK_ASSERT(lastKnownIndex != nullptr);
  std::vector<Block> blocks;
  for (auto* block = tipIndex->pprev;
       block != nullptr && block->getHash() != lastKnownHash;
       block = block->pprev) {
    const auto& header = block->getHeader();
    blocks.push_back(header);
  }
  std::reverse(blocks.begin(), blocks.end());
  return blocks;
}

template <typename BlockTree, typename Block>
BlockIndex<Block>* MockMiner::acceptBlock(BlockTree& tree, const Block& block) {
  ValidationState state;
  if (!tree.acceptBlockHeader(block, state)) {
    return nullptr;
  }
  auto* index = tree.getBlockIndex(block.getHash());
  VBK_ASSERT_MSG(index != nullptr,
                 "could not find the block we have just added");
  index->addRef(0);
  return index;
}

BlockIndex<VbkBlock>* MockMiner::mineBlocks(
    size_t amount,
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkTx>& txs,
    const std::vector<VbkPopTx>& popTxs) {
  VBK_ASSERT(amount > 0);
  auto* index = mineBlock(tip, txs, popTxs);
  for (size_t i = 1; i < amount; i++) {
    index = mineBlock(*index, {}, {});
  }
  return index;
}

BlockIndex<BtcBlock>* MockMiner::mineBlocks(size_t amount,
                                            const BlockIndex<BtcBlock>& tip,
                                            const std::vector<BtcTx>& txs) {
  VBK_ASSERT(amount > 0);
  auto* index = mineBlock(tip, txs);
  for (size_t i = 1; i < amount; i++) {
    index = mineBlock(*index, {});
  }
  return index;
}

BlockIndex<VbkBlock>* MockMiner::mineBlock(const BlockIndex<VbkBlock>& tip,
                                           const std::vector<VbkTx>& txs) {
  if (txs.empty()) {
    VbkBlock block = vbk_miner_.createNextBlock(tip);
    return acceptBlock(vbk_tree_, block);
  }

  VbkMerkleTree merkleTree(hashAll(txs), {});
  const auto& merkleRoot =
      merkleTree.getMerkleRoot().template trim<VBK_MERKLE_ROOT_HASH_SIZE>();

  VbkBlock block = vbk_miner_.createNextBlock(tip, merkleRoot);
  BlockIndex<VbkBlock>* blockIndex = acceptBlock(vbk_tree_, block);

  vbk_merkle_trees_.insert({block.getHash(), merkleTree});

  return blockIndex;
}

BlockIndex<VbkBlock>* MockMiner::mineBlock(const BlockIndex<VbkBlock>& tip,
                                           const std::vector<VbkPopTx>& txs) {
  if (txs.empty()) {
    VbkBlock block = vbk_miner_.createNextBlock(tip);
    return acceptBlock(vbk_tree_, block);
  }

  VbkMerkleTree merkleTree({}, hashAll(txs));
  const auto& merkleRoot =
      merkleTree.getMerkleRoot().template trim<VBK_MERKLE_ROOT_HASH_SIZE>();

  VbkBlock block = vbk_miner_.createNextBlock(tip, merkleRoot);
  BlockIndex<VbkBlock>* blockIndex = acceptBlock(vbk_tree_, block);

  vbk_merkle_trees_.insert({block.getHash(), merkleTree});
  if (!saveVTBs(blockIndex, txs)) {
    return nullptr;
  }

  return blockIndex;
}

BlockIndex<VbkBlock>* MockMiner::mineBlock(
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkTx>& txs,
    const std::vector<VbkPopTx>& popTxs) {
  if (txs.empty() && popTxs.empty()) {
    VbkBlock block = vbk_miner_.createNextBlock(tip);
    return acceptBlock(vbk_tree_, block);
  }

  auto txs_hash = hashAll(txs);
  auto popTxs_hash = hashAll(popTxs);

  VbkMerkleTree merkleTree(hashAll(txs), hashAll(popTxs));
  const auto& merkleRoot =
      merkleTree.getMerkleRoot().template trim<VBK_MERKLE_ROOT_HASH_SIZE>();

  VbkBlock block = vbk_miner_.createNextBlock(tip, merkleRoot);
  BlockIndex<VbkBlock>* blockIndex = acceptBlock(vbk_tree_, block);

  vbk_merkle_trees_.insert({block.getHash(), merkleTree});
  if (!saveVTBs(blockIndex, popTxs)) {
    return nullptr;
  }

  return blockIndex;
}

BlockIndex<BtcBlock>* MockMiner::mineBlock(const BlockIndex<BtcBlock>& tip,
                                           const std::vector<BtcTx>& txs) {
  if (txs.empty()) {
    BtcBlock block = btc_miner_.createNextBlock(tip);
    return acceptBlock(btc_tree_, block);
  }

  BtcMerkleTree merkleTree(hashAll(txs));
  const auto& merkleRoot = merkleTree.getMerkleRoot();

  BtcBlock block = btc_miner_.createNextBlock(tip, merkleRoot);
  BlockIndex<BtcBlock>* blockIndex = acceptBlock(btc_tree_, block);

  btc_merkle_trees_.insert({block.getHash(), merkleTree});

  return blockIndex;
}

bool MockMiner::saveVTBs(BlockIndex<VbkBlock>* blockIndex,
                         const std::vector<VbkPopTx>& txs) {
  std::vector<VTB> vtbs(txs.size());
  for (size_t i = 0; i < vtbs.size(); i++) {
    vtbs[i] = createVTB(blockIndex->getHeader(), txs[i]);
  }

  PopData popData;
  popData.vtbs = vtbs;
  payloads_provider_.writePayloads(popData);
  const auto hash = blockIndex->getHash();
  ValidationState state;
  if (!vbk_tree_.addPayloads(hash, vtbs, state)) {
    blockIndex->removeRef(0);
    vbk_tree_.removeLeaf(*blockIndex);
    return false;
    //    throw std::domain_error(state.toString());
  }

  vtbs_.insert({hash, vtbs});

  return true;
}

VbkMerklePath MockMiner::getMerklePath(
    const VbkBlock& block,
    const uint256& txHash,
    VbkMerkleTree::TreeIndex treeIndex) const {
  auto it = vbk_merkle_trees_.find(block.getHash());
  VBK_ASSERT(it != vbk_merkle_trees_.end());
  const auto& merkleTree = it->second;
  return merkleTree.getMerklePath(txHash, treeIndex);
}

MerklePath MockMiner::getMerklePath(const BtcBlock& block,
                                    const uint256& txHash) const {
  auto it = btc_merkle_trees_.find(block.getHash());
  VBK_ASSERT(it != btc_merkle_trees_.end());
  const auto& merkleTree = it->second;
  return merkleTree.getMerklePath(txHash);
}

const std::unordered_map<VbkBlock::hash_t, std::vector<VTB>>&
MockMiner::getAllVTBs() const {
  return vtbs_;
}

}  // namespace altintegration

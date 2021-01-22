// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdexcept>

#include "veriblock/crypto/secp256k1.hpp"
#include "veriblock/entities/address.hpp"
#include "veriblock/mock_miner_2.hpp"
#include "veriblock/strutil.hpp"

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

// wrappers that prevent the block from being deleted
template <typename BlockTree, typename Block>
bool acceptBlock(BlockTree& tree, const Block& block, ValidationState& state) {
  if (!tree.acceptBlock(block, state)) {
    return false;
  }
  auto* index = tree.getBlockIndex(block.getHash());
  VBK_ASSERT(index != nullptr && "could not find the block we have just added");
  index->addRef(0);
  return true;
}

inline static void Check(bool condition, const std::string& message) {
  if (!condition) {
    throw std::domain_error(message);
  }
}

uint256 calculateMerkleRoot(const std::vector<BtcTx>& transactions) {
  const std::vector<BtcTx::hash_t>& hashes = hashAll<BtcTx>(transactions);
  BtcMerkleTree merkleTree(hashes);
  return merkleTree.getMerkleRoot();
}

uint128 calculateMerkleRoot(const std::vector<VbkPopTx>& transactions) {
  std::vector<VbkPopTx::hash_t> hashes = hashAll<VbkPopTx>(transactions);
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree merkleTree(hashes, treeIndex);
  return merkleTree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>();
}

uint128 calculateMerkleRoot(const std::vector<VbkTx>& transactions) {
  std::vector<VbkTx::hash_t> hashes = hashAll<VbkTx>(transactions);
  const int32_t treeIndex = 1;  // this is NORMAL tx
  VbkMerkleTree merkleTree(hashes, treeIndex);
  return merkleTree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>();
}

template <typename Miner, typename BlockTree, typename Block, typename Tx>
BlockIndex<Block>* mineBlock(Miner& miner,
                             BlockTree& tree,
                             const BlockIndex<Block>& tip,
                             const std::vector<Tx>& transactions) {
  Block block;
  if (transactions.empty()) {
    block = miner.createNextBlock(tip);
  } else {
    const auto& merkleRoot = calculateMerkleRoot(transactions);
    block = miner.createNextBlock(tip, merkleRoot);
  }
  ValidationState state;
  Check(acceptBlock(tree, block, state), state.toString());
  BlockIndex<Block>* index = tree.getBlockIndex(block.getHash());
  Check(index != nullptr, "Index not found for " + block.toPrettyString());
  return index;
}

VbkPopTx MockMiner2::endorseVbkBlock(
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  BtcTx containingTx = createBtcTxEndorsingVbkBlock(publishedBlock);
  BlockIndex<BtcBlock>* containingBlockIndex = mineBtcBlocks(1, {containingTx});
  VbkPopTx popTx = createVbkPopTxEndorsingVbkBlock(
      containingBlockIndex->getHeader(), containingTx,
      publishedBlock, lastKnownBtcBlockHash);
  return popTx;
}

BtcTx MockMiner2::createBtcTxEndorsingVbkBlock(
    const VbkBlock& publishedBlock) {
  WriteStream stream;
  publishedBlock.toRaw(stream);
  auto addr = Address::fromPublicKey(defaultPublicKeyVbk);
  addr.getPopBytes(stream);
  auto tx = BtcTx(stream.data());
  btcmempool.push_back(tx);
  return tx;
}

VbkPopTx MockMiner2::createVbkPopTxEndorsingVbkBlock(
    const BtcBlock& containingBlock,
    const BtcTx& containingTx,
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  const auto& btc = vbktree.btc();
  auto containingBlockIndex = btc.getBlockIndex(containingBlock.getHash());
  Check(containingBlockIndex != nullptr,
        "containing block with hash " +
        containingBlock.getHash().toHex() +
        " does not exist in BTC ");

  VbkPopTx popTx;
  popTx.networkOrType.networkType = vbk_params.getTransactionMagicByte();
  popTx.networkOrType.typeId = (uint8_t)TxType::VBK_POP_TX;
  popTx.address = Address::fromPublicKey(defaultPublicKeyVbk);
  popTx.publishedBlock = publishedBlock;
  popTx.blockOfProof = containingBlock;
  popTx.publicKey = defaultPublicKeyVbk;

  // search for transactions in containing block
  auto btcit = btcTxs.find(containingBlock.getHash());
  Check(btcit != btcTxs.end(),
        "in attempt to create VBK pop TX endorsing VBK block " +
        publishedBlock.getHash().toHex() +
        " we tried to search for BTC block " +
        containingBlock.getHash().toHex() + " but were unable to find it");

  // search for containing tx
  auto& txes = btcit->second;
  auto txit = std::find(txes.begin(), txes.end(), containingTx);
  Check(txit != txes.end(),
        "in attempt to create VBK pop TX endorsing VBK block " +
        publishedBlock.getHash().toHex() +
        " we tried to search for BTC TX containing that block in BTC block " +
        containingBlock.getHash().toHex() + " but were unable to find it");

  auto txhashes = hashAll<BtcTx>(txes);
  BtcMerkleTree mtree(txhashes);
  int32_t txindex = (int32_t)std::distance(txes.begin(), txit);

  popTx.bitcoinTransaction = *txit;
  popTx.merklePath.index = txindex;
  popTx.merklePath.subject = txhashes[txindex];
  popTx.merklePath.layers = mtree.getMerklePathLayers(txhashes[txindex]);

  for (auto* walkBlock = containingBlockIndex->pprev;
       walkBlock != nullptr && walkBlock->getHash() != lastKnownBtcBlockHash;
       walkBlock = walkBlock->pprev) {
    const auto& header = walkBlock->getHeader();
    popTx.blockOfProofContext.push_back(header);
  }
  std::reverse(popTx.blockOfProofContext.begin(),
               popTx.blockOfProofContext.end());

  auto hash = popTx.getHash();
  popTx.signature =
      secp256k1::sign(hash, secp256k1::privateKeyFromVbk(defaultPrivateKeyVbk));

  vbkmempool.push_back(popTx);

  return popTx;
}

VbkTx MockMiner2::createVbkTxEndorsingAltBlock(
    const PublicationData& publicationData) {
  VbkTx transaction;
  transaction.signatureIndex = 7;
  transaction.networkOrType.networkType = vbk_params.getTransactionMagicByte();
  transaction.networkOrType.typeId = (uint8_t)TxType::VBK_TX;
  transaction.sourceAmount = Coin(1000);
  transaction.sourceAddress = Address::fromPublicKey(defaultPublicKeyVbk);
  transaction.publicKey = defaultPublicKeyVbk;
  transaction.publicationData = publicationData;

  auto hash = transaction.getHash();
  transaction.signature =
      secp256k1::sign(hash, secp256k1::privateKeyFromVbk(defaultPrivateKeyVbk));

  return transaction;
}

BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(size_t amount) {
  return mineBtcBlocks(amount, *getBtcTipIndex());
}

BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(
    size_t amount,
    const std::vector<BtcTx>& transactions) {
  return mineBtcBlocks(amount, *getBtcTipIndex(), transactions);
}

BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(
    size_t amount,
    const BlockIndex<BtcBlock>& tip) {
  BlockIndex<BtcBlock>* blockIndex = mineBtcBlocks(1, tip, btcmempool);
  btcmempool.clear();
  if (amount == 1) {
    return blockIndex;
  }
  return mineBtcBlocks(amount - 1, *blockIndex, {});
}

BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(
    size_t amount,
    const BlockIndex<BtcBlock>& tip,
    const std::vector<BtcTx>& transactions) {
  const BlockIndex<BtcBlock>* lastBlockIndex = &tip;
  BlockIndex<BtcBlock>* blockIndex = nullptr;
  for (size_t i = 0; i < amount; i++) {
    blockIndex =
        mineBlock(btc_miner, vbktree.btc(), *lastBlockIndex, transactions);
    btcTxs[blockIndex->getHash()] = transactions;
    lastBlockIndex = blockIndex;
  }
  return blockIndex;
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(size_t amount) {
  return mineVbkBlocks(amount, *getVbkTipIndex());
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(
    size_t amount,
    const std::vector<VbkPopTx>& transactions) {
  return mineVbkBlocks(amount, *getVbkTipIndex(), transactions);
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(
    size_t amount,
    const std::vector<VbkTx>& transactions) {
  return mineVbkBlocks(amount, *getVbkTipIndex(), transactions);
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(size_t amount,
                                                const BlockIndex<VbkBlock>& tip) {
  BlockIndex<VbkBlock>* blockIndex = mineVbkBlocks(1, tip, vbkmempool);
  vbkmempool.clear();
  if (amount == 1) {
    return blockIndex;
  }
  return mineVbkBlocks(amount - 1, *blockIndex, std::vector<VbkPopTx>());
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(
    size_t amount,
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkPopTx>& transactions) {
  const BlockIndex<VbkBlock>* lastBlockIndex = &tip;
  BlockIndex<VbkBlock>* blockIndex = nullptr;
  for (size_t i = 0; i < amount; i++) {
    blockIndex = mineBlock(vbk_miner, vbktree, *lastBlockIndex, transactions);
    vbkPopTxs[blockIndex->getHash()] = transactions;
    savePayloads(blockIndex);
    lastBlockIndex = blockIndex;
  }
  return blockIndex;
}

BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(
    size_t amount,
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkTx>& transactions) {
  const BlockIndex<VbkBlock>* lastBlockIndex = &tip;
  BlockIndex<VbkBlock>* blockIndex = nullptr;
  for (size_t i = 0; i < amount; i++) {
    blockIndex = mineBlock(vbk_miner, vbktree, *lastBlockIndex, transactions);
    vbkTxs[blockIndex->getHash()] = transactions;
    lastBlockIndex = blockIndex;
  }
  return blockIndex;
}

std::vector<VTB> MockMiner2::getVTBs(
    const BlockIndex<VbkBlock>& blockIndex) const {
  auto it = vbkPopTxs.find(blockIndex.getHash());
  if (it == vbkPopTxs.end()) {
    return {};
  }
  std::vector<VbkPopTx> transactions = it->second;
  std::vector<VbkPopTx::hash_t> hashes = hashAll<VbkPopTx>(transactions);
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree merkleTree(hashes, treeIndex);
  std::vector<VTB> vtbs(transactions.size());
  for (size_t i = 0; i < transactions.size(); i++) {
    VTB& vtb = vtbs[i];
    vtb.transaction = transactions[i];
    vtb.merklePath.treeIndex = treeIndex;
    vtb.merklePath.index = i;
    vtb.merklePath.subject = hashes[i];
    vtb.merklePath.layers = merkleTree.getMerklePathLayers(hashes[i]);
    vtb.containingBlock = blockIndex.getHeader();
  }
  return vtbs;
}

std::vector<ATV> MockMiner2::getATVs(
    const BlockIndex<VbkBlock>& blockIndex) const {
  auto it = vbkTxs.find(blockIndex.getHash());
  if (it == vbkTxs.end()) {
    return {};
  }
  std::vector<VbkTx> transactions = it->second;
  std::vector<VbkTx::hash_t> hashes = hashAll<VbkTx>(transactions);
  const int32_t treeIndex = 1;  // this is NORMAL tx
  VbkMerkleTree merkleTree(hashes, treeIndex);
  std::vector<ATV> atvs(transactions.size());
  for (size_t i = 0; i < transactions.size(); i++) {
    ATV& atv = atvs[i];
    atv.transaction = transactions[i];
    atv.merklePath.treeIndex = treeIndex;
    atv.merklePath.index = i;
    atv.merklePath.subject = hashes[i];
    atv.merklePath.layers = merkleTree.getMerklePathLayers(hashes[i]);
    atv.blockOfProof = blockIndex.getHeader();
  }
  return atvs;
}

const BtcBlock MockMiner2::getBtcTip() const {
  return getBtcTipIndex()->getHeader();
}

const BlockIndex<BtcBlock>* MockMiner2::getBtcTipIndex() const {
  BlockIndex<BtcBlock>* index = vbktree.btc().getBestChain().tip();
  Check(index != nullptr,
        "BTC tip is undefined (blockchain may be not bootstrapped)");
  return index;
}

const VbkBlock MockMiner2::getVbkTip() const {
  return getVbkTipIndex()->getHeader();
}

const BlockIndex<VbkBlock>* MockMiner2::getVbkTipIndex() const {
  BlockIndex<VbkBlock>* index = vbktree.getBestChain().tip();
  Check(index != nullptr,
        "VBK tip is undefined (blockchain may be not bootstrapped)");
  return index;
}

void MockMiner2::savePayloads(BlockIndex<VbkBlock>* blockIndex) {
  std::vector<VTB> vtbs = getVTBs(*blockIndex);
  PopData pd;
  pd.vtbs = vtbs;
  payloadsProvider.writePayloads(pd);
  const auto hash = blockIndex->getHash();
  ValidationState state;
  if (!vbktree.addPayloads(hash, vtbs, state)) {
    blockIndex->removeRef(0);
    vbktree.removeLeaf(*blockIndex);
    throw std::domain_error(state.toString());
  }
  vbkPayloads[hash] = vtbs;
}

// TODO: Inline and get rid of
ATV MockMiner2::applyATV(const VbkTx& transaction) {
  BlockIndex<VbkBlock>* blockIndex = mineVbkBlocks(1, {transaction});
  return getATVs(*blockIndex)[0];
}

}  // namespace altintegration

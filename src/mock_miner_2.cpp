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
Block mineBlock(Miner& miner,
                BlockTree& tree,
                const std::vector<Tx>& transactions,
                const BlockIndex<Block>& tip) {
  Block block;
  if (transactions.empty()) {
    block = miner.createNextBlock(tip);
  } else {
    const auto& merkleRoot = calculateMerkleRoot(transactions);
    block = miner.createNextBlock(tip, merkleRoot);
  }
  ValidationState state;
  Check(acceptBlock(tree, block, state), state.toString());
  return block;
}

VbkPopTx MockMiner2::endorseVbkBlock(
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  BtcTx containingTx = createBtcTxEndorsingVbkBlock(publishedBlock);
  BtcBlock containingBlock = mineBtcBlock({containingTx});
  VbkPopTx popTx = createVbkPopTxEndorsingVbkBlock(
      containingBlock, containingTx, publishedBlock, lastKnownBtcBlockHash);
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

BtcBlock MockMiner2::mineBtcBlocks_(size_t amount) {
  return mineBtcBlocks(amount, *getBtcTipIndex());
}

BtcBlock MockMiner2::mineBtcBlocks(size_t amount,
                                   const BlockIndex<BtcBlock>& tip) {
  BtcBlock lastBlock;
  const BlockIndex<BtcBlock>* lastBlockIndex = &tip;
  for (size_t i = 0; i < amount; i++) {
    lastBlock = mineBtcBlock(*lastBlockIndex);
    lastBlockIndex = getBtcBlockIndex(lastBlock);
  }
  return lastBlock;
}

BtcBlock MockMiner2::mineBtcBlock(const BlockIndex<BtcBlock>& tip) {
  BtcBlock block = mineBtcBlock(btcmempool, tip);
  btcmempool.clear();
  return block;
}

BtcBlock MockMiner2::mineBtcBlock(const std::vector<BtcTx>& transactions) {
  return mineBtcBlock(transactions, *getBtcTipIndex());
}

BtcBlock MockMiner2::mineBtcBlock(const std::vector<BtcTx>& transactions,
                                  const BlockIndex<BtcBlock>& tip) {
  BtcBlock block = mineBlock(btc_miner, vbktree.btc(), transactions, tip);
  btcTxs[block.getHash()] = transactions;
  return block;
}

VbkBlock MockMiner2::mineVbkBlocks_(size_t amount) {
  return mineVbkBlocks(amount, *getVbkTipIndex());
}

VbkBlock MockMiner2::mineVbkBlocks(size_t amount,
                                   const BlockIndex<VbkBlock>& tip) {
  VbkBlock lastBlock;
  const BlockIndex<VbkBlock>* lastBlockIndex = &tip;
  for (size_t i = 0; i < amount; i++) {
    lastBlock = mineVbkBlock(*lastBlockIndex);
    lastBlockIndex = getVbkBlockIndex(lastBlock);
  }
  return lastBlock;
}

VbkBlock MockMiner2::mineVbkBlock(const BlockIndex<VbkBlock>& tip) {
  VbkBlock block = mineVbkBlock(vbkmempool, tip);
  vbkmempool.clear();
  return block;
}

VbkBlock MockMiner2::mineVbkBlock(const std::vector<VbkPopTx>& transactions) {
  return mineVbkBlock(transactions, *getVbkTipIndex());
}

VbkBlock MockMiner2::mineVbkBlock(const std::vector<VbkPopTx>& transactions,
                                  const BlockIndex<VbkBlock>& tip) {
  VbkBlock block = mineBlock(vbk_miner, vbktree, transactions, tip);
  vbkPopTxs[block.getHash()] = transactions;
  savePayloads(block);
  return block;
}

VbkBlock MockMiner2::mineVbkBlock(const std::vector<VbkTx>& transactions) {
  return mineVbkBlock(transactions, *getVbkTipIndex());
}

VbkBlock MockMiner2::mineVbkBlock(const std::vector<VbkTx>& transactions,
                                  const BlockIndex<VbkBlock>& tip) {
  VbkBlock block = mineBlock(vbk_miner, vbktree, transactions, tip);
  vbkTxs[block.getHash()] = transactions;
  return block;
}

std::vector<VTB> MockMiner2::getVTBs(const VbkBlock& block) const {
  auto it = vbkPopTxs.find(block.getHash());
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
    vtb.containingBlock = block;
  }
  return vtbs;
}

std::vector<ATV> MockMiner2::getATVs(const VbkBlock& block) const {
  auto it = vbkTxs.find(block.getHash());
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
    atv.blockOfProof = block;
  }
  return atvs;
}

BlockIndex<BtcBlock>* MockMiner2::getBtcBlockIndex(const BtcBlock& block) {
  return getBtcBlockIndex(block.getHash());
}

BlockIndex<BtcBlock>* MockMiner2::getBtcBlockIndex(
    const BtcBlock::hash_t& blockHash) {
  BlockIndex<BtcBlock>* index = vbktree.btc().getBlockIndex(blockHash);
  Check(index != nullptr,
        "Index not found in BTC tree for " + blockHash.toPrettyString());
  return index;
}

BlockIndex<VbkBlock>* MockMiner2::getVbkBlockIndex(const VbkBlock& block) {
  return getVbkBlockIndex(block.getHash());
}

BlockIndex<VbkBlock>* MockMiner2::getVbkBlockIndex(
    const VbkBlock::hash_t& blockHash) {
  BlockIndex<VbkBlock>* index = vbktree.getBlockIndex(blockHash);
  Check(index != nullptr,
        "Index not found in VBK tree for " + blockHash.toPrettyString());
  return index;
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

void MockMiner2::savePayloads(const VbkBlock& block) {
  std::vector<VTB> vtbs = getVTBs(block);
  const auto hash = block.getHash();
  PopData pd;
  pd.vtbs = vtbs;
  payloadsProvider.writePayloads(pd);
  ValidationState state;
  if (!vbktree.addPayloads(hash, vtbs, state)) {
    BlockIndex<VbkBlock>* index = getVbkBlockIndex(hash);
    index->removeRef(0);
    vbktree.removeLeaf(*index);
    throw std::domain_error(state.toString());
  }
  vbkPayloads[hash] = vtbs;
}

// TODO: Inline and get rid of
BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(const BlockIndex<BtcBlock>& tip,
                                                size_t amount) {
  BtcBlock block = mineBtcBlocks(amount, tip);
  return getBtcBlockIndex(block);
}
BlockIndex<BtcBlock>* MockMiner2::mineBtcBlocks(size_t amount) {
  BtcBlock block = mineBtcBlocks_(amount);
  return getBtcBlockIndex(block);
}
BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                                size_t amount) {
  VbkBlock block = mineVbkBlocks(amount, tip);
  return getVbkBlockIndex(block);
}
BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(size_t amount) {
  VbkBlock block = mineVbkBlocks_(amount);
  return getVbkBlockIndex(block);
}
BlockIndex<VbkBlock>* MockMiner2::mineVbkBlocks(
    const BlockIndex<VbkBlock>& tip,
    const std::vector<VbkPopTx>& poptxs) {
  VbkBlock block = mineVbkBlock(poptxs, tip);
  return getVbkBlockIndex(block);
}
VbkBlock MockMiner2::applyVTB(const VbkPopTx& tx) {
  return mineVbkBlock({tx});
}
VbkBlock MockMiner2::applyVTB(const VbkBlock::hash_t& tip,
                              const VbkPopTx& tx) {
  return mineVbkBlock({tx}, *getVbkBlockIndex(tip));
}
VbkBlock MockMiner2::applyVTBs(const std::vector<VbkPopTx>& txes) {
  return mineVbkBlock(txes);
}
VbkBlock MockMiner2::applyVTBs(const BlockIndex<VbkBlock>& tip,
                               const std::vector<VbkPopTx>& txes) {
  return mineVbkBlock(txes, tip);
}
ATV MockMiner2::applyATV(const VbkTx& transaction) {
  VbkBlock block = mineVbkBlock({transaction});
  return getATVs(block)[0];
}
std::vector<ATV> MockMiner2::applyATVs(const std::vector<VbkTx>& transactions) {
  VbkBlock block = mineVbkBlock(transactions);
  return getATVs(block);
}

}  // namespace altintegration

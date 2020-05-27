// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/mock_miner.hpp"

#include <stdexcept>

#include "veriblock/entities/address.hpp"
#include "veriblock/signutil.hpp"
#include "veriblock/strutil.hpp"

namespace altintegration {

static auto defaultPrivateKeyVbk = ParseHex(
    "303e020100301006072a8648ce3d020106052b8104000a0427302502010104203abf83fa47"
    "0423d4788a760ef6b7aae1dacf98784b0646057a0adca24e522acb");

static auto defaultPublicKeyVbk = ParseHex(
    "3056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099"
    "ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a8061"
    "10fa05d9014a102a0513dd354ec5");

template <typename Block, typename BlockParams>
bool mineBlocks(const uint32_t& n,
                Miner<Block, BlockParams>& miner,
                BlockTree<Block, BlockParams>& blockChain,
                ValidationState& state) {
  for (uint32_t i = 0; i < n; ++i) {
    auto* tip = blockChain.getBestChain().tip();
    assert(tip != nullptr && "block tree is not bootstrapped");

    Block block = miner.createNextBlock(*tip);
    if (!blockChain.acceptBlock(block, state)) {
      return false;
    }
  }

  return true;
}

VbkTx MockMiner::createVbkTxEndorsingAltBlock(
    const PublicationData& publicationData) {
  VbkTx transaction;
  transaction.signatureIndex = 7;
  transaction.networkOrType.hasNetworkByte =
      vbk_params.getTransactionMagicByte().hasValue;
  transaction.networkOrType.networkByte =
      vbk_params.getTransactionMagicByte().value;
  transaction.networkOrType.typeId = (uint8_t)TxType::VBK_TX;
  transaction.sourceAmount = Coin(1000);
  transaction.sourceAddress = Address::fromPublicKey(defaultPublicKeyVbk);
  transaction.publicKey = defaultPublicKeyVbk;
  transaction.publicationData = publicationData;

  auto hash = transaction.getHash();
  transaction.signature =
      veriBlockSign(hash, privateKeyFromVbk(defaultPrivateKeyVbk));

  return transaction;
}

ATV MockMiner::generateATV(const VbkTx& transaction,
                           const VbkBlock::hash_t& lastKnownVbkBlockHash,
                           ValidationState& state) {
  // build merkle tree
  auto hashes = hashAll<VbkTx>({transaction});
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto* tip = vbktree.getBestChain().tip();
  assert(tip != nullptr && "VBK blockchain is not bootstrapped");

  VbkBlock containingBlock = vbk_miner.createNextBlock(
      *tip, mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  ATV atv;
  atv.transaction = transaction;
  atv.merklePath.treeIndex = treeIndex;
  atv.merklePath.index = 0;
  atv.merklePath.subject = hashes[0];
  atv.merklePath.layers = mtree.getMerklePathLayers(hashes[0]);
  atv.containingBlock = containingBlock;

  for (auto* walkBlock = tip;
       walkBlock && walkBlock->header->getHash() != lastKnownVbkBlockHash;
       walkBlock = walkBlock->pprev) {
    atv.context.push_back(*walkBlock->header);
  }

  // since we inserted in reverse order, we need to reverse context blocks
  std::reverse(atv.context.begin(), atv.context.end());

  if (!vbktree.acceptBlock(containingBlock, state)) {
    throw std::domain_error(state.toString());
  }

  return atv;
}

BtcTx MockMiner::createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) {
  WriteStream stream;
  publishedBlock.toRaw(stream);
  auto addr = Address::fromPublicKey(defaultPublicKeyVbk);
  addr.getPopBytes(stream);
  auto tx = BtcTx(stream.data());
  btcmempool.push_back(tx);
  return tx;
}

VbkPopTx MockMiner::createVbkPopTxEndorsingVbkBlock(
    const BtcBlock& containingBlock,
    const BtcTx& containingTx,
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash) {
  const auto& btc = vbktree.btc();
  auto containingBlockIndex = btc.getBlockIndex(containingBlock.getHash());
  if (!containingBlockIndex) {
    throw std::domain_error("containing block with hash " +
                            containingBlock.getHash().toHex() +
                            " does not exist in BTC ");
  }

  VbkPopTx popTx;
  popTx.networkOrType.hasNetworkByte =
      vbk_params.getTransactionMagicByte().hasValue;
  popTx.networkOrType.networkByte = vbk_params.getTransactionMagicByte().value;
  popTx.networkOrType.typeId = (uint8_t)TxType::VBK_POP_TX;
  popTx.address = Address::fromPublicKey(defaultPublicKeyVbk);
  popTx.publishedBlock = publishedBlock;
  popTx.blockOfProof = containingBlock;
  popTx.publicKey = defaultPublicKeyVbk;

  // search for transactions in containing block
  auto btcit = btctxes.find(containingBlock.getHash());
  if (btcit == btctxes.end()) {
    throw std::domain_error(
        "in attempt to create VBK pop TX endorsing VBK block " +
        publishedBlock.getHash().toHex() +
        " we tried to search for BTC block " +
        containingBlock.getHash().toHex() + " but were unable to find it");
  }

  // search for containing tx
  auto& txes = btcit->second;
  auto txit = std::find(txes.begin(), txes.end(), containingTx);
  if (txit == txes.end()) {
    throw std::domain_error(
        "in attempt to create VBK pop TX endorsing VBK block " +
        publishedBlock.getHash().toHex() +
        " we tried to search for BTC TX containing that block in BTC block " +
        containingBlock.getHash().toHex() + " but were unable to find it");
  }

  auto txhashes = hashAll<BtcTx>(txes);
  BtcMerkleTree mtree(txhashes);
  int32_t txindex = (int32_t)std::distance(txes.begin(), txit);

  popTx.bitcoinTransaction = *txit;
  popTx.merklePath.index = txindex;
  popTx.merklePath.subject = txhashes[txindex];
  popTx.merklePath.layers = mtree.getMerklePathLayers(txhashes[txindex]);

  for (auto* walkBlock = containingBlockIndex->pprev;
       walkBlock && walkBlock->getHash() != lastKnownBtcBlockHash;
       walkBlock = walkBlock->pprev) {
    auto header = *walkBlock->header;
    popTx.blockOfProofContext.push_back(header);
  }
  std::reverse(popTx.blockOfProofContext.begin(),
               popTx.blockOfProofContext.end());

  auto hash = popTx.getHash();
  popTx.signature =
      veriBlockSign(hash, privateKeyFromVbk(defaultPrivateKeyVbk));

  vbkmempool.push_back(popTx);

  return popTx;
}

VbkPopTx MockMiner::endorseVbkBlock(
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash,
    ValidationState& state) {
  VbkPopTx popTx;
  popTx.networkOrType.hasNetworkByte =
      vbk_params.getTransactionMagicByte().hasValue;
  popTx.networkOrType.networkByte = vbk_params.getTransactionMagicByte().value;
  popTx.networkOrType.typeId = (uint8_t)TxType::VBK_POP_TX;
  popTx.address = Address::fromPublicKey(defaultPublicKeyVbk);
  popTx.publishedBlock = publishedBlock;

  popTx.bitcoinTransaction = createBtcTxEndorsingVbkBlock(publishedBlock);
  popTx.merklePath.index = 0;
  popTx.merklePath.subject = popTx.bitcoinTransaction.getHash();
  popTx.merklePath.layers = {popTx.bitcoinTransaction.getHash()};

  auto tip = vbktree.btc().getBestChain().tip();
  assert(tip != nullptr && "BTC blockchain is not bootstrapped");

  for (auto* walkBlock = tip;
       walkBlock && walkBlock->getHash() != lastKnownBtcBlockHash;
       walkBlock = walkBlock->pprev) {
    popTx.blockOfProofContext.push_back(*walkBlock->header);
  }
  std::reverse(popTx.blockOfProofContext.begin(),
               popTx.blockOfProofContext.end());

  popTx.blockOfProof = btc_miner.createNextBlock(
      *tip, popTx.merklePath.calculateMerkleRoot().reverse());

  if (!vbktree.btc().acceptBlock(popTx.blockOfProof, state)) {
    throw std::domain_error(state.GetDebugMessage());
  }

  popTx.publicKey = defaultPublicKeyVbk;

  auto hash = popTx.getHash();

  popTx.signature =
      veriBlockSign(hash, privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

VbkBlock MockMiner::applyVTB(const BlockIndex<VbkBlock>& tip,
                             VbkBlockTree& tree,
                             const VbkPopTx& tx,
                             ValidationState& state) {
  return applyVTBs(tip, tree, {tx}, state);
}

VbkBlock MockMiner::applyVTB(VbkBlockTree& tree,
                             const VbkPopTx& tx,
                             ValidationState& state) {
  auto* tip = tree.getBestChain().tip();
  assert(tip && "VBK blockchain is not bootstrapped");
  return applyVTB(*tip, tree, tx, state);
}

VbkBlock MockMiner::applyVTBs(VbkBlockTree& tree,
                              const std::vector<VbkPopTx>& txes,
                              ValidationState& state) {
  auto* tip = tree.getBestChain().tip();
  assert(tip && "VBK blockchain is not bootstrapped");
  return applyVTBs(*tip, tree, txes, state);
}

VbkBlock MockMiner::applyVTBs(const BlockIndex<VbkBlock>& tip,
                              VbkBlockTree& tree,
                              const std::vector<VbkPopTx>& txes,
                              ValidationState& state) {
  // build merkle tree
  auto hashes = hashAll<VbkPopTx>(txes);
  const int32_t treeIndex = 0;  // this is POP tx
  VbkMerkleTree mtree(hashes, treeIndex);

  // create containing block
  auto containingBlock = vbk_miner.createNextBlock(
      tip, mtree.getMerkleRoot().trim<VBK_MERKLE_ROOT_HASH_SIZE>());

  // map VbkPopTx -> VTB
  std::vector<VTB> vtbs;
  vtbs.reserve(txes.size());
  int32_t index = 0;
  std::transform(txes.begin(),
                 txes.end(),
                 std::back_inserter(vtbs),
                 [&](const VbkPopTx& tx) -> VTB {
                   VTB vtb;
                   vtb.transaction = tx;
                   vtb.merklePath.treeIndex = treeIndex;
                   vtb.merklePath.index = index;
                   vtb.merklePath.subject = hashes[index];
                   vtb.merklePath.layers =
                       mtree.getMerklePathLayers(hashes[index]);
                   vtb.containingBlock = containingBlock;
                   index++;

                   return vtb;
                 });

  auto containingHash = containingBlock.getHash();
  if (!tree.acceptBlock(containingBlock, state)) {
    throw std::domain_error(state.toString());
  }
  if (!tree.addPayloads(containingHash, vtbs, state)) {
    throw std::domain_error(state.toString());
  }
  vbkPayloads[containingHash] = vtbs;

  return containingBlock;
}

BlockIndex<BtcBlock>* MockMiner::mineBtcBlocks(const BlockIndex<BtcBlock>& tip,
                                               size_t amount) {
  BtcBlock::hash_t last = tip.getHash();
  if (!btcmempool.empty() && amount > 0) {
    //! we "simulate" mempool - a vector of transactions that can be added for
    //! "further processing". here we mine first block separately, as it should
    //! contain all transactions from mempool
    BtcMerkleTree mtree(hashAll<BtcTx>(btcmempool));
    auto block = btc_miner.createNextBlock(tip, mtree.getMerkleRoot());
    if (!vbktree.btc().acceptBlock(block, state_)) {
      throw std::domain_error(state_.GetDebugMessage());
    }

    btctxes[block.getHash()] = btcmempool;

    btcmempool.clear();
    last = block.getHash();
    --amount;
  }
  for (size_t i = 0; i < amount; i++) {
    auto* index = vbktree.btc().getBlockIndex(last);
    assert(index);
    auto block = btc_miner.createNextBlock(*index);
    if (!vbktree.btc().acceptBlock(block, state_)) {
      throw std::domain_error(state_.GetDebugMessage());
    }
    last = block.getHash();
  }

  return vbktree.btc().getBlockIndex(last);
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(const BlockIndex<VbkBlock>& tip,
                                               size_t amount) {
  VbkBlock::hash_t last = tip.getHash();
  if (!vbkmempool.empty() && amount > 0) {
    //! we "simulate" mempool - a vector of transactions that can be added for
    //! "further processing". here we mine first block separately, as it should
    //! contain all transactions from mempool.
    auto containing = applyVTBs(tip, vbktree, vbkmempool, state_);
    last = containing.getHash();

    // we generated 1 block
    --amount;
    vbkmempool.clear();
  }

  for (size_t i = 0; i < amount; i++) {
    auto* index = vbktree.getBlockIndex(last);
    assert(index);
    auto block = vbk_miner.createNextBlock(*index);
    if (!vbktree.acceptBlock(block, state_)) {
      throw std::domain_error(state_.GetDebugMessage());
    }
    last = block.getHash();
  }

  return vbktree.getBlockIndex(last);
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(
    const BlockIndex<VbkBlock>& tip, const std::vector<VbkPopTx>& poptxs) {
  //! we "simulate" mempool - a vector of transactions that can be added for
  //! "further processing". here we mine first block separately, as it
  //! should contain all transactions from mempool.
  auto containing = applyVTBs(tip, vbktree, poptxs, state_);
  return vbktree.getBlockIndex(containing.getHash());
}

BlockIndex<BtcBlock>* MockMiner::mineBtcBlocks(size_t amount) {
  auto* tip = vbktree.btc().getBestChain().tip();
  assert(tip);
  return mineBtcBlocks(*tip, amount);
}

BlockIndex<VbkBlock>* MockMiner::mineVbkBlocks(size_t amount) {
  auto* tip = vbktree.getBestChain().tip();
  assert(tip);
  return mineVbkBlocks(*tip, amount);
}

// void MockMiner::getGeneratedVTBs(const BlockIndex<VbkBlock>& containingBlock,
//                                 std::vector<VTB>& vtbs) {
////  vtbp_.get(containingBlock.containingPayloads, &vtbs);
//}

}  // namespace altintegration

#include "veriblock/mock_miner.hpp"

#include "veriblock/entities/address.hpp"
#include "veriblock/signutil.hpp"
#include "veriblock/strutil.hpp"

using namespace AltIntegrationLib;

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

VbkTx MockMiner::generateSignedVbkTx(const PublicationData& publicationData) {
  VbkTx transaction;
  transaction.signatureIndex = 7;
  transaction.networkOrType.hasNetworkByte =
      vbk_params->getTransactionMagicByte().hasValue;
  transaction.networkOrType.networkByte =
      vbk_params->getTransactionMagicByte().value;
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

ATV MockMiner::generateValidATV(const PublicationData& publicationData,
                                const VbkBlock::hash_t& lastKnownVbkBlockHash,
                                ValidationState& state) {
  ATV atv;
  atv.transaction = generateSignedVbkTx(publicationData);
  atv.merklePath.treeIndex = 1;
  atv.merklePath.index = 0;
  atv.merklePath.subject = atv.transaction.getHash();
  atv.merklePath.layers = {atv.transaction.getHash()};

  BlockIndex<vbk_block_t>* tip = vbk_blockchain->getBestChain().tip();

  assert(tip != nullptr && "not bootstrapped");

  for (BlockIndex<VbkBlock>* walkBlock = tip;
       walkBlock->header.getHash() != lastKnownVbkBlockHash;
       walkBlock = walkBlock->pprev) {
    atv.context.push_back(walkBlock->header);
  }
  // since we inserted in reverse order, we need to reverse context blocks
  std::reverse(atv.context.begin(), atv.context.end());

  atv.containingBlock =
      vbk_miner->createNextBlock(*tip, atv.merklePath.calculateMerkleRoot());

  vbk_blockchain->acceptBlock(atv.containingBlock, state);

  return atv;
}

VbkPopTx MockMiner::generateSignedVbkPoptx(
    const VbkBlock& publishedBlock,
    const BtcBlock::hash_t& lastKnownBtcBlockHash,
    ValidationState& state) {
  VbkPopTx popTx;
  popTx.networkOrType.hasNetworkByte =
      vbk_params->getTransactionMagicByte().hasValue;
  popTx.networkOrType.networkByte = vbk_params->getTransactionMagicByte().value;
  popTx.networkOrType.typeId = (uint8_t)TxType::VBK_POP_TX;
  popTx.address = Address::fromPublicKey(defaultPublicKeyVbk);
  popTx.publishedBlock = publishedBlock;

  WriteStream stream;
  popTx.publishedBlock.toRaw(stream);
  popTx.address.getPopBytes(stream);

  popTx.bitcoinTransaction = BtcTx(stream.data());
  popTx.merklePath.index = 0;
  popTx.merklePath.subject = popTx.bitcoinTransaction.getHash();
  popTx.merklePath.layers = {popTx.bitcoinTransaction.getHash()};

  BlockIndex<btc_block_t>* tip = btc_blockchain->getBestChain().tip();

  assert(tip != nullptr && "BTC blockchain is not bootstrapped");

  for (auto* walkBlock = tip;
       walkBlock && walkBlock->getHash() != lastKnownBtcBlockHash;
       walkBlock = walkBlock->pprev) {
    popTx.blockOfProofContext.push_back(walkBlock->header);
  }
  std::reverse(popTx.blockOfProofContext.begin(),
               popTx.blockOfProofContext.end());

  popTx.blockOfProof = btc_miner->createNextBlock(
      *tip, popTx.merklePath.calculateMerkleRoot().reverse());

  btc_blockchain->acceptBlock(popTx.blockOfProof, state);

  popTx.publicKey = defaultPublicKeyVbk;

  auto hash = popTx.getHash();

  popTx.signature =
      veriBlockSign(hash, privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

VTB MockMiner::generateValidVTB(const VbkBlock& publishedBlock,
                                const VbkBlock::hash_t& lastKnownVbkBlockHash,
                                const BtcBlock::hash_t& lastKnownBtcBlockHash,
                                ValidationState& state) {
  VTB vtb;
  vtb.transaction =
      generateSignedVbkPoptx(publishedBlock, lastKnownBtcBlockHash, state);
  vtb.merklePath.treeIndex = 1;
  vtb.merklePath.index = 0;
  vtb.merklePath.subject = vtb.transaction.getHash();
  vtb.merklePath.layers = {vtb.transaction.getHash()};

  auto* tip = vbk_blockchain->getBestChain().tip();

  assert(tip != nullptr && "VBK blockhain is not bootstrapped");

  for (auto* walkBlock = tip;
       walkBlock && walkBlock->getHash() != lastKnownVbkBlockHash;
       walkBlock = walkBlock->pprev) {
    vtb.context.push_back(walkBlock->header);
  }
  std::reverse(vtb.context.begin(), vtb.context.end());

  vtb.containingBlock =
      vbk_miner->createNextBlock(*tip, vtb.merklePath.calculateMerkleRoot());

  vbk_blockchain->acceptBlock(vtb.containingBlock, state);

  return vtb;
}

Publications MockMiner::mine(const PublicationData& publicationData,
                             const VbkBlock::hash_t& lastKnownVbkBlockHash,
                             const BtcBlock::hash_t& lastKnownBtcBlockHash,
                             const uint32_t& vbkBlockDelay,
                             ValidationState& state) {
  ATV atv = generateValidATV(publicationData, lastKnownVbkBlockHash, state);

  for (uint32_t i = 0; i != vbkBlockDelay; ++i) {
    BlockIndex<vbk_block_t>* tip = vbk_blockchain->getBestChain().tip();

    assert(tip != nullptr && "VBK blockchain is not bootstrapped");

    VbkBlock minedBlock = vbk_miner->createNextBlock(*tip, uint128());
    vbk_blockchain->acceptBlock(minedBlock, state);
  }

  VTB vtb = generateValidVTB(
      atv.containingBlock, lastKnownVbkBlockHash, lastKnownBtcBlockHash, state);

  return {atv, {vtb}};
}

bool MockMiner::mineBtcBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, *btc_miner, *btc_blockchain, state);
}

bool MockMiner::mineVbkBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, *vbk_miner, *vbk_blockchain, state);
}
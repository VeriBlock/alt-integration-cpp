#include "veriblock/entities/address.hpp"
#include "veriblock/mock_miner.hpp"
#include "veriblock/signutil.hpp"
#include "veriblock/strutil.hpp"

using namespace VeriBlock;

static auto defaultPrivateKeyVbk = ParseHex(
    "303e020100301006072a8648ce3d020106052b8104000a0427302502010104203abf83fa47"
    "0423d4788a760ef6b7aae1dacf98784b0646057a0adca24e522acb");

static auto defaultPublicKeyVbk = ParseHex(
    "3056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099"
    "ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a8061"
    "10fa05d9014a102a0513dd354ec5");

template <typename Block, typename BlockParams>
void mineBlocks(const uint32_t& n,
                std::shared_ptr<Miner<Block, BlockParams>> miner,
                std::shared_ptr<BlockTree<Block, BlockParams>> blockChain,
                ValidationState& state) {
  for (uint32_t i = 0; i < n; ++i) {
    Block block = miner->createNextBlock(*blockChain->getBestChain().tip(), {});
    blockChain->acceptBlock(block, state);
  }
}

VbkTx MockMiner::generateSignedVbkTx(const PublicationData& publicationData) {
  VbkTx transaction;
  transaction.signatureIndex = 7;
  transaction.networkOrType.hasNetworkByte = true;
  transaction.networkOrType.networkByte = 1;
  transaction.networkOrType.typeId = (uint8_t)TxType::VBK_TX;
  transaction.sourceAmount = Coin(1000);
  transaction.sourceAddress = Address::fromPublicKey(defaultPublicKeyVbk);
  transaction.publicKey = defaultPublicKeyVbk;
  transaction.publicationData = publicationData;

  transaction.signature = veriBlockSign(
      transaction.getHash().slice(), privateKeyFromVbk(defaultPrivateKeyVbk));

  return transaction;
}

ATV MockMiner::generateValidATV(const PublicationData& publicationData,
                                const VbkBlock& lastKnownVbkBlock) {
  ATV atv;
  atv.transaction = generateSignedVbkTx(publicationData);
  atv.merklePath.treeIndex = 1;
  atv.merklePath.index = 0;
  atv.merklePath.subject = atv.transaction.getHash();
  atv.merklePath.layers = {atv.transaction.getHash()};

  for (BlockIndex<VbkBlock>* walkBlock = vbk_blockchain->getBestChain().tip();
       walkBlock->header != lastKnownVbkBlock;
       walkBlock = walkBlock->pprev) {
    atv.context.insert(atv.context.begin(), walkBlock->header);
  }

  atv.containingBlock =
      vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                 atv.merklePath.calculateMerkleRoot());

  vbk_blockchain->acceptBlock(atv.containingBlock, state);

  return atv;
}

VbkPopTx MockMiner::generateSignedVbkPoptx(const VbkBlock& publishedBlock,
                                           const BtcBlock& lastKnownBtcBlock) {
  VbkPopTx popTx;
  popTx.networkOrType.hasNetworkByte = true;
  popTx.networkOrType.networkByte = 1;
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

  for (BlockIndex<BtcBlock>* walkBlock = btc_blockchain->getBestChain().tip();
       walkBlock->header != lastKnownBtcBlock;
       walkBlock = walkBlock->pprev) {
    popTx.blockOfProofContext.insert(popTx.blockOfProofContext.begin(),
                                     walkBlock->header);
  }

  popTx.blockOfProof = btc_miner->createNextBlock(
      *btc_blockchain->getBestChain().tip(),
      popTx.merklePath.calculateMerkleRoot().reverse());

  btc_blockchain->acceptBlock(popTx.blockOfProof, state);

  popTx.publicKey = defaultPublicKeyVbk;

  popTx.signature = veriBlockSign(popTx.getHash().slice(),
                                  privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

VTB MockMiner::generateValidVTB(const VbkBlock& publishedBlock,
                                const BtcBlock& lastKnownBtcBlock,
                                const uint32_t& vbkBlockDelay) {
  VTB vtb;
  vtb.transaction = generateSignedVbkPoptx(publishedBlock, lastKnownBtcBlock);
  vtb.merklePath.treeIndex = 1;
  vtb.merklePath.index = 0;
  vtb.merklePath.subject = vtb.transaction.getHash();
  vtb.merklePath.layers = {vtb.transaction.getHash()};

  for (uint32_t i = 0; i != vbkBlockDelay; ++i) {
    VbkBlock mindeBlock =
        vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                   vtb.merklePath.calculateMerkleRoot());
    vbk_blockchain->acceptBlock(mindeBlock, state);
    vtb.context.push_back(mindeBlock);
  }

  vtb.containingBlock =
      vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                 vtb.merklePath.calculateMerkleRoot());

  vbk_blockchain->acceptBlock(vtb.containingBlock, state);

  return vtb;
}

Publications MockMiner::mine(const PublicationData& publicationData,
                             const VbkBlock& lastKnownVbkBlock,
                             const BtcBlock& lastKnownBtcBlock,
                             const uint32_t& vbkBlockDelay) {
  ATV atv = generateValidATV(publicationData, lastKnownVbkBlock);
  VTB vtb =
      generateValidVTB(atv.containingBlock, lastKnownBtcBlock, vbkBlockDelay);

  return {atv, {vtb}};
}

void MockMiner::addNewBtcBlocksIntoChainState(const uint32_t& n) {
  mineBlocks(n, btc_miner, btc_blockchain, state);
}

void MockMiner::addNewVbkBlocksIntoChainState(const uint32_t& n) {
  mineBlocks(n, vbk_miner, vbk_blockchain, state);
}

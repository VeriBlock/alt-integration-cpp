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
bool mineBlocks(const uint32_t& n,
                std::shared_ptr<Miner<Block, BlockParams>> miner,
                std::shared_ptr<BlockTree<Block, BlockParams>> blockChain,
                ValidationState& state) {
  bool res = true;
  for (uint32_t i = 0; i < n && res; ++i) {
    BlockIndex<Block>* tip = blockChain->getBestChain().tip();
    assert(tip != nullptr);

    Block block = miner->createNextBlock(*tip, {});
    res = blockChain->acceptBlock(block, state);
  }

  return res;
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

  assert(tip != nullptr);

  for (BlockIndex<VbkBlock>* walkBlock = tip;
       walkBlock->header.getHash() != lastKnownVbkBlockHash;
       walkBlock = walkBlock->pprev) {
    atv.context.insert(atv.context.begin(), walkBlock->header);
  }

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

  BlockIndex<btc_block_t>* tip = btc_blockchain->getBestChain().tip();

  assert(tip != nullptr);

  for (BlockIndex<BtcBlock>* walkBlock = tip;
       walkBlock->header.getHash() != lastKnownBtcBlockHash;
       walkBlock = walkBlock->pprev) {
    popTx.blockOfProofContext.insert(popTx.blockOfProofContext.begin(),
                                     walkBlock->header);
  }

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
                                const BtcBlock::hash_t& lastKnownBtcBlockHash,
                                const uint32_t& vbkBlockDelay,
                                ValidationState& state) {
  VTB vtb;
  vtb.transaction =
      generateSignedVbkPoptx(publishedBlock, lastKnownBtcBlockHash, state);
  vtb.merklePath.treeIndex = 1;
  vtb.merklePath.index = 0;
  vtb.merklePath.subject = vtb.transaction.getHash();
  vtb.merklePath.layers = {vtb.transaction.getHash()};

  for (uint32_t i = 0; i != vbkBlockDelay; ++i) {
    BlockIndex<vbk_block_t>* tip = vbk_blockchain->getBestChain().tip();

    assert(tip != nullptr);

    VbkBlock mindeBlock =
        vbk_miner->createNextBlock(*tip, vtb.merklePath.calculateMerkleRoot());
    vbk_blockchain->acceptBlock(mindeBlock, state);
    vtb.context.push_back(mindeBlock);
  }

  BlockIndex<vbk_block_t>* tip = vbk_blockchain->getBestChain().tip();

  assert(tip != nullptr);

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
  VTB vtb = generateValidVTB(
      atv.containingBlock, lastKnownBtcBlockHash, vbkBlockDelay, state);

  return {atv, {vtb}};
}

bool MockMiner::mineBtcBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, btc_miner, btc_blockchain, state);
}

bool MockMiner::mineVbkBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, vbk_miner, vbk_blockchain, state);
}

bool MockMiner::bootstrapBtcChainWithGenesis(ValidationState& state) {
  return btc_blockchain->bootstrapWithGenesis(state);
}

bool MockMiner::bootstrapBtcChainWithChain(
    btc_block_t::height_t startHeight,
    const std::vector<btc_block_t>& chain,
    ValidationState& state) {
  return btc_blockchain->bootstrapWithChain(startHeight, chain, state);
}

bool MockMiner::bootstrapVbkChainWithGenesis(ValidationState& state) {
  return vbk_blockchain->bootstrapWithGenesis(state);
}

bool MockMiner::bootstrapVbkChainWithChain(
    vbk_block_t::height_t startHeight,
    const std::vector<vbk_block_t>& chain,
    ValidationState& state) {
  return vbk_blockchain->bootstrapWithChain(startHeight, chain, state);
}

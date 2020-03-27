#include "veriblock/mock_miner.hpp"

#include <stdexcept>

#include "veriblock/entities/address.hpp"
#include "veriblock/signutil.hpp"
#include "veriblock/strutil.hpp"

using namespace altintegration;

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

ATV MockMiner::generateAndApplyATV(
    const PublicationData& publicationData,
    const VbkBlock::hash_t& lastKnownVbkBlockHash,
    ValidationState& state) {
  ATV atv = generateATV(publicationData);

  auto* tip = vbk_blockchain->getBestChain().tip();
  assert(tip != nullptr && "VBK blockchain is not bootstrapped");

  for (auto* walkBlock = tip;
       walkBlock->header.getHash() != lastKnownVbkBlockHash;
       walkBlock = walkBlock->pprev) {
    atv.context.push_back(walkBlock->header);
  }
  // since we inserted in reverse order, we need to reverse context blocks
  std::reverse(atv.context.begin(), atv.context.end());

  atv.containingBlock =
      vbk_miner->createNextBlock(*tip, atv.merklePath.calculateMerkleRoot());

  if (!vbk_blockchain->acceptBlock(atv.containingBlock, {}, state)) {
    throw std::logic_error(state.GetDebugMessage());
  }

  return atv;
}

ATV MockMiner::generateATV(const PublicationData& publicationData) {
  ATV atv;
  atv.transaction = generateSignedVbkTx(publicationData);
  atv.merklePath.treeIndex = 1;
  atv.merklePath.index = 0;
  atv.merklePath.subject = atv.transaction.getHash();
  atv.merklePath.layers = {atv.transaction.getHash()};
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

  auto tip = btc_blockchain->getBestChain().tip();
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

  if (!btc_blockchain->acceptBlock(popTx.blockOfProof, state)) {
    throw std::domain_error(state.GetDebugMessage());
  }

  popTx.publicKey = defaultPublicKeyVbk;

  auto hash = popTx.getHash();

  popTx.signature =
      veriBlockSign(hash, privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

VTB MockMiner::generateAndApplyVTB(VbkBlockTree& tree,
                                   const VbkBlock& publishedBlock,
                                   ValidationState& state) {
  auto* btctip = tree.btc().getBestChain().tip();
  assert(btctip && "BTC blockchain is not bootstrapped");

  auto lastKnownBtcBlockHash = btctip->getHash();

  VTB vtb = generateVTB(publishedBlock, lastKnownBtcBlockHash, state);

  auto* tip = tree.getBestChain().tip();
  assert(tip && "VBK blockchain is not bootstrapped");

  vtb.containingBlock =
      vbk_miner->createNextBlock(*tip, vtb.merklePath.calculateMerkleRoot());

  if (!tree.acceptBlock(vtb.containingBlock, {vtb}, state)) {
    throw std::domain_error(state.GetDebugMessage());
  }

  return vtb;
}

VTB MockMiner::generateVTB(const VbkBlock& publishedBlock,
                           const BtcBlock::hash_t& lastKnownBtcBlockHash,
                           ValidationState& state) {
  VTB vtb;
  vtb.transaction =
      generateSignedVbkPoptx(publishedBlock, lastKnownBtcBlockHash, state);
  vtb.merklePath.treeIndex = 1;
  vtb.merklePath.index = 0;
  vtb.merklePath.subject = vtb.transaction.getHash();
  vtb.merklePath.layers = {vtb.transaction.getHash()};

  return vtb;
}

bool MockMiner::mineBtcBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, *btc_miner, *btc_blockchain, state);
}

bool MockMiner::mineVbkBlocks(const uint32_t& n, ValidationState& state) {
  return mineBlocks(n, *vbk_miner, *vbk_blockchain, state);
}
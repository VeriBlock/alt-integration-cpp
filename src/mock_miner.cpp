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

ATV MockMiner::generateValidATV(const PublicationData& publicationData) {
  ATV atv;
  atv.transaction = generateSignedVbkTx(publicationData);
  atv.merklePath.treeIndex = 1;
  atv.merklePath.index = 0;
  atv.merklePath.subject = atv.transaction.getHash();
  atv.merklePath.layers = {atv.transaction.getHash()};

  atv.containingBlock =
      vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                 atv.merklePath.calculateMerkleRoot());

  return atv;
}

VbkPopTx MockMiner::generateSignedVbkPoptx(const VbkBlock& publishedBlock) {
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
  popTx.blockOfProof = btc_miner->createNextBlock(
      *btc_blockchain->getBestChain().tip(),
      popTx.merklePath.calculateMerkleRoot().reverse());

  popTx.publicKey = defaultPublicKeyVbk;

  popTx.signature = veriBlockSign(popTx.getHash().slice(),
                                  privateKeyFromVbk(defaultPrivateKeyVbk));

  return popTx;
}

VTB MockMiner::generateValidVTB(const VbkBlock& publishedBlock) {
  VTB vtb;
  vtb.transaction = generateSignedVbkPoptx(publishedBlock);
  vtb.merklePath.treeIndex = 1;
  vtb.merklePath.index = 0;
  vtb.merklePath.subject = vtb.transaction.getHash();
  vtb.merklePath.layers = {vtb.transaction.getHash()};

  vtb.containingBlock =
      vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                 vtb.merklePath.calculateMerkleRoot());

  return vtb;
}

Publications MockMiner::mine(const PublicationData& publicationData) {
  ATV atv = generateValidATV(publicationData);
  VTB vtb = generateValidVTB(atv.containingBlock);

  return {atv, {vtb}};
}

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
  transaction.networkOrType = {true, 1, (uint8_t)TxType::VBK_TX};
  transaction.sourceAmount = Coin(1000);
  transaction.sourceAddress = Address::fromPublicKey(defaultPublicKeyVbk);
  transaction.publicKey = defaultPublicKeyVbk;
  transaction.publicationData = publicationData;

  auto privateKey = privateKeyFromVbk(defaultPrivateKeyVbk);
  auto signature = veriBlockSign(transaction.getHash().slice(), privateKey);

  transaction.signature = signature;

  return transaction;
}

ATV MockMiner::generateValidATV(const PublicationData& publicationData) {
  ATV atv;
  atv.transaction = generateSignedVbkTx(publicationData);
  atv.merklePath = {
      1, 0, atv.transaction.getHash(), {atv.transaction.getHash()}};

  atv.containingBlock =
      vbk_miner->createNextBlock(*vbk_blockchain->getBestChain().tip(),
                                 atv.merklePath.calculateMerkleRoot());

  return atv;
}

Publications MockMiner::mine(const PublicationData& publicationData) {
  ATV atv = generateValidATV(publicationData);

  return {atv, std::vector<VTB>()};
}

#include <algorithm>
#include <string>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/consts.hpp"
#include "veriblock/stateless_validation.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

static const ArithUint256 MAXIMUM_DIFFICULTY(
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

bool checkBitcoinTransactionForPoPData(const VbkPopTx& tx,
                                       ValidationState& state) {
  WriteStream stream;
  tx.publishedBlock.toRaw(stream);
  tx.address.getPopBytes(stream);

  // finding that stream data contains in the tx.bitcoinTransaction
  for (size_t i = 0, j = 0;
       i < tx.bitcoinTransaction.tx.size() - stream.data().size() + 1;
       ++i) {
    bool found = true;
    for (; j < stream.data().size(); ++j) {
      if (tx.bitcoinTransaction.tx[i + j] != stream.data()[j]) {
        found = false;
        break;
      }
    }
    if (found) {
      return true;
    }
  }

  return state.Invalid(
      "checkBitcoinTransactionForPoPData()"
      "Invalid Vbk Pop transaction",
      "Bitcoin transaction does not contain PoP publication data");
}

bool checkBtcBlocks(const std::vector<BtcBlock>& btcBlock,
                    ValidationState& state) {
  if (btcBlock.empty()) {
    return true;
  }

  uint256 lastHash = btcBlock[0].getHash();
  for (size_t i = 1; i < btcBlock.size(); ++i) {
    if (!checkBtcBlock(btcBlock[i], state)) {
      return state.addStackFunction("checkBitcoinBlocks()");
    }

    // Check that it's the next height and affirms the previous hash
    if (btcBlock[i].previousBlock != lastHash.reverse()) {
      return state.Invalid("checkBitcoinBlocks()",
                           "Invalid Vbk Pop transaction",
                           "Blocks are not contiguous");
    }
    lastHash = btcBlock[i].getHash();
  }
  return true;
}

bool checkVbkBlocks(const std::vector<VbkBlock>& vbkBlocks,
                    ValidationState& state) {
  if (vbkBlocks.empty()) {
    return true;
  }

  int32_t lastHeight = vbkBlocks[0].height;
  uint192 lastHash = vbkBlocks[0].getHash();

  for (size_t i = 0; i < vbkBlocks.size(); ++i) {
    if (!checkVbkBlock(vbkBlocks[i], state)) {
      return state.addStackFunction("checkVeriBlockBlocks()");
    }

    if (vbkBlocks[i].height != lastHeight + 1 ||
        vbkBlocks[i].getHash() != lastHash.reverse()) {
      return state.Invalid("checkVeriBlockBlocks()",
                           "VeriBlock Blocks invalid",
                           "Blocks are not contiguous");
    }
    lastHeight = vbkBlocks[i].height;
    lastHash = vbkBlocks[i].getHash();
  }
  return true;
}

bool checkProofOfWork(const BtcBlock& block, ValidationState& state) {
  ArithUint256 blockHash(block.getHash());
  ArithUint256 target;
  target.decodeBits(block.bits);
  if (target <= blockHash) {
    return state.Invalid("checkProofOfWork()",
                         "Invalid Btc Block",
                         "Block hash is higher than target");
  }
  return true;
}

bool checkProofOfWork(const VbkBlock& block, ValidationState& state) {
  ArithUint256 blockHash(block.getHash());
  ArithUint256 target;
  target.decodeBits(block.difficulty);
  target = MAXIMUM_DIFFICULTY / target;
  if (target <= blockHash) {
    return state.Invalid("checkProofOfWork()",
                         "Invalid VbkBlock Block",
                         "Block hash is higher than target");
  }
  return true;
}

bool checkVbkPopTx(const VbkPopTx& tx, ValidationState& state) {
  if (!checkSignature(tx, state)) {
    return state.addStackFunction("checkVbkPopTx()");
  }

  if (!checkBitcoinTransactionForPoPData(tx, state)) {
    return state.addStackFunction("checkVbkPopTx()");
  }

  if (!checkMerklePath(tx.merklePath,
                       tx.bitcoinTransaction.getHash(),
                       tx.blockOfProof.merkleRoot.reverse(),
                       state)) {
    return state.addStackFunction("checkVbkPopTx()");
  }

  if (!checkBtcBlocks(tx.blockOfProofContext, state)) {
    return state.addStackFunction("checkVbkPopTx()");
  }

  return true;
}

bool checkVbkTx(const VbkTx& tx, ValidationState& state) {
  if (!checkSignature(tx, state)) {
    return state.addStackFunction("checkVbkTx()");
  }
  return true;
}

bool checkBtcBlock(const BtcBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block, state)) {
    return state.addStackFunction("checkBtcBlock()");
  }

  if (!checkMaximumDrift(block, state)) {
    return state.addStackFunction("checkBtcBlock()");
  }

  return true;
}

bool checkVbkBlock(const VbkBlock& block, ValidationState& state) {
  if (!checkProofOfWork(block, state)) {
    return state.addStackFunction("checkVbkBlock()");
  }

  if (!checkMaximumDrift(block, state)) {
    return state.addStackFunction("checkVbkBlock()");
  }
  return true;
}

bool checkSignature(const VbkTx& tx, ValidationState& state) {
  if (!tx.sourceAddress.isDerivedFromPublicKey(tx.publicKey)) {
    return state.Invalid("checkSignature()",
                         "Invalid Vbk transaction",
                         "Vbk transaction contains an invalid public key");
  }

  if (!veriBlockVerify(
          Slice<const uint8_t>(tx.getHash().data(), tx.getHash().size()),
          tx.signature,
          publicKeyFromVbk(tx.publicKey))) {
    return state.Invalid("checkSignature()",
                         "Vbk transaction",
                         "Vbk transaction is incorrectly signed");
  }
  return true;
}

bool checkSignature(const VbkPopTx& tx, ValidationState& state) {
  if (!tx.address.isDerivedFromPublicKey(tx.publicKey)) {
    return state.Invalid("checkSignature()",
                         "Invalid Vbk Pop transaction",
                         "Vbk Pop transaction contains an invalid public key");
  }

  if (!veriBlockVerify(
          Slice<const uint8_t>(tx.getHash().data(), tx.getHash().size()),
          tx.signature,
          publicKeyFromVbk(tx.publicKey))) {
    return state.Invalid("checkSignature()",
                         "Invalid Vbk Pop transaction",
                         "Vbk Pop transaction is incorrectly signed");
  }
  return true;
}

bool checkATV(const ATV& atv, ValidationState& state) {
  if (!checkVbkTx(atv.transaction, state)) {
    return state.addStackFunction("checkATV()");
  }
  if (!checkMerklePath(atv.merklePath,
                       atv.transaction.getHash(),
                       atv.containingBlock.merkleRoot,
                       state)) {
    return state.addStackFunction("checkATV()");
  }

  if (!checkVbkBlocks(atv.context, state)) {
    return state.addStackFunction("checkATV()");
  }

  return true;
}

bool checkVTB(const VTB& vtb, ValidationState& state) {
  if (!checkVbkPopTx(vtb.transaction, state)) {
    return state.addStackFunction("checkVTB()");
  }

  if (!checkMerklePath(vtb.merklePath,
                       vtb.transaction.getHash(),
                       vtb.containingBlock.merkleRoot,
                       state)) {
    return state.addStackFunction("checkVTB()");
  }

  if (!checkVbkBlocks(vtb.context, state)) {
    return state.addStackFunction("checkVTB()");
  }

  return true;
}

}  // namespace VeriBlock

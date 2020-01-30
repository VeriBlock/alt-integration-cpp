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

bool checkBitcoinBlocks(const std::vector<BtcBlock>& btcBlock,
                        ValidationState& state) {
  uint256 lashHash;
  for (const auto& block : btcBlock) {
    if (!checkBtcBlock(block, state)) {
      return state.addStackFunction("checkBitcoinBlocks()");
    }

    if (lashHash != uint256()) {
      // Check that it's the next height and affirms the previous hash
      if (block.previousBlock != lashHash.reverse()) {
        return state.Invalid("checkBitcoinBlocks()",
                             "Invalid Vbk Pop transaction",
                             "Blocks are not contiguous");
      }
    }
    lashHash = block.getHash();
  }
  return true;
}

bool checkVeriBlockBlocks(const std::vector<VbkBlock>& vbkBlocks,
                          ValidationState& state) {
  int32_t lastHeight = 0;
  uint192 lastHash;

  for (const auto& block : vbkBlocks) {
    if (!checkVbkBlock(block, state)) {
      return state.addStackFunction("checkVeriBlockBlocks()");
    }

    if (lastHash != uint192()) {
      if (block.height != lastHeight + 1 ||
          block.getHash() != lastHash.reverse()) {
        return state.Invalid("checkVeriBlockBlocks()",
                             "VeriBlock Blocks invalid",
                             "Blocks are not contiguous");
      }
    }
    lastHeight = block.height;
    lastHash = block.getHash();
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

  if (!checkBitcoinBlocks(tx.blockOfProofContext, state)) {
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
  if (!tx.sourceAddress.isDerivedFromPublicKey(
          Slice<const uint8_t>(tx.publicKey))) {
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
  if (!tx.address.isDerivedFromPublicKey(Slice<const uint8_t>(tx.publicKey))) {
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

  if (!checkVeriBlockBlocks(atv.context, state)) {
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

  if (!checkVeriBlockBlocks(vtb.context, state)) {
    return state.addStackFunction("checkVTB()");
  }

  return true;
}

}  // namespace VeriBlock

#include <algorithm>
#include <bitset>
#include <string>
#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blob.hpp"
#include "veriblock/consts.hpp"
#include "veriblock/stateless_validation.hpp"
#include "veriblock/strutil.hpp"

namespace {

int getIntFromBits(const std::vector<bool>& bits,
                   const uint32_t& fromIndex,
                   const uint32_t& toIndex) {
  int value = 0;
  for (size_t j = fromIndex; j < toIndex; ++j) {
    if (bits[j]) {
      value += 1 << (j - fromIndex);
    }
  }
  return value;
}

};  // namespace

namespace VeriBlock {

static const ArithUint256 MAXIMUM_DIFFICULTY(
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

bool containsSplit(const std::vector<uint8_t>& pop_data,
                   const std::vector<uint8_t>& btcTx_data) {
  static const std::vector<uint8_t> magicBytes = {0x92, 0x7a, 0x59};

  ReadStream buffer(btcTx_data);

  try {
    size_t lastPos = 0;
    while (buffer.remaining() > 5) {
      if (buffer.readBE<uint8_t>() != magicBytes[0] ||
          buffer.readBE<uint8_t>() != magicBytes[1] ||
          buffer.readBE<uint8_t>() != magicBytes[2]) {
        continue;
      }

      lastPos = buffer.position();
      // Parse the first byte to get the number of chunks, their positions and
      // lengths
      uint8_t descriptor = buffer.readBE<uint8_t>();

      uint32_t chunks = 0;
      uint32_t offsetLength = 4;
      uint32_t sectionLength = 4;
      for (int i = 0; i < 8; ++i) {
        if ((descriptor >> i) & 1) {
          if (i < 2) {
            sectionLength += 1 << i;
          } else if (i < 4) {
            offsetLength += 1 << i;
          } else {
            chunks += 1 << (i - 4);
          }
        }
      }

      // Parse the actual chunk descriptors now that we know the sizes

      uint32_t chunkDescriptorBitLength =
          (chunks * offsetLength) + (sectionLength * (chunks - 1));
      uint32_t chunkDescriptorBytesLength =
          (chunkDescriptorBitLength + 8 - (chunkDescriptorBitLength % 8)) / 8;
      uint32_t waste =
          chunkDescriptorBytesLength * 8 - chunkDescriptorBitLength;

      std::vector<uint8_t> chunkDescriptorBytes =
          buffer.readSlice(chunkDescriptorBytesLength).reverse();
      std::vector<bool> chunkDescriptor;
      // Read from Slice bits
      uint32_t chunkDescriptorLength = 0;
      for (const auto& i1 : chunkDescriptorBytes) {
        std::bitset<8> temp(i1);
        for (uint32_t i2 = 0; i2 < 8; ++i2) {
          chunkDescriptor.push_back(temp[i2]);
          if (temp[i2]) {
            chunkDescriptorLength = (uint32_t)chunkDescriptor.size();
          }
        }
      }

      int totalBytesRead = 0;
      buffer.setPosition(0);
      std::vector<uint8_t> extracted;
      for (int i = chunks - 1; i >= 0; --i) {
        uint32_t chunkOffset = waste + (i * (offsetLength + sectionLength));
        uint32_t limit =
            std::min(chunkDescriptorLength, chunkOffset + offsetLength);
        int sectionOffsetValue =
            ::getIntFromBits(chunkDescriptor, chunkOffset, limit);

        uint32_t sectionLengthValue = 0;
        if (i == 0) {
          sectionLengthValue = (uint32_t)pop_data.size() - totalBytesRead;
        } else {
          sectionLengthValue = ::getIntFromBits(
              chunkDescriptor, chunkOffset - sectionLength, chunkOffset);
        }
        buffer.setPosition(buffer.position() + sectionOffsetValue);
        Slice<const uint8_t> bytes = buffer.readSlice(sectionLengthValue);
        totalBytesRead += sectionLengthValue;
        extracted.insert(extracted.end(), bytes.begin(), bytes.end());
      }

      if (pop_data == extracted) {
        return true;
      }
      buffer.setPosition(lastPos);
    }
  } catch (const std::exception&) {
  }

  return false;
}  // namespace VeriBlock

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

  if (!containsSplit(stream.data(), tx.bitcoinTransaction.tx)) {
    return state.Invalid(
        "checkBitcoinTransactionForPoPData()"
        "Invalid Vbk Pop transaction",
        "Bitcoin transaction does not contain PoP publication data");
  }

  return true;
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

  for (size_t i = 1; i < vbkBlocks.size(); ++i) {
    if (!checkVbkBlock(vbkBlocks[i], state)) {
      return state.addStackFunction("checkVeriBlockBlocks()");
    }

    if (vbkBlocks[i].height != lastHeight + 1 ||
        vbkBlocks[i].previousBlock.reverse() !=
            trim<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE, VBLAKE_BLOCK_HASH_SIZE>(
                lastHash)) {
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

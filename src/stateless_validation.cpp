// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/stateless_validation.hpp"

#include <algorithm>
#include <bitset>
#include <string>
#include <unordered_set>
#include <vector>
#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/alt_chain_params.hpp>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/blob.hpp"
#include "veriblock/consts.hpp"
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

namespace altintegration {

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
      auto descriptor = buffer.readBE<uint8_t>();

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
            (std::min)(chunkDescriptorLength, chunkOffset + offsetLength);
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
  VBK_ASSERT(stream.data().size() == 80);

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
        "invalid-vbk-pop-tx",
        "Bitcoin transaction does not contain PoP publication data");
  }

  return true;
}

bool checkBtcBlocks(const std::vector<BtcBlock>& btcBlock,
                    ValidationState& state,
                    const BtcChainParams& params) {
  if (btcBlock.empty()) {
    return true;
  }

  if (!checkBlock(btcBlock[0], state, params)) {
    return state.Invalid("vbk-check-block");
  }

  uint256 lastHash = btcBlock[0].getHash();
  for (size_t i = 1; i < btcBlock.size(); ++i) {
    if (!checkBlock(btcBlock[i], state, params)) {
      return state.Invalid("btc-check-block");
    }

    // Check that it's the next height and affirms the previous hash
    if (btcBlock[i].getPreviousBlock() != lastHash) {
      return state.Invalid("invalid-btc-block", "Blocks are not contiguous");
    }
    lastHash = btcBlock[i].getHash();
  }
  return true;
}

bool checkVbkBlocks(const std::vector<VbkBlock>& vbkBlocks,
                    ValidationState& state,
                    const VbkChainParams& params) {
  if (vbkBlocks.empty()) {
    return true;
  }

  if (!checkBlock(vbkBlocks[0], state, params)) {
    return state.Invalid("vbk-check-block");
  }

  int32_t lastHeight = vbkBlocks[0].getHeight();
  auto lastHash = vbkBlocks[0].getHash();

  for (size_t i = 1; i < vbkBlocks.size(); ++i) {
    if (!checkBlock(vbkBlocks[i], state, params)) {
      return state.Invalid("vbk-check-block");
    }

    if (vbkBlocks[i].getHeight() != lastHeight + 1 ||
        vbkBlocks[i].getPreviousBlock() !=
            lastHash.template trimLE<VBLAKE_PREVIOUS_BLOCK_HASH_SIZE>()) {
      return state.Invalid("invalid-vbk-block", "Blocks are not contiguous");
    }
    lastHeight = vbkBlocks[i].getHeight();
    lastHash = vbkBlocks[i].getHash();
  }
  return true;
}

bool checkProofOfWork(const BtcBlock& block, const BtcChainParams& param) {
  ArithUint256 blockHash = ArithUint256::fromLEBytes(block.getHash());
  auto powLimit = ArithUint256(param.getPowLimit());
  bool negative = false;
  bool overflow = false;
  auto target =
      ArithUint256::fromBits(block.getDifficulty(), &negative, &overflow);

  if (negative || overflow || target == 0 || target > powLimit) {
    return false;
  }

  return !(blockHash > target);
}

bool checkProofOfWork(const VbkBlock& block, const VbkChainParams& param) {
  static const auto max = ArithUint256::fromHex(VBK_MAXIMUM_DIFFICULTY);
  auto blockHash = ArithUint256::fromLEBytes(block.getHash());
  auto minDiff = ArithUint256(param.getMinimumDifficulty());
  bool negative = false;
  bool overflow = false;
  auto target =
      ArithUint256::fromBits(block.getDifficulty(), &negative, &overflow);

  if (negative || overflow || target == 0 || target < minDiff) {
    return false;
  }

  target = max / target;

  return !(blockHash > target);
}

bool checkVbkPopTx(const VbkPopTx& tx,
                   ValidationState& state,
                   const BtcChainParams& btc) {
  if (!checkSignature(tx, state)) {
    return state.Invalid("vbk-check-signature");
  }

  if (!checkBitcoinTransactionForPoPData(tx, state)) {
    return state.Invalid("vbk-check-btc-tx-for-pop");
  }

  if (!checkMerklePath(tx.merklePath,
                       tx.bitcoinTransaction.getHash(),
                       tx.blockOfProof.getMerkleRoot().reverse(),
                       state)) {
    return state.Invalid("vbk-check-merkle-path");
  }

  if (!checkBtcBlocks(tx.blockOfProofContext, state, btc)) {
    return state.Invalid("vbk-check-btc-blocks");
  }

  if (tx.blockOfProofContext.size() > MAX_CONTEXT_COUNT_VBK_PUBLICATION) {
    return state.Invalid(
        "vbk-btc-context-too-many",
        fmt::format("Maximum allowed BTC context size is {}, got {}",
                    MAX_CONTEXT_COUNT_VBK_PUBLICATION,
                    tx.blockOfProofContext.size()));
  }

  return true;
}

bool checkVbkTx(const VbkTx& tx, ValidationState& state) {
  if (!checkSignature(tx, state)) {
    return state.Invalid("vbktx-check-signature");
  }

  if (tx.outputs.size() > MAX_OUTPUTS_COUNT) {
    return state.Invalid(
        "vbktx-too-many-outputs",
        fmt::format("Too many outputs. Expected less than {}, got {}",
                    MAX_OUTPUTS_COUNT,
                    tx.outputs.size()));
  }

  return true;
}

bool checkSignature(const VbkTx& tx, ValidationState& state) {
  if (!tx.sourceAddress.isDerivedFromPublicKey(tx.publicKey)) {
    return state.Invalid("invalid-vbk-tx",
                         "Vbk transaction contains an invalid public key");
  }

  auto hash = tx.getHash();
  if (!secp256k1::verify(
          hash, tx.signature, secp256k1::publicKeyFromVbk(tx.publicKey))) {
    return state.Invalid("invalid-vbk-tx",
                         "Vbk transaction is incorrectly signed");
  }
  return true;
}

bool checkSignature(const VbkPopTx& tx, ValidationState& state) {
  if (!tx.address.isDerivedFromPublicKey(tx.publicKey)) {
    return state.Invalid("invalid-vbk-pop-tx",
                         "Vbk Pop transaction contains an invalid public key");
  }
  auto hash = tx.getHash();
  if (!secp256k1::verify(
          hash, tx.signature, secp256k1::publicKeyFromVbk(tx.publicKey))) {
    return state.Invalid("invalid-vbk-pop-tx",
                         "Vbk Pop transaction is incorrectly signed");
  }
  return true;
}

bool checkATV(const ATV& atv,
              ValidationState& state,
              const AltChainParams& altp) {
  if (atv.checked) {
    // we've already checked that ATV
    return true;
  }

  auto id = atv.transaction.publicationData.identifier;
  auto eid = altp.getIdentifier();
  if (eid != id) {
    return state.Invalid("atv-bad-identifier",
                         "Wrong chain identifier. Expected " +
                             std::to_string(eid) + ", got " +
                             std::to_string(id) + ".");
  }

  if (!checkVbkTx(atv.transaction, state)) {
    return state.Invalid("vbk-check-tx");
  }

  if (!checkMerklePath(atv.merklePath,
                       atv.transaction.getHash(),
                       atv.blockOfProof.getMerkleRoot(),
                       state)) {
    return state.Invalid("vbk-check-merkle-path");
  }

  atv.checked = true;

  return true;
}

bool checkVTB(const VTB& vtb,
              ValidationState& state,
              const BtcChainParams& btc) {
  if (vtb.checked) {
    // we've already checked that VTB
    return true;
  }

  if (!checkVbkPopTx(vtb.transaction, state, btc)) {
    return state.Invalid("vbk-check-pop-tx");
  }

  if (!checkMerklePath(vtb.merklePath,
                       vtb.transaction.getHash(),
                       vtb.containingBlock.getMerkleRoot(),
                       state)) {
    return state.Invalid("vbk-check-merkle-path");
  }

  vtb.checked = true;

  return true;
}

bool checkBlock(const VbkBlock& block,
                ValidationState& state,
                const VbkChainParams& params) {
  if (!checkProofOfWork(block, params)) {
    return state.Invalid("vbk-bad-pow", "Invalid Block proof of work");
  }

  return true;
}

bool checkBlock(const BtcBlock& block,
                ValidationState& state,
                const BtcChainParams& params) {
  if (!checkProofOfWork(block, params)) {
    return state.Invalid("btc-bad-pow", "Invalid Block proof of work");
  }

  return true;
}

template <typename I>
bool hasDuplicateIds(const std::vector<I> payloads) {
  std::unordered_set<std::vector<uint8_t>> ids;
  for (const auto& payload : payloads) {
    const auto id = getIdVector(payload);
    bool inserted = ids.insert(id).second;
    if (!inserted) {
      return true;
    }
  }
  return false;
}

bool checkPopDataForDuplicates(const PopData& popData, ValidationState& state) {
  if (hasDuplicateIds(popData.context)) {
    return state.Invalid("popdata-duplicate-vbk", "duplicate VBK blocks");
  }
  if (hasDuplicateIds(popData.vtbs)) {
    return state.Invalid("popdata-duplicate-vtb", "duplicate VTBs");
  }
  if (hasDuplicateIds(popData.atvs)) {
    return state.Invalid("popdata-duplicate-atv", "duplicate ATVs");
  }

  return true;
}

bool checkPopData(PopValidator& validator,
                  const PopData& popData,
                  ValidationState& state) {
  if (!checkPopDataForDuplicates(popData, state)) {
    return state.Invalid("pop-statelessly-invalid-has-duplicates");
  }

  std::vector<std::future<ValidationState>> results;
  results.reserve(popData.context.size() + popData.vtbs.size() +
                  popData.atvs.size());

  for (const auto& b : popData.context) {
    results.push_back(validator.addCheck(b));
  }
  for (const auto& vtb : popData.vtbs) {
    results.push_back(validator.addCheck(vtb));
  }
  for (const auto& atv : popData.atvs) {
    results.push_back(validator.addCheck(atv));
  }
  for (auto& r : results) {
    auto result = r.get();
    if (!result.IsValid()) {
      state = result;
      return state.Invalid("pop-statelessly-invalid");
    }
  }

  return true;
}

}  // namespace altintegration

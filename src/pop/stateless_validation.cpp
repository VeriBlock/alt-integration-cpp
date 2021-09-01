// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <bitset>
#include <unordered_set>
#include <vector>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/blob.hpp>
#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/stateless_validation.hpp>
#include <veriblock/pop/trace.hpp>

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

#define VBK_COMPARE_MAGIC(i, magic)             \
  {                                             \
    uint8_t byte = 0;                           \
    if (!buffer.readBE<uint8_t>(byte, state)) { \
      return state.Invalid("bad-magic", i);     \
    }                                           \
    if (byte != magic) {                        \
      continue;                                 \
    }                                           \
  }

#define VBK_READ_OR_RETURN(type, result, msg)

bool containsSplit(const std::vector<uint8_t>& pop_data,
                   const std::vector<uint8_t>& btcTx_data,
                   ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;

  static const std::array<uint8_t, 3> magic = {0x92, 0x7a, 0x59};

  ReadStream buffer(btcTx_data);

  size_t lastPos = 0;
  while (buffer.remaining() > 5) {
    VBK_COMPARE_MAGIC(0, magic[0]);
    VBK_COMPARE_MAGIC(1, magic[1]);
    VBK_COMPARE_MAGIC(2, magic[2]);

    lastPos = buffer.position();
    // Parse the first byte to get the number of chunks, their positions and
    // lengths
    uint8_t descriptor = 0;
    if (!buffer.readBE<uint8_t>(descriptor, state)) {
      return state.Invalid("bad-descriptor");
    }

    uint32_t chunks = 0;
    uint32_t offsetLength = 4;
    uint32_t sectionLength = 4;
    for (int i = 0; i < 8; ++i) {
      if (((descriptor >> i) & 1) != 0) {
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
    uint32_t waste = chunkDescriptorBytesLength * 8 - chunkDescriptorBitLength;

    Slice<const uint8_t> out;
    if (!buffer.readSlice(chunkDescriptorBytesLength, out, state)) {
      return state.Invalid("bad-descriptor-bytes");
    }

    std::vector<uint8_t> chunkDescriptorBytes = out.reverse();
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
          getIntFromBits(chunkDescriptor, chunkOffset, limit);

      uint32_t sectionLengthValue = 0;
      if (i == 0) {
        sectionLengthValue = (uint32_t)pop_data.size() - totalBytesRead;
      } else {
        sectionLengthValue = ::getIntFromBits(
            chunkDescriptor, chunkOffset - sectionLength, chunkOffset);
      }
      buffer.setPosition(buffer.position() + sectionOffsetValue);
      Slice<const uint8_t> bytes;
      if (!buffer.readSlice(sectionLengthValue, bytes, state)) {
        return state.Invalid("bad-section-length", i);
      }
      totalBytesRead += sectionLengthValue;
      extracted.insert(extracted.end(), bytes.begin(), bytes.end());
    }

    if (pop_data == extracted) {
      return true;
    }

    buffer.setPosition(lastPos);
  }

  return false;
}  // namespace VeriBlock

bool checkBitcoinTransactionForPoPData(const VbkPopTx& tx,
                                       ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;

  WriteStream stream;
  tx.publishedBlock.toRaw(stream);
  tx.address.getPopBytes(stream);
  if (stream.data().size() != VBK_PUBLICATIONDATA_SIZE) {
    return state.Invalid("bad-pubdata",
                         format("Expected size for pubdata is {}, got {}",
                                VBK_PUBLICATIONDATA_SIZE,
                                stream.data().size()));
  }

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

  if (!containsSplit(stream.data(), tx.bitcoinTransaction.tx, state)) {
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

  VBK_TRACE_ZONE_SCOPED;

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

  VBK_TRACE_ZONE_SCOPED;

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
            lastHash.template trimLE<VBK_PREVIOUS_BLOCK_HASH_SIZE>()) {
      return state.Invalid("invalid-vbk-block", "Blocks are not contiguous");
    }
    lastHeight = vbkBlocks[i].getHeight();
    lastHash = vbkBlocks[i].getHash();
  }
  return true;
}

bool checkProofOfWork(const BtcBlock& block, const BtcChainParams& param) {
  VBK_TRACE_ZONE_SCOPED;

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
  VBK_TRACE_ZONE_SCOPED;

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
                   const BtcChainParams& btc,
                   const VbkChainParams& vbk) {
  VBK_TRACE_ZONE_SCOPED;

  // least expensive checks at start
  if (tx.blockOfProofContext.size() > MAX_BTC_BLOCKS_IN_VBKPOPTX) {
    return state.Invalid(
        "vbk-btc-context-too-many",
        format("Maximum allowed BTC context size is {}, got {}",
               MAX_BTC_BLOCKS_IN_VBKPOPTX,
               tx.blockOfProofContext.size()));
  }

  if (tx.networkOrType.networkType != vbk.getTransactionMagicByte()) {
    return state.Invalid("vbkpoptx-bad-tx-byte",
                         format("Bad magic byte. Expected {}, got {}",
                                vbk.getTransactionMagicByte().toPrettyString(),
                                tx.networkOrType.networkType.toPrettyString()));
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

  // most expensive checks in the end
  if (!checkSignature(tx, state)) {
    return state.Invalid("vbk-check-signature");
  }

  return true;
}

bool checkPublicationData(const PublicationData& pub,
                          const AltChainParams& params,
                          ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;

  if (pub.identifier != params.getIdentifier()) {
    return state.Invalid(
        "bad-altchain-id",
        format(
            "Expected id={}, got={}", params.getIdentifier(), pub.identifier));
  }

  ReadStream stream(pub.contextInfo);
  AuthenticatedContextInfoContainer c;
  if (!DeserializeFromVbkEncoding(stream, c, state)) {
    return state.Invalid("bad-contextinfo");
  }

  auto root = c.getTopLevelMerkleRoot();
  if (!params.checkBlockHeader(pub.header, root.asVector(), state)) {
    return state.Invalid("bad-endorsed-header",
                         "Bad endorsed header or unauthenticated contextInfo");
  }

  return true;
}

bool checkVbkTx(const VbkTx& tx,
                const AltChainParams& params,
                const VbkChainParams& vbkparams,
                ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;

  if (tx.outputs.size() > MAX_OUTPUTS_COUNT) {
    return state.Invalid(
        "vbktx-too-many-outputs",
        format("Too many outputs. Expected less than {}, got {}",
               MAX_OUTPUTS_COUNT,
               tx.outputs.size()));
  }

  if (tx.networkOrType.networkType != vbkparams.getTransactionMagicByte()) {
    return state.Invalid(
        "vbktx-bad-tx-byte",
        format("Bad magic byte. Expected {}, got {}",
               vbkparams.getTransactionMagicByte().toPrettyString(),
               tx.networkOrType.networkType.toPrettyString()));
  }

  if (!checkPublicationData(tx.publicationData, params, state)) {
    return state.Invalid("vbktx-bad-publicationdata");
  }

  // most expensive check in the end
  if (!checkSignature(tx, state)) {
    return state.Invalid("vbktx-check-signature");
  }

  return true;
}

bool checkSignature(const VbkTx& tx, ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED_N("CheckSignatureVbkTx");

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
  VBK_TRACE_ZONE_SCOPED_N("CheckSignatureVbkPopTx");
  if (!tx.address.isDerivedFromPublicKey(tx.publicKey)) {
    return state.Invalid("invalid-vbk-pop-tx",
                         "Vbk Pop transaction contains an invalid public key");
  }
  auto hash = tx.getHash();
  if (!secp256k1::verify(
          hash, tx.signature, secp256k1::publicKeyFromVbk(tx.publicKey))) {
    return state.Invalid("invalid-vbk-pop-tx", "Invalid signature in VbkPopTx");
  }
  return true;
}

bool checkATV(const ATV& atv,
              ValidationState& state,
              const AltChainParams& altp,
              const VbkChainParams& vbkp) {
  if (atv.checked) {
    // we've already checked that ATV
    return true;
  }

  VBK_TRACE_ZONE_SCOPED;

  if (!checkVbkTx(atv.transaction, altp, vbkp, state)) {
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
              const BtcChainParams& btc,
              const VbkChainParams& vbk) {
  if (vtb.checked) {
    // we've already checked that VTB
    return true;
  }

  VBK_TRACE_ZONE_SCOPED;

  if (!checkVbkPopTx(vtb.transaction, state, btc, vbk)) {
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

bool checkVbkBlockPlausibility(const VbkBlock& block,
                               ValidationState& state,
                               const VbkChainParams& params) {
  VBK_TRACE_ZONE_SCOPED;

  const auto progPowForkHeight = params.getProgPowForkHeight();
  if (block.getHeight() < progPowForkHeight) {
    return state.Invalid("height-too-low",
                         format("Too low height. Supporting blocks "
                                "starting with height={}, got height={}",
                                params.getProgPowForkHeight(),
                                block.getHeight()));
  }

  const auto epoch = progpow::ethashGetEpochWithoutOffset(block.getHeight());
  if (epoch > VBK_MAX_CALCULATED_EPOCHS_SIZE) {
    const auto maxHeight =
        VBK_MAX_CALCULATED_EPOCHS_SIZE * VBK_ETHASH_EPOCH_LENGTH;
    return state.Invalid("height-too-high",
                         format("Too high height. Supporting blocks "
                                "starting with height={}, got height={}",
                                maxHeight,
                                block.getHeight()));
  }

  // if this check id disabled, return early
  if (!params.isProgPowStartTimeEpochEnabled()) {
    return true;
  }

  // estimate blockchain expected block height to get rid of blocks that are
  // too high, thus invalidating how progpow caches.
  // algorithm is copied from Nodecore
  const auto height = block.getHeight();
  const auto blocktimeSeconds = params.getTargetBlockTime();
  const auto startTimeEpoch = params.getProgPowStartTimeEpoch();
  const auto gracePeriodDays = 5;
  const auto timestamp = block.getTimestamp();
  if (timestamp < startTimeEpoch) {
    return state.Invalid(
        "timestamp-too-low",
        format("Too low timestamp. Supporting blocks starting with "
               "timestamp={}, got timestamp={}",
               startTimeEpoch,
               timestamp));
  }

  static const auto secondsInDay = 24 * 60 * 60;

  const uint32_t upperBound =
      startTimeEpoch +
      ((uint64_t)blocktimeSeconds * (height - progPowForkHeight) * 12 / 10) +
      (secondsInDay * gracePeriodDays);

  uint32_t lowerBound =
      startTimeEpoch +
      ((uint64_t)blocktimeSeconds * (height - progPowForkHeight) * 10 / 12) -
      (secondsInDay * gracePeriodDays);

  lowerBound = std::max({lowerBound, startTimeEpoch});

  if (timestamp > upperBound) {
    // Timestamp is more than upper bound, invalid
    return state.Invalid(
        "timestamp-upper-bound",
        format("Timestamp higher than upper bound={}, timestamp={}",
               upperBound,
               timestamp));
  }

  if (timestamp < lowerBound) {
    // Timestamp is less than upper bound, invalid
    return state.Invalid(
        "timestamp-lower-bound",
        format("Timestamp lower than lower bound={}, timestamp={}",
               lowerBound,
               timestamp));
  }

  return true;
}

bool checkBlock(const VbkBlock& block,
                ValidationState& state,
                const VbkChainParams& params) {
  VBK_TRACE_ZONE_SCOPED_N("CheckVbkBlock");
  // before we calculate PoW, determine its plausibility
  if (!checkVbkBlockPlausibility(block, state, params)) {
    return state.Invalid("vbk-bad-block");
  }

  if (!checkProofOfWork(block, params)) {
    return state.Invalid("vbk-bad-pow", "Invalid Block proof of work");
  }

  return true;
}

bool checkBlock(const BtcBlock& block,
                ValidationState& state,
                const BtcChainParams& params) {
  VBK_TRACE_ZONE_SCOPED_N("CheckBtcBlock");
  if (!checkProofOfWork(block, params)) {
    return state.Invalid("btc-bad-pow", "Invalid Block proof of work");
  }

  return true;
}

template <typename P>
static bool hasDuplicatePayloads(const std::vector<P>& payloads) {
  const auto& ids = map_get_id(payloads);
  return hasDuplicateIds<P>(ids);
}

bool checkPopDataForDuplicates(const PopData& popData, ValidationState& state) {
  VBK_TRACE_ZONE_SCOPED;
  if (hasDuplicatePayloads(popData.context)) {
    return state.Invalid("duplicate-vbk", "duplicate VBK blocks");
  }
  if (hasDuplicatePayloads(popData.vtbs)) {
    return state.Invalid("duplicate-vtb", "duplicate VTBs");
  }
  if (hasDuplicatePayloads(popData.atvs)) {
    return state.Invalid("duplicate-atv", "duplicate ATVs");
  }

  return true;
}

bool checkPopData(PopValidator& validator,
                  const PopData& popData,
                  ValidationState& state) {
  if (popData.checked) {
    return true;
  }

  // trace after first check to improve tracing performance
  VBK_TRACE_ZONE_SCOPED;

  auto& altparam = validator.getAltParams();
  size_t estimate_size = popData.estimateSize();
  if (estimate_size > altparam.getMaxPopDataSize()) {
    return state.Invalid("pop-sl-oversize",
                         format("popData raw size more than allowed, "
                                "current size: {}, allowed size: {}",
                                estimate_size,
                                altparam.getMaxPopDataSize()));
  }

  if (popData.context.size() > altparam.getMaxVbkBlocksInAltBlock()) {
    return state.Invalid(
        "pop-sl-context-oversize",
        format("Too many VBK blocks. Expected {} or less, got {}",
               altparam.getMaxVbkBlocksInAltBlock(),
               popData.context.size()));
  }

  if (popData.vtbs.size() > altparam.getMaxVTBsInAltBlock()) {
    return state.Invalid("pop-sl-vtbs-oversize",
                         format("Too many VTBs. Expected {} or less, got {}",
                                altparam.getMaxVTBsInAltBlock(),
                                popData.vtbs.size()));
  }

  if (popData.atvs.size() > altparam.getMaxATVsInAltBlock()) {
    return state.Invalid("pop-sl-atvs-oversize",
                         format("Too many ATVs. Expected {} or less, got {}",
                                altparam.getMaxVTBsInAltBlock(),
                                popData.atvs.size()));
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
      validator.clear();
      state = result;
      return state.Invalid("pop-sl-invalid");
    }
  }

  // this check has to be after parallel stateless validation, as it requires
  // hash to be calculated, and it is faster to calculate hashes in thread pool
  if (!checkPopDataForDuplicates(popData, state)) {
    return state.Invalid("pop-sl-invalid-has-duplicates");
  }

  popData.checked = true;
  return true;
}

}  // namespace altintegration

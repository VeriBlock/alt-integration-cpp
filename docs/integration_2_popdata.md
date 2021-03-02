# Adding PopData. {#integration_2_popdata}

[TOC]

# Overview

We should add new PopData entity into the CBlock class in the block.h file and provide new nVersion flag. It is needed for storing VeriBlock specific information such as ATVs, VTBs, VBKs.
First we will add new POP_BLOCK_VERSION_BIT flag, that will help to distinguish original blocks that don't have any VeriBlock specific data, and blocks that contain such data.
Next, update serialization of the block, that will serialize/deserialize PopData if POP_BLOCK_VERSION_BIT is set. Finally extend serialization/deserialization for the PopData, so we can use native serialization/deserialization.

# 1. Helper for the block hash serialization

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h)

[class base_blob](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h#L20)
```cpp
     {
         s.read((char*)data, sizeof(data));
     }
+
+    std::vector<uint8_t> asVector() const {
+        return std::vector<uint8_t>{begin(), end()};
+    }
 };
```

# 2. Define POP_BLOCK_VERSION_BIT flag.

POP_BLOCK_VERSION_BIT is defined in [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/vbk.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/vbk.hpp). Copy this file to your project.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/primitives/block.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/primitives/block.h)
```cpp
 #include <serialize.h>
 #include <uint256.h>
+#include <vbk/vbk.hpp>
+
+#include "veriblock/entities/popdata.hpp"
```
[class CBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/primitives/block.h#L75)
```cpp
public:
     // network and disk
     std::vector<CTransactionRef> vtx;
+    // VeriBlock  data network and disk
+    altintegration::PopData popData;
```
```cpp
     // memory only
     mutable bool fChecked;
     inline void SerializationOp(Stream& s, Operation ser_action) {
         READWRITEAS(CBlockHeader, *this);
         READWRITE(vtx);
+        if (this->nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+            READWRITE(popData);
+        }
     }

     void SetNull()
     {
         CBlockHeader::SetNull();
         vtx.clear();
+        popData.context.clear();
+        popData.vtbs.clear();
+        popData.atvs.clear();
         fChecked = false;
     }
```

# 3. Add new PopData field into the BlockTransaction, CBlockHeaderAndShortTxIDs, PartiallyDownloadedBlock and update their serialization/deserialization.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h)

[class BlockTransactions](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L70)
```cpp
// A BlockTransactions message
     uint256 blockhash;
     std::vector<CTransactionRef> txn;
+    // VeriBlock data
+    altintegration::PopData popData;
```
```cpp 
             for (size_t i = 0; i < txn.size(); i++)
                 READWRITE(TransactionCompressor(txn[i]));
         }
+
+        // VeriBlock data
+        READWRITE(popData);
     }
```

[class CBlockHeaderAndShortTxIDs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L134)
```cpp
public:
     CBlockHeader header;
+    // VeriBlock data
+    altintegration::PopData popData;
```
```cpp 
             }
         }
 
+        if (this->header.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+            READWRITE(popData);
+        }
+
+
         READWRITE(prefilledtxn);
```
[class PartiallyDownloadedBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L206)
```cpp
 public:
     CBlockHeader header;
+    // VeriBlock data
+    altintegration::PopData popData;
+
     explicit PartiallyDownloadedBlock(CTxMemPool* poolIn) : pool(poolIn) {}
 
     // extra_txn is a list of extra transactions to look at, in <witness hash, reference> form
     ReadStatus InitData(const CBlockHeaderAndShortTxIDs& cmpctblock, const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn);
     bool IsTxAvailable(size_t index) const;
     ReadStatus FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing);
+    ReadStatus FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing, const altintegration::PopData& popData);
 };
```

# 4. Update PartiallyDownloadedBlock initializing - set PopData field.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp)

[method CBlockHeaderAndShortTxIDs::CBlockHeaderAndShortTxIDs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L21)
```cpp
         const CTransaction& tx = *block.vtx[i];
         shorttxids[i - 1] = GetShortID(fUseWTXID ? tx.GetWitnessHash() : tx.GetHash());
     }
+    // VeriBlock
+    this->popData = block.popData;
```
[method PartiallyDownloadedBlock::InitData](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L53)
```cpp
-    LogPrint(BCLog::CMPCTBLOCK, "Initialized PartiallyDownloadedBlock for block %s using a cmpctblock of size %lu\n", cmpctblock.header.GetHash().ToString(), GetSerializeSize(cmpctblock, PROTOCOL_VERSION));
+    // VeriBlock: set pop data
+    this->popData = cmpctblock.popData;
+
+    LogPrint(BCLog::CMPCTBLOCK, "Initialized PartiallyDownloadedBlock for block %s using a cmpctblock of size %lu with %d VBK %d VTB %d ATV\n", cmpctblock.header.GetHash().ToString(), GetSerializeSize(cmpctblock, PROTOCOL_VERSION), this->popData.context.size(), this->popData.vtbs.size(), this->popData.atvs.size());

     return READ_STATUS_OK;
```
[method PartiallyDownloadedBlock::FillBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L184)
```cpp
+ReadStatus PartiallyDownloadedBlock::FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing, const altintegration::PopData& popData) {
+    block.popData = popData;
+    ReadStatus status = FillBlock(block, vtx_missing);
+    return status;
+}
```
[method PartiallyDownloadedBlock::FillBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L190)
```cpp
     if (vtx_missing.size() != tx_missing_offset)
         return READ_STATUS_INVALID;

+    // VeriBlock: set popData before CheckBlock
+    block.popData = this->popData;
+
     BlockValidationState state;
```
```cpp
-    LogPrint(BCLog::CMPCTBLOCK, "Successfully reconstructed block %s with %lu txn prefilled, %lu txn from mempool (incl at least %lu from extra pool) and %lu txn requested\n", hash.ToString(), prefilled_count, mempool_count, extra_count, vtx_missing.size());
+    LogPrint(BCLog::CMPCTBLOCK, "Successfully reconstructed block %s with %lu txn prefilled, %lu txn from mempool (incl at least %lu from extra pool) and %lu txn requested, and %d VBK %d VTB %d ATV\n", hash.ToString(), prefilled_count, mempool_count, extra_count, vtx_missing.size(), this->popData.context.size(), this->popData.vtbs.size(), this->popData.atvs.size());
     if (vtx_missing.size() < 5) {
```

# 5. Update setting up the PopData fields during the net processing.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp)

[method SendBlockTransactions](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1655)
```cpp
     const CNetMsgMaker msgMaker(pfrom->GetSendVersion());
     int nSendFlags = State(pfrom->GetId())->fWantsCmpctWitness ? 0 : SERIALIZE_TRANSACTION_NO_WITNESS;
+
+    //VeriBlock add popData
+    resp.popData = block.popData;
+
     connman->PushMessage(pfrom, msgMaker.Make(nSendFlags, NetMsgType::BLOCKTXN, resp));
```
[method ProcessMessage](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1919)
```cpp
                     BlockTransactions txn;
                     txn.blockhash = cmpctblock.header.GetHash();
+                    txn.popData = cmpctblock.popData;
                     blockTxnMsg << txn;
                     fProcessBLOCKTXN = true;
```
```cpp
                 if (status == READ_STATUS_OK) {
                     fBlockReconstructed = true;
+                    if(pblock && pblock->nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+                        assert(!pblock->popData.empty() && "POP bit is set and POP data is empty");
+                    }
                 }
```
```cpp
             PartiallyDownloadedBlock& partialBlock = *it->second.second->partialBlock;
-            ReadStatus status = partialBlock.FillBlock(*pblock, resp.txn);
+            ReadStatus status = partialBlock.FillBlock(*pblock, resp.txn, resp.popData);
```
```cpp
             ReadCompactSize(vRecv); // ignore tx count; assume it is 0.
+            if (headers[n].nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+                altintegration::PopData tmp;
+                vRecv >> tmp;
+            }
```

# 6. Update validation rules.

Add check that if block contains VeriBlock PopData then block.nVersion must contain POP_BLOCK_VERSION_BIT. Otherwise block.nVersion should not contain POP_BLOCK_VERSION_BIT.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method UpdateTip](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2360)
```cpp
             int32_t nExpectedVersion = ComputeBlockVersion(pindex->pprev, chainParams.GetConsensus());
-            if (pindex->nVersion > VERSIONBITS_LAST_OLD_BLOCK_VERSION && (pindex->nVersion & ~nExpectedVersion) != 0)
+            // do not expect this flag to be set
+            auto version = pindex->nVersion & (~VeriBlock::POP_BLOCK_VERSION_BIT);
+            if (pindex->nVersion > VERSIONBITS_LAST_OLD_BLOCK_VERSION && (version & ~nExpectedVersion) != 0)
                 ++nUpgraded;
             pindex = pindex->pprev;
```
[method CheckBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3376)
```cpp
+    if (block.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT && block.popData.empty()) {
+        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop-version", "POP bit is set, but pop data is empty");
+    }
+    if (!(block.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) && !block.popData.empty()) {
+        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop-version", "POP bit is NOT set, and pop data is NOT empty");
     }
```

# 7. Update the mining code to setup POP_BLOCK_VERSION_BIT if VeriBlock PopData is contained in the block.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp)

[method BlockAssembler::CreateNewBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp#L96)
```cpp
+    if (!pblock->popData.empty()) {
+        pblock->nVersion |= VeriBlock::POP_BLOCK_VERSION_BIT;
+    }

     int64_t nTime1 = GetTimeMicros();
```

# 8. Overload serialization operations for the VeriBlock PopData and other VeriBlock entities.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h)

```cpp
 #include <prevector.h>
 #include <span.h>

+#include <vbk/adaptors/univalue_json.hpp>
+
+#include <veriblock/entities/popdata.hpp>
+#include <veriblock/entities/btcblock.hpp>
+#include <veriblock/entities/altblock.hpp>
+#include <veriblock/blockchain/block_index.hpp>
+#include <veriblock/serde.hpp>
+
 static const unsigned int MAX_SIZE = 0x02000000;
```
```cpp 
+// VeriBlock: Serialize a PopData object
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::PopData& pop_data) {
+    std::vector<uint8_t> bytes_data = pop_data.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
 
+template <typename T>
+void UnserializeOrThrow(const std::vector<uint8_t>& in, T& out) {
+    altintegration::ValidationState state;
+    altintegration::ReadStream stream(in);
+    if(!altintegration::DeserializeFromVbkEncoding(stream, out, state)) {
+        throw std::invalid_argument(state.toString());
+    }
+}
+
+template <typename T>
+void UnserializeOrThrow(const std::vector<uint8_t>& in, T& out, typename T::hash_t precalculatedHash) {
+    altintegration::ValidationState state;
+    altintegration::ReadStream stream(in);
+    if(!altintegration::DeserializeFromVbkEncoding(stream, out, state, precalculatedHash)) {
+        throw std::invalid_argument(state.toString());
+    }
+}
+
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::PopData& pop_data) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, pop_data);
+}
+
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::ATV& atv) {
+    std::vector<uint8_t> bytes_data = atv.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
+
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::ATV& atv) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, atv);
+}
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::VTB& vtb) {
+    std::vector<uint8_t> bytes_data = vtb.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::VTB& vtb) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, vtb);
+}
+
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::BlockIndex<altintegration::BtcBlock>& b) {
+    std::vector<uint8_t> bytes_data = b.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::BlockIndex<altintegration::BtcBlock>& b) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, b);
+}
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::BlockIndex<altintegration::VbkBlock>& b) {
+    std::vector<uint8_t> bytes_data = b.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::BlockIndex<altintegration::VbkBlock>& b) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, b);
+}
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::BlockIndex<altintegration::AltBlock>& b) {
+    std::vector<uint8_t> bytes_data = b.toVbkEncoding();
+    Serialize(s, bytes_data);
+}
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::BlockIndex<altintegration::AltBlock>& b) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, b);
+}
+template<typename Stream, size_t N> inline void Serialize(Stream& s, const altintegration::Blob<N>& b) {
+    Serialize(s, b.asVector());
+}
+template<typename Stream, size_t N> inline void Unserialize(Stream& s, altintegration::Blob<N>& b) {
+    std::vector<uint8_t> bytes;
+    Unserialize(s, bytes);
+    if(bytes.size() > N) {
+        throw std::invalid_argument("Blob: bad size. Expected <= " + std::to_string(N) + ", got=" + std::to_string(bytes.size()));
+    }
+    b = bytes;
+}
+
+template<typename Stream> inline void Serialize(Stream& s, const altintegration::VbkBlock& block) {
+    altintegration::WriteStream stream;
+    block.toVbkEncoding(stream);
+    Serialize(s, stream.data());
+}
+template<typename Stream> inline void Unserialize(Stream& s, altintegration::VbkBlock& block) {
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, block);
+}
+template <typename Stream>
+inline void UnserializeWithHash(Stream& s, altintegration::BlockIndex<altintegration::VbkBlock>& block, const altintegration::VbkBlock::hash_t& precalculatedHash = altintegration::VbkBlock::hash_t())
+{
+    std::vector<uint8_t> bytes_data;
+    Unserialize(s, bytes_data);
+    UnserializeOrThrow(bytes_data, block, precalculatedHash);
+}
```
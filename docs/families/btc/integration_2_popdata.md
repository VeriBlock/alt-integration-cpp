# Adding PopData {#integration_2_popdata}

[TOC]

# Overview

We will add:
1. altintegration::PopData entity into the `CBlock` class in `primitives/block.h` file. This is needed to store POP-related consensus information, such as ATVs, VTBs, and VBk blocks.
2. `POP_BLOCK_VERSION_BIT` flag - to distinguish if `CBlockHeader` has altintegration::PopData or not.
3. Update serialization for `CBlock` to include altintegration::PopData.
4. Add serialization/deserialization code for altintegration::PopData entity.

# 1. Helper for the block hash serialization.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h)

[class base_blob](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/uint256.h#L20)
```cpp
     {
         s.read((char*)data, sizeof(data));
     }
+
+    // helper to build std::vector from uint256
+    std::vector<uint8_t> asVector() const {
+        return std::vector<uint8_t>{begin(), end()};
+    }
 };
```

# 2. Define POP_BLOCK_VERSION_BIT flag.

This flag is set in a block header version and will tell users if given block header contains PopData or not.

In file `src/version.h` define this flag:

```cpp
static const int PROTOCOL_VERSION = 80000;

+namespace VeriBlock {
+static const int32_t POP_BLOCK_VERSION_BIT = 0x80000UL;
+}
```


[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/primitives/block.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/primitives/block.h)
```cpp
 #include <serialize.h>
 #include <uint256.h>
+#include <version.h>
+#include <veriblock/pop.hpp>
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
+        // if bit is set, then add popData to serialization of a block body
+        if (this->nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+            READWRITE(popData);
+        }
     }

     void SetNull()
     {
         CBlockHeader::SetNull();
         vtx.clear();
+        popData.clear();
         fChecked = false;
     }
```

# 3. Add new PopData field into the BlockTransactions, CBlockHeaderAndShortTxIDs, PartiallyDownloadedBlock and update their serialization/deserialization.

Bitcoin uses these classes during sync protocol.
We must ensure that altintegration::PopData is propagated alongside transactions during sync.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h)

[class BlockTransactions](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L70)
```cpp
class BlockTransactions {
public:
    // A BlockTransactions message
    uint256 blockhash;
    std::vector<CTransactionRef> txn;
+   altintegration::PopData popData;
```
```cpp
             for (size_t i = 0; i < txn.size(); i++)
                 READWRITE(TransactionCompressor(txn[i]));
         }
+
+        // add popData to serialization
+        READWRITE(popData);
     }
```

[class CBlockHeaderAndShortTxIDs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L134)
```cpp
public:
     CBlockHeader header;
+    altintegration::PopData popData;
```
```cpp
             }
         }

+        if (this->header.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+            READWRITE(popData);
+        }

         READWRITE(prefilledtxn);
```
[class PartiallyDownloadedBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.h#L206)
```cpp
 public:
     CBlockHeader header;
+    altintegration::PopData popData;

     explicit PartiallyDownloadedBlock(CTxMemPool* poolIn) : pool(poolIn) {}

     // extra_txn is a list of extra transactions to look at, in <witness hash, reference> form
     ReadStatus InitData(const CBlockHeaderAndShortTxIDs& cmpctblock, const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn);
     bool IsTxAvailable(size_t index) const;
     ReadStatus FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing);
+    ReadStatus FillBlock(CBlock& block, const std::vector<CTransactionRef>& vtx_missing, const altintegration::PopData& popData) {
+        block.popData = popData;
+        return FillBlock(block, vtx_missing);
+    }
 };
```

# 4. Update PartiallyDownloadedBlock initializing - set PopData field.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp)

[method CBlockHeaderAndShortTxIDs::CBlockHeaderAndShortTxIDs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L21)
```cpp
         const CTransaction& tx = *block.vtx[i];
         shorttxids[i - 1] = GetShortID(fUseWTXID ? tx.GetWitnessHash() : tx.GetHash());
     }
+    // don't forget to set popData
+    this->popData = block.popData;
```
[method PartiallyDownloadedBlock::InitData](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/blockencodings.cpp#L53)
```cpp
    if (mempool_count == shorttxids.size())
            break;
    }

+   // VeriBlock: set pop data
+   this->popData = cmpctblock.popData;

    return READ_STATUS_OK;
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
             if (status == READ_STATUS_INVALID) {
```
```cpp
          for (unsigned int n = 0; n < nCount; n++) {
             vRecv >> headers[n];
             ReadCompactSize(vRecv); // ignore tx count; assume it is 0.
+            if (headers[n].nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) {
+                altintegration::PopData tmp;
+                vRecv >> tmp;
+            }
          }
```

# 6. Update validation rules.

Add check that if block contains VeriBlock PopData then `block.nVersion` must contain `POP_BLOCK_VERSION_BIT`. Otherwise `block.nVersion` should not contain `POP_BLOCK_VERSION_BIT`.

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

Update `CheckBlock`:
- merkle root verification with POP depends on current state, so this validation has been migrated to `ContextuallyCheckBlock`.
- if POP_BLOCK_VERSION_BIT is set, then PopData MUST exist and be non-empty.

[method CheckBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3376)
```cpp
     // Check that the header is valid (particularly PoW).  This is mostly
     // redundant with the call in AcceptBlockHeader.
     if (!CheckBlockHeader(block, state, consensusParams, fCheckPOW))
        return false;

-    // Check the merkle root.
-    if (fCheckMerkleRoot) {
-        bool mutated;
-        uint256 hashMerkleRoot2 = BlockMerkleRoot(block, &mutated);
-        if (block.hashMerkleRoot != hashMerkleRoot2)
-            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "bad-txnmrklroot", "hashMerkleRoot mismatch");
-
-        // Check for merkle tree malleability (CVE-2012-2459): repeating sequences
-        // of transactions in a block without affecting the merkle root of a block,
-        // while still invalidating it.
-        if (mutated)
-            return state.Invalid(BlockValidationResult::BLOCK_MUTATED, "bad-txns-duplicate", "duplicate transaction");
-    }

+    // VeriBlock: merkle root verification currently depends on a context, so it has been moved to ContextualCheckBlock
+    if ((block.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) && block.popData.empty()) {
+       return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop-version", "POP bit is set, but pop data is empty");
+    }
+    if (!(block.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) && !block.popData.empty()) {
+       return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop-version", "POP bit is NOT set, and pop data is NOT empty");
+    }

     // All potential-corruption validation must be done before we do any
```

# 7. Update the mining code to setup POP_BLOCK_VERSION_BIT if VeriBlock PopData is contained in the block.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp)

[method BlockAssembler::CreateNewBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp#L96)
```cpp
   addPackageTxs<ancestor_score>(nPackagesSelected, nDescendantsUpdated);

+  // VeriBlock: add PopData into the block
+  if (chainparams.isPopActive(nHeight))
+  {
+      pblock->popData = VeriBlock::getPopData(*pindexPrev);
+  }
+
+  if (!pblock->popData.empty()) {
+      pblock->nVersion |= VeriBlock::POP_BLOCK_VERSION_BIT;
+  }
+
   int64_t nTime1 = GetTimeMicros();

   m_last_block_num_txs = nBlockTx;
   m_last_block_weight = nBlockWeight;
```

# 8. Overload serialization operations for the VeriBlock PopData and other VeriBlock entities.

@warning In our example we use VeriBlock-native serialization (function `toVbkEncoding`). It essentially couples Altchain CBlock serialization to VBK encoding. If VBK encoding ever change, Altchain will get hard fork. To mitigate this, it is stongly suggested to implement your own serialization.

During deserialization, if any of internal entities can not be deserialized, throw an exception.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h)

```cpp
 #include <prevector.h>
 #include <span.h>

+#include <veriblock/pop.hpp>
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

[Next Section](./integration_3_forkpoint.md)

# Update P2P protocol {#integration_11_p2p}

[TOC]

# Overview

Original Bitcoin P2P sync protocol does not download blocks if advertized cumulative difficulty is lower than ours. It is no longer valid for Pop aware chains. P2P protocol should be modified to offer and accept all blocks and also should properly process ATV and VTB data.

# 1. Add P2P service files: p2p_sync.hpp, p2p_sync.cpp.

VeriBlock P2P service files contain interactions with Pop mempool, data filtering, misbehaving nodes protection.

P2P header: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/p2p_sync.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/p2p_sync.hpp). Copy this file to your project.

P2P source: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/p2p_sync.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/p2p_sync.cpp). Copy this file to your project.

# 2. Introduce new protocol version.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/version.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/version.h)
```cpp
-static const int PROTOCOL_VERSION = 70015;
+static const int PROTOCOL_VERSION = 80000;
```
```cpp
 static const int INVALID_CB_NO_BAN_VERSION = 70015;

+//! ping p2p msg contains 'best chain'
+static const int PING_BESTCHAIN_VERSION = 80000;
```

# 3. Allow node to download chain with less chainWork.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.h)
```cpp
 bool GetNodeStateStats(NodeId nodeid, CNodeStateStats &stats);

+/** Increase a node's misbehavior score. */
+void Misbehaving(NodeId nodeid, int howmuch, const std::string& message="") EXCLUSIVE_LOCKS_REQUIRED(cs_main);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp)
```cpp
 #include <util/validation.h>
+#include <vbk/p2p_sync.hpp>
```
```cpp
 void EraseOrphansFor(NodeId peer);

-/** Increase a node's misbehavior score. */
-void Misbehaving(NodeId nodeid, int howmuch, const std::string& message="") EXCLUSIVE_LOCKS_REQUIRED(cs_main);
```
[struct CNodeState](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L203)
```cpp
     //! The best known block we know this peer has announced.
-    const CBlockIndex *pindexBestKnownBlock;
+    const CBlockIndex *pindexBestKnownBlock = nullptr;
+    //! The block this peer thinks is current tip.
+    const CBlockIndex *pindexLastAnnouncedBlock = nullptr;
     //! The hash of the last unknown block this peer has announced.
     uint256 hashLastUnknownBlock;
     //! The last full block we both have.
-    const CBlockIndex *pindexLastCommonBlock;
+    const CBlockIndex *pindexLastCommonBlock = nullptr;
+    //! The last full block we both have from announced chain.
+    const CBlockIndex *pindexLastCommonAnnouncedBlock = nullptr;
```
```cpp
+/** Update tracking information about which blocks a peer is assumed to have. */
+static void UpdateBestChainTip(NodeId nodeid, const uint256 &tip) EXCLUSIVE_LOCKS_REQUIRED(cs_main) {
+    CNodeState *state = State(nodeid);
+    assert(state != nullptr);
+
+    const CBlockIndex* pindex = LookupBlockIndex(tip);
+    if (pindex && pindex->nChainWork > 0) {
+        state->pindexLastAnnouncedBlock = pindex;
+        LogPrint(BCLog::NET, "peer=%s: announced best chain %s\n", nodeid, tip.GetHex());
+
+        // announced block is better by chainwork. update pindexBestKnownBlock
+        if(state->pindexBestKnownBlock == nullptr || pindex->nChainWork >= state->pindexBestKnownBlock->nChainWork) {
+            state->pindexBestKnownBlock = pindex;
+        }
+    }
+
+    ProcessBlockAvailability(nodeid);
+}
```
```cpp
-static void FindNextBlocksToDownload(NodeId nodeid, unsigned int count, std::vector<const CBlockIndex*>& vBlocks, NodeId& nodeStaller, const Consensus::Params& consensusParams) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+static void FindNextBlocksToDownload(NodeId nodeid, unsigned int count, std::vector<const CBlockIndex*>& vBlocks, NodeId& nodeStaller, const Consensus::Params& consensusParams,
+    // either pindexBestBlock or pindexLastAnouncedBlock
+    const CBlockIndex* bestBlock,
+    // out parameter: sets last common block
+    const CBlockIndex** lastCommonBlockOut
+    ) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
 {
     if (count == 0)
         return;

     vBlocks.reserve(vBlocks.size() + count);
-    CNodeState *state = State(nodeid);
-    assert(state != nullptr);
-
-    // Make sure pindexBestKnownBlock is up to date, we'll need it.
-    ProcessBlockAvailability(nodeid);

-    if (state->pindexBestKnownBlock == nullptr || state->pindexBestKnownBlock->nChainWork < ::ChainActive().Tip()->nChainWork || state->pindexBestKnownBlock->nChainWork < nMinimumChainWork) {
+    if (bestBlock == nullptr || bestBlock->nChainWork < nMinimumChainWork) {
         // This peer has nothing interesting.
         return;
     }

-    if (state->pindexLastCommonBlock == nullptr) {
+    assert(lastCommonBlockOut);
+
+    if (*lastCommonBlockOut == nullptr) {
         // Bootstrap quickly by guessing a parent of our best tip is the forking point.
         // Guessing wrong in either direction is not a problem.
-        state->pindexLastCommonBlock = ::ChainActive()[std::min(state->pindexBestKnownBlock->nHeight, ::ChainActive().Height())];
+        *lastCommonBlockOut = ::ChainActive()[std::min(bestBlock->nHeight, ::ChainActive().Height())];
     }

     // If the peer reorganized, our previous pindexLastCommonBlock may not be an ancestor
     // of its current tip anymore. Go back enough to fix that.
-    state->pindexLastCommonBlock = LastCommonAncestor(state->pindexLastCommonBlock, state->pindexBestKnownBlock);
-    if (state->pindexLastCommonBlock == state->pindexBestKnownBlock)
+    *lastCommonBlockOut = LastCommonAncestor(*lastCommonBlockOut, bestBlock);
+    if (*lastCommonBlockOut == bestBlock)
         return;

     std::vector<const CBlockIndex*> vToFetch;
-    const CBlockIndex *pindexWalk = state->pindexLastCommonBlock;
+    const CBlockIndex *pindexWalk = *lastCommonBlockOut;
     // Never fetch further than the best block we know the peer has, or more than BLOCK_DOWNLOAD_WINDOW + 1 beyond the last
     // linked block we have in common with this peer. The +1 is so we can detect stalling, namely if we would be able to
     // download that next block if the window were 1 larger.
-    int nWindowEnd = state->pindexLastCommonBlock->nHeight + BLOCK_DOWNLOAD_WINDOW;
-    int nMaxHeight = std::min<int>(state->pindexBestKnownBlock->nHeight, nWindowEnd + 1);
+    int nWindowEnd = (*lastCommonBlockOut)->nHeight + BLOCK_DOWNLOAD_WINDOW;
+    int nMaxHeight = std::min<int>(bestBlock->nHeight, nWindowEnd + 1);
     NodeId waitingfor = -1;
```
[method FindNextBlocksToDownload](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L618)
```cpp
         int nToFetch = std::min(nMaxHeight - pindexWalk->nHeight, std::max<int>(count - vBlocks.size(), 128));
         vToFetch.resize(nToFetch);
-        pindexWalk = state->pindexBestKnownBlock->GetAncestor(pindexWalk->nHeight + nToFetch);
+        pindexWalk = bestBlock->GetAncestor(pindexWalk->nHeight + nToFetch);
```
[method FindNextBlocksToDownload](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L618)
```cpp
             if (pindex->nStatus & BLOCK_HAVE_DATA || ::ChainActive().Contains(pindex)) {
                 if (pindex->HaveTxsDownloaded())
-                    state->pindexLastCommonBlock = pindex;
+                    *lastCommonBlockOut = pindex;
             } else if (mapBlocksInFlight.count(pindex->GetBlockHash()) == 0) {
```
[method ProcessHeadersMessage](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1675)
```cpp
-        if (fCanDirectFetch && pindexLast->IsValid(BLOCK_VALID_TREE) && ::ChainActive().Tip()->nChainWork <= pindexLast->nChainWork) {
+        if (fCanDirectFetch && pindexLast->IsValid(BLOCK_VALID_TREE)
+            // VeriBlock: download the chain suggested by the peer
+            /* && ::ChainActive().Tip()->nChainWork <= pindexLast->nChainWork */) {
             std::vector<const CBlockIndex*> vToFetch;
             const CBlockIndex *pindexWalk = pindexLast;
```
[method PeerLogicValidation::SendMessages](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L3596)
```cpp
         if (!pto->fClient && ((fFetch && !pto->m_limited_node) || !::ChainstateActive().IsInitialBlockDownload()) && state.nBlocksInFlight < MAX_BLOCKS_IN_TRANSIT_PER_PEER) {
             std::vector<const CBlockIndex*> vToDownload;
             NodeId staller = -1;
-            FindNextBlocksToDownload(pto->GetId(), MAX_BLOCKS_IN_TRANSIT_PER_PEER - state.nBlocksInFlight, vToDownload, staller, consensusParams);
+            // VeriBlock: find "blocks to download" in 2 chains: one that has "best chainwork", and second that is reported by peer as best.
+            ProcessBlockAvailability(pto->GetId());
+            // always download chain with higher chainwork
+            if(state.pindexBestKnownBlock) {
+                FindNextBlocksToDownload(pto->GetId(), MAX_BLOCKS_IN_TRANSIT_PER_PEER - state.nBlocksInFlight, vToDownload, staller, consensusParams, state.pindexBestKnownBlock, &state.pindexLastCommonBlock);
+            }
+            // should we fetch announced chain?
+            if(state.pindexLastAnnouncedBlock && state.pindexBestKnownBlock) {
+                // last announced block is by definition always <= chainwork than best known block by chainwork
+                assert(state.pindexLastAnnouncedBlock->nChainWork <= state.pindexBestKnownBlock->nChainWork);
+
+                // are they in the same chain?
+                if (state.pindexBestKnownBlock->GetAncestor(state.pindexLastAnnouncedBlock->nHeight) != state.pindexLastAnnouncedBlock) {
+                    // no, additionally sync 'announced' chain
+                    LogPrint(BCLog::NET, "Requesting announced best chain %d:%s from peer=%d\n", state.pindexLastAnnouncedBlock->GetBlockHash().ToString(),
+                        state.pindexLastAnnouncedBlock->nHeight, pto->GetId());
+                    FindNextBlocksToDownload(pto->GetId(), MAX_BLOCKS_IN_TRANSIT_PER_PEER - state.nBlocksInFlight, vToDownload, staller, consensusParams, state.pindexLastAnnouncedBlock, &state.pindexLastCommonAnnouncedBlock);
+                }
+            }
             for (const CBlockIndex *pindex : vToDownload) {
```

# 4. Update Ping and Pong calls to provide best block hash.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp)

[method ProcessMessage](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1919)
```cpp
             uint64_t nonce = 0;
             vRecv >> nonce;
+
+            if(pfrom->nVersion > PING_BESTCHAIN_VERSION) {
+                // VeriBlock: immediately after nonce, receive best block hash
+                LOCK(cs_main);
+                uint256 bestHash;
+                vRecv >> bestHash;
+                UpdateBestChainTip(pfrom->GetId(), bestHash);
+
+                connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::PONG, nonce, ::ChainActive().Tip()->GetBlockHash()));
+                return true;
+            }
+
             // Echo the message back with the nonce. This allows for two useful features:
```
[method ProcessMessage](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1919)
```cpp
                         sProblem = "Nonce zero";
                     }
                 }
+
+                if(pfrom->nVersion > PING_BESTCHAIN_VERSION) {
+                    LOCK(cs_main);
+                    uint256 bestHash;
+                    vRecv >> bestHash;
+                    UpdateBestChainTip(pfrom->GetId(), bestHash);
+                }
             } else {
                 sProblem = "Unsolicited pong without ping";
             }
```
[method PeerLogicValidation::SendMessages](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L3596)
```cpp
             if (pto->nVersion > BIP0031_VERSION) {
                 pto->nPingNonceSent = nonce;
-                connman->PushMessage(pto, msgMaker.Make(NetMsgType::PING, nonce));
+                if(pto->nVersion > PING_BESTCHAIN_VERSION) {
+                    connman->PushMessage(pto, msgMaker.Make(NetMsgType::PING, nonce, ::ChainActive().Tip()->GetBlockHash()));
+                } else {
+                    connman->PushMessage(pto, msgMaker.Make(NetMsgType::PING, nonce));
+                }
```

# 5. Offer and process Pop data.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp)

[method PeerLogicValidation::FinalizeNode](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L799)
```cpp
     assert(g_outbound_peers_with_protect_from_disconnect >= 0);

     mapNodeState.erase(nodeid);
+    // VeriBlock
+    VeriBlock::p2p::erasePopDataNodeState(nodeid);
```
[method ProcessMessage](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L1919)
```cpp
     }

+    // VeriBlock: if POP is not enabled, ignore POP-related P2P calls
+    int tipHeight = ChainActive().Height();
+    if (Params().isPopActive(tipHeight)) {
+        int pop_res = VeriBlock::p2p::processPopData(pfrom, strCommand, vRecv, connman);
+        if (pop_res >= 0) {
+            return pop_res;
+        }
+    }

     if (!(pfrom->GetLocalServices() & NODE_BLOOM) &&
               (strCommand == NetMsgType::FILTERLOAD ||
```
 [method PeerLogicValidation::SendMessages](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/net_processing.cpp#L3596)
```cpp
         if (!vInv.empty())
             connman->PushMessage(pto, msgMaker.Make(NetMsgType::INV, vInv));

+        // VeriBlock offer Pop Data
+        {
+            VeriBlock::p2p::offerPopData<altintegration::ATV>(pto, connman, msgMaker);
+            VeriBlock::p2p::offerPopData<altintegration::VTB>(pto, connman, msgMaker);
+            VeriBlock::p2p::offerPopData<altintegration::VbkBlock>(pto, connman, msgMaker);
+        }
+
         // Detect whether we're stalling
```

# 6. Subscribe the library to the mempool events.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
 #include <vbk/adaptors/block_provider.hpp>
+#include <vbk/p2p_sync.hpp>
 #include <vbk/pop_common.hpp>
```
[method InitPopContext](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp#L30)
```cpp
     SetPop(payloads_provider);
+
+    auto& app = GetPop();
+    app.getMemPool().onAccepted<altintegration::ATV>(VeriBlock::p2p::offerPopDataToAllNodes<altintegration::ATV>);
+    app.getMemPool().onAccepted<altintegration::VTB>(VeriBlock::p2p::offerPopDataToAllNodes<altintegration::VTB>);
+    app.getMemPool().onAccepted<altintegration::VbkBlock>(VeriBlock::p2p::offerPopDataToAllNodes<altintegration::VbkBlock>);
```

# 7. Add P2P service to the makefile.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```cpp
libbitcoin_server_a_SOURCES = \
   policy/settings.cpp \
   pow.cpp \
   rest.cpp \
+  vbk/p2p_sync.hpp \
+  vbk/p2p_sync.cpp \
   rpc/blockchain.cpp \
```

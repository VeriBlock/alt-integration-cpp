# AltTree management {#integration_7_alttree}

[TOC]

# Overview

At this stage we should add functions for maintaining the VeriBlock AltTree: `setState()`, `acceptBlock()`, `addAllBlockPayloads()`. They provide API to change the state of the VeriBlock AltTree.

- `acceptBlock()` - adds an ALT block into to the library.
- `addAllBlockPayloads()` - adds popData for the current ALT block into the library and should be invoked before `acceptBlock()` call.
- `setState()` - changes the state of the VeriBlock AltTree as if the provided ALT block was the current tip of the blockchain.

# 1. Implement AltTree related methods in the pop_service.hpp and pop_service.cpp source files.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp)
```cpp
#include <vbk/adaptors/payloads_provider.hpp>
#include <vbk/util.hpp>

+class BlockValidationState;
+class CBlock;
 class CBlockTreeDB;
 class CDBIterator;
 class CDBWrapper;
+class CBlockIndex;
+class CChainParams;
+
+namespace Consensus {
+struct Params;
+}
+
 namespace VeriBlock {

+using BlockBytes = std::vector<uint8_t>;
+using PoPRewards = std::map<CScript, CAmount>;
+
 void InitPopContext(CDBWrapper& db);
```
```cpp
 bool loadTrees(CDBWrapper& db);

+//! alttree methods
+bool acceptBlock(const CBlockIndex& indexNew, BlockValidationState& state);
+bool addAllBlockPayloads(const CBlock& block, BlockValidationState& state);
+bool setState(const uint256& block, altintegration::ValidationState& state);
+
+std::vector<BlockBytes> getLastKnownVBKBlocks(size_t blocks);
+std::vector<BlockBytes> getLastKnownBTCBlocks(size_t blocks);
 } // namespace VeriBlock
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
+bool acceptBlock(const CBlockIndex& indexNew, BlockValidationState& state)
+{
+    AssertLockHeld(cs_main);
+    auto containing = VeriBlock::blockToAltBlock(indexNew);
+    altintegration::ValidationState instate;
+    if (!GetPop().getAltBlockTree().acceptBlockHeader(containing, instate)) {
+        LogPrintf("ERROR: alt tree cannot accept block %s\n", instate.toString());
+        return state.Invalid(BlockValidationResult::BLOCK_CACHED_INVALID, instate.GetPath());
+    }
+
+    return true;
+}
+
+bool addAllBlockPayloads(const CBlock& block, BlockValidationState& state) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    auto bootstrapBlockHeight = GetPop().getConfig().getAltParams().getBootstrapBlock().height;
+    auto hash = block.GetHash();
+    auto* index = LookupBlockIndex(hash);
+
+    if (index->nHeight == bootstrapBlockHeight) {
+        // skip bootstrap block block
+        return true;
+    }
+
+    altintegration::ValidationState instate;
+
+    if (!GetPop().check(block.popData, instate)) {
+        return error("[%s] block %s is not accepted because popData is invalid: %s", __func__, block.GetHash().ToString(),
+            instate.toString());
+    }
+
+    GetPop().getAltBlockTree().acceptBlock(block.GetHash().asVector(), block.popData);
+
+    return true;
+}
+
+bool setState(const uint256& block, altintegration::ValidationState& state) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    return GetPop().getAltBlockTree().setState(block.asVector(), state);
+}
+
+std::vector<BlockBytes> getLastKnownVBKBlocks(size_t blocks)
+{
+    LOCK(cs_main);
+    return altintegration::getLastKnownBlocks(GetPop().getVbkBlockTree(), blocks);
+}
+
+std::vector<BlockBytes> getLastKnownBTCBlocks(size_t blocks)
+{
+    LOCK(cs_main);
+    return altintegration::getLastKnownBlocks(GetPop().getBtcBlockTree(), blocks);
+}
```

# 2. Update block processing in the ConnectBlock(), DisconnectBlock(), UpdateTip(), LoadGenesisBlock(), AcceptBlockHeader(), AcceptBlock(), TestBlockValidity().

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method CChainState::DisconnectBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L1661)
```cpp
+    auto prevHash = pindex->pprev->GetBlockHash();
+    altintegration::ValidationState state;
+    VeriBlock::setState(prevHash, state);
+
     // move best block pointer to prevout block
-    view.SetBestBlock(pindex->pprev->GetBlockHash());
+    view.SetBestBlock(prevHash);

     return fClean ? DISCONNECT_OK : DISCONNECT_UNCLEAN;
```
[method CChainState::ConnectBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L1879)
```cpp
+    altintegration::ValidationState _state;
+    if (!VeriBlock::setState(pindex->GetBlockHash(), _state)) {
+        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop", strprintf("Block %s is POP invalid: %s", pindex->GetBlockHash().ToString(), _state.toString()));
     }

     if (!control.Wait()) {
         LogPrintf("ERROR: %s: CheckQueue failed\n", __func__);
         return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "block-validation-failed");
     }
```
[method UpdateTip](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2360)
```cpp
         g_best_block_cv.notify_all();
     }
+    altintegration::ValidationState state;
+    bool ret = VeriBlock::setState(pindexNew->GetBlockHash(), state);
+    assert(ret && "block has been checked previously and should be valid");

     std::string warningMessages;
```
[method UpdateTip](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2360)
```cpp
-    LogPrintf("%s: new best=%s height=%d version=0x%08x log2_work=%.8g tx=%lu date='%s' progress=%f cache=%.1fMiB(%utxo)%s\n", __func__,
-      pindexNew->GetBlockHash().ToString(), pindexNew->nHeight, pindexNew->nVersion,
-      log(pindexNew->nChainWork.getdouble())/log(2.0), (unsigned long)pindexNew->nChainTx,
-      FormatISO8601DateTime(pindexNew->GetBlockTime()),
-      GuessVerificationProgress(chainParams.TxData(), pindexNew), ::ChainstateActive().CoinsTip().DynamicMemoryUsage() * (1.0 / (1<<20)), ::ChainstateActive().CoinsTip().GetCacheSize(),
-      !warningMessages.empty() ? strprintf(" warning='%s'", warningMessages) : "");

+    auto& pop = VeriBlock::GetPop();
+    auto* vbktip = pop.getVbkBlockTree().getBestChain().tip();
+    auto* btctip = pop.getBtcBlockTree().getBestChain().tip();
+    LogPrintf("%s: new best=ALT:%d:%s %s %s version=0x%08x log2_work=%.8g tx=%lu date='%s' progress=%f cache=%.1fMiB(%utxo)%s\n", __func__,
+        pindexNew->nHeight,
+        pindexNew->GetBlockHash().GetHex(),
+        (vbktip ? vbktip->toShortPrettyString() : "VBK:nullptr"),
+        (btctip ? btctip->toShortPrettyString() : "BTC:nullptr"),
+        pindexNew->nVersion,
+        log(pindexNew->nChainWork.getdouble()) / log(2.0), (unsigned long)pindexNew->nChainTx,
+        FormatISO8601DateTime(pindexNew->GetBlockTime()),
+        GuessVerificationProgress(chainParams.TxData(), pindexNew), ::ChainstateActive().CoinsTip().DynamicMemoryUsage() * (1.0 / (1 << 20)), ::ChainstateActive().CoinsTip().GetCacheSize(),
+        !warningMessages.empty() ? strprintf(" warning='%s'", warningMessages) : "");
 }
```
[method BlockManager::AcceptBlockHeader](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3659)
```cpp
     if (ppindex)
         *ppindex = pindex;

+    if (!VeriBlock::acceptBlock(*pindex, state)) {
+        return error("%s: ALT tree could not accept block ALT:%d:%s, reason: %s", __func__, pindex->nHeight, pindex->GetBlockHash().ToString(), state.ToString());
+    }
     return true;
```
[method CChainState::AcceptBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3796)
```cpp
         return error("%s: %s", __func__, FormatStateMessage(state));
     }

+    {
+        if (!VeriBlock::addAllBlockPayloads(block, state)) {
+            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-block-pop-payloads",
+                strprintf("Can not add POP payloads to block height: %d , hash: %s: %s",
+                    pindex->nHeight, block.GetHash().ToString(),
+                    FormatStateMessage(state)));
+        }
+    }
+
     // Header is valid/has work, merkle tree and segwit merkle tree are good...RELAY NOW
```
[method TestBlockValidity](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3919)
```cpp
+    // VeriBlock: Block that have been passed to TestBlockValidity may not exist in alt tree, because technically it was not created ("mined").
+    // in this case, add it and then remove
+    auto& tree = VeriBlock::GetPop().getAltBlockTree();
+    auto _hash = block_hash.asVector();
+    bool shouldRemove = false;
+    if (!tree.getBlockIndex(_hash)) {
+        shouldRemove = true;
+        auto containing = VeriBlock::blockToAltBlock(indexDummy);
+        altintegration::ValidationState _state;
+        bool ret = tree.acceptBlockHeader(containing, _state);
+        assert(ret && "alt tree can not accept alt block");
+
+        tree.acceptBlock(_hash, block.popData);
+    }
+
+    auto _f = altintegration::Finalizer([shouldRemove, _hash, &tree]() {
+        if (shouldRemove) {
+            tree.removeSubtree(_hash);
+        }
+    });
+
     if (!::ChainstateActive().ConnectBlock(block, state, &indexDummy, viewNew, chainparams, true))
         return false;
```
[method BlockManager::LoadBlockIndex](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L4177)
```cpp
-        if (pindex->IsValid(BLOCK_VALID_TREE) && (pindexBestHeader == nullptr || CBlockIndexWorkComparator()(pindexBestHeader, pindex)))
-            pindexBestHeader = pindex;
+        // do not set best chain here
     }

     // get best chain from ALT tree and update vBTC's best chain
     {
         AssertLockHeld(cs_main);

         // load blocks
         if(!VeriBlock::loadTrees(blocktree)) {
             return false;
         }
+
+        // ALT tree tip should be set - this is our last best tip
+        auto* tip = VeriBlock::GetPop().altTree->getBestChain().tip();
+        assert(tip && "we could not load tip of alt block");
+        uint256 hash(tip->getHash());
+
+        CBlockIndex* index = LookupBlockIndex(hash);
+        assert(index);
+        if (index->IsValid(BLOCK_VALID_TREE)) {
+            pindexBestHeader = index;
+        } else {
+            return false;
+        }
     }
```
[method CChainState::LoadGenesisBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L4751)
```cpp
         if (blockPos.IsNull())
             return error("%s: writing genesis block to disk failed", __func__);
-        CBlockIndex *pindex = m_blockman.AddToBlockIndex(block);
+        CBlockIndex* pindex = m_blockman.AddToBlockIndex(block);
+        BlockValidationState state;
+        if (!VeriBlock::acceptBlock(*pindex, state)) {
+            return false;
+        }
         ReceivedBlockTransactions(block, pindex, blockPos, chainparams.GetConsensus());
```

# 3. Show Pop related info when node starts.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp)

[method AppInitMain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp#L1209)
```cpp
+    {
+        auto& pop = VeriBlock::GetPop();
+        auto* tip = ChainActive().Tip();
+        altintegration::ValidationState state;
+        LOCK(cs_main);
+        bool ret = VeriBlock::setState(tip->GetBlockHash(), state);
+        auto* alttip = pop.getAltBlockTree().getBestChain().tip();
+        assert(ret && "bad state");
+        assert(tip->nHeight == alttip->getHeight());
+
+        LogPrintf("ALT tree best height = %d\n", pop.getAltBlockTree().getBestChain().tip()->getHeight());
+        LogPrintf("VBK tree best height = %d\n", pop.getVbkBlockTree().getBestChain().tip()->getHeight());
+        LogPrintf("BTC tree best height = %d\n", pop.getBtcBlockTree().getBestChain().tip()->getHeight());
+    }
+
     return true;
```

# 4. Update Pop logging support.

Pop logger: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/log.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/log.hpp). Copy this file to your project.

@note Pop logger inherits from altintegration::Logger. It should write to ALT log in the overriden virtual method `log()`.

Add additional flag for the logger.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.h)

[namespace BCLog](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.h#L35)
```cpp
         QT          = (1 << 19),
         LEVELDB     = (1 << 20),
+        POP         = (1 << 21),
         ALL         = ~(uint32_t)0,
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.cpp)

[field LogCategories](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/logging.cpp#L142)
```cpp
     {BCLog::LEVELDB, "leveldb"},
+    {BCLog::POP, "pop"},
     {BCLog::ALL, "1"},
```

Attach veriblock-pop-cpp library to the native logger.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp)
```cpp
+#include <vbk/log.hpp>
 #include <vbk/pop_service.hpp>
```
[method InitLogging](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp#L843)
```cpp
     LogInstance().m_log_threadnames = gArgs.GetBoolArg("-logthreadnames", DEFAULT_LOGTHREADNAMES);
+    LogInstance().EnableCategory(BCLog::POP);
+
+    std::string poplogverbosity = gArgs.GetArg("-poplogverbosity", "warn");
+    altintegration::SetLogger<VeriBlock::VBTCLogger>();
+    altintegration::GetLogger().level = altintegration::StringToLevel(poplogverbosity);

     fLogIPs = gArgs.GetBoolArg("-logips", DEFAULT_LOGIPS);
```

Add `poplogverbosity` to the application arguments. It allows to choose the verbosity level for the library logger.

[method SetupServerArgs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp#L347)
```cpp
         "-choosedatadir", "-lang=<lang>", "-min", "-resetguisettings", "-splash", "-uiplatform"};
+    // VBK
+    gArgs.AddArg("-poplogverbosity", "Verbosity for alt-cpp lib: debug/info/(warn)/error/off", ArgsManager::ALLOW_STRING, OptionsCategory::OPTIONS);
+    // VBK
```

# 5. Add unit tests to test the functionality we have added before.

VeriBlock precalculated data for the tests: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/util/consts.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/util/consts.hpp). Copy this file to your project.

Unit testing setup and helper methods: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/util/e2e_fixture.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/util/e2e_fixture.hpp). Copy this file to your project.

Update test setup to check if Pop AltTree was initialized properly.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp)

[method TestChain100Setup::TestChain100Setup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L162)
```cpp
     assert(ChainActive().Tip()->nHeight == 100);
     assert(BlockIndex().size() == 101);
+
+    auto& tree = *VeriBlock::GetPop().altTree;
+    assert(tree.getBestChain().tip()->getHeight() == ChainActive().Tip()->nHeight);
```

# 6. Add test case which tests the VeriBlock Pop behaviour: e2e_poptx_tests.cpp.

E2E Pop basic functionality test: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/e2e_poptx_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/e2e_poptx_tests.cpp). Copy this file to your project.

# 7. Update makefile to enable new unit test.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```cpp
+### VeriBlock section start
+# path is relative to src
+VBK_TESTS = \
+    vbk/test/unit/e2e_poptx_tests.cpp
+### VeriBlock section end
+
 # test_bitcoin binary #
 BITCOIN_TESTS =\
+  $(VBK_TESTS) \
   test/arith_uint256_tests.cpp \
   test/scriptnum10.h \
```

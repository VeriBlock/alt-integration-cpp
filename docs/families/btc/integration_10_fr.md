# Pop fork resolution {#integration_10_fr}

[TOC]

# Overview

Pop fork resolution uses Pop score to determine the winning chain. One block with highest Pop score can rollback a consideraby lengthy chain. There is no cumulative chain work like in Bitcoin. Therefore work comparator should be replaced with Pop aware comparator and candidate chain tips should be stored disregard their cumulative difficulty.

# 1. Add fork resoultion functions to the pop_service.cpp and pop_service.hpp.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp)
```cpp
+CBlockIndex* compareTipToBlock(CBlockIndex* candidate);
+int compareForks(const CBlockIndex& left, const CBlockIndex& right);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
+CBlockIndex* compareTipToBlock(CBlockIndex* candidate)
+{
+    AssertLockHeld(cs_main);
+    assert(candidate != nullptr && "block has no according header in block tree");
+
+    auto blockHash = candidate->GetBlockHash();
+    auto* tip = ChainActive().Tip();
+    if (!tip) {
+        // if tip is not set, candidate wins
+        return nullptr;
+    }
+
+    auto tipHash = tip->GetBlockHash();
+    if (tipHash == blockHash) {
+        // we compare tip with itself
+        return tip;
+    }
+
+    int result = 0;
+    if (Params().isPopActive(tip->nHeight)) {
+        result = compareForks(*tip, *candidate);
+    } else {
+        result = CBlockIndexWorkComparator()(tip, candidate) ? -1 : 1;
+    }
+
+    if (result < 0) {
+        // candidate has higher POP score
+        return candidate;
+    }
+
+    if (result == 0 && tip->nChainWork < candidate->nChainWork) {
+        // candidate is POP equal to current tip;
+        // candidate has higher chainwork
+        return candidate;
+    }
+
+    // otherwise, current chain wins
+    return tip;
+}
+
+int compareForks(const CBlockIndex& leftForkTip, const CBlockIndex& rightForkTip) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    auto& pop = GetPop();
+    AssertLockHeld(cs_main);
+    if (&leftForkTip == &rightForkTip) {
+        return 0;
+    }
+
+    auto left = blockToAltBlock(leftForkTip);
+    auto right = blockToAltBlock(rightForkTip);
+    auto state = altintegration::ValidationState();
+
+    if (!pop.getAltBlockTree().setState(left.hash, state)) {
          assert(false && "current tip is invalid");
+    }
+
+    return pop.getAltBlockTree().comparePopScore(left.hash, right.hash);
+}
```

# 2. Update validation.cpp to support Pop fork resolution.

Add additional failure codes.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chain.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chain.h)

[enum BlockStatus](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chain.h#L94)
```cpp
     BLOCK_FAILED_CHILD       =   64, //!< descends from failed block
-    BLOCK_FAILED_MASK        =   BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,

     BLOCK_OPT_WITNESS       =   128, //!< block data in blk*.data was received with a witness-enforcing client
+
+    // VeriBlock: block status
+    VERIBLOCK_BLOCK_FAILED_POP = 256,
+    VERIBLOCK_BLOCK_FAILED_CHILD = 512,
+
+    BLOCK_FAILED_MASK = BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD | VERIBLOCK_BLOCK_FAILED_POP | VERIBLOCK_BLOCK_FAILED_CHILD,
 };
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h)

[class CChainState](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h#L552)
```cpp
     void InvalidBlockFound(CBlockIndex *pindex, const BlockValidationState &state) EXCLUSIVE_LOCKS_REQUIRED(cs_main);
-    CBlockIndex* FindMostWorkChain() EXCLUSIVE_LOCKS_REQUIRED(cs_main);
+    CBlockIndex* FindBestChain() EXCLUSIVE_LOCKS_REQUIRED(cs_main);
```
[class CChainState](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h#L552)
```cpp
     //! Mark a block as not having block data
     void EraseBlockData(CBlockIndex* index) EXCLUSIVE_LOCKS_REQUIRED(cs_main);
+
+    bool TestBlockIndex(CBlockIndex* inhdex) EXCLUSIVE_LOCKS_REQUIRED(cs_main);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method CChainState::InvalidBlockFound](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L1390)
```cpp
     if (state.GetResult() != BlockValidationResult::BLOCK_MUTATED) {
         pindex->nStatus |= BLOCK_FAILED_VALID;
+
+        VeriBlock::GetPop()
+            .getAltBlockTree()
+            .invalidateSubtree(pindex->GetBlockHash().asVector(), altintegration::BLOCK_FAILED_BLOCK);
+
         m_blockman.m_failed_blocks.insert(pindex);
```
```cpp
-CBlockIndex* CChainState::FindMostWorkChain() {
-    do {
-        CBlockIndex *pindexNew = nullptr;
+CBlockIndex* CChainState::FindBestChain()
+{
+    AssertLockHeld(cs_main);
+    CBlockIndex* bestCandidate = m_chain.Tip();

-        // Find the best candidate header.
-        {
-            std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = setBlockIndexCandidates.rbegin();
-            if (it == setBlockIndexCandidates.rend())
-                return nullptr;
-            pindexNew = *it;
-        }
-
-        // Check whether all blocks on the path between the currently active chain and the candidate are valid.
-        // Just going until the active chain is an optimization, as we know all blocks in it are valid already.
-        CBlockIndex *pindexTest = pindexNew;
-        bool fInvalidAncestor = false;
-        while (pindexTest && !m_chain.Contains(pindexTest)) {
-            assert(pindexTest->HaveTxsDownloaded() || pindexTest->nHeight == 0);
-
-            // Pruned nodes may have entries in setBlockIndexCandidates for
-            // which block files have been deleted.  Remove those as candidates
-            // for the most work chain if we come across them; we can't switch
-            // to a chain unless we have all the non-active-chain parent blocks.
-            bool fFailedChain = pindexTest->nStatus & BLOCK_FAILED_MASK;
-            bool fMissingData = !(pindexTest->nStatus & BLOCK_HAVE_DATA);
-            if (fFailedChain || fMissingData) {
-                // Candidate chain is not usable (either invalid or missing data)
-                if (fFailedChain && (pindexBestInvalid == nullptr || pindexNew->nChainWork > pindexBestInvalid->nChainWork))
-                    pindexBestInvalid = pindexNew;
-                CBlockIndex *pindexFailed = pindexNew;
-                // Remove the entire chain from the set.
-                while (pindexTest != pindexFailed) {
-                    if (fFailedChain) {
-                        pindexFailed->nStatus |= BLOCK_FAILED_CHILD;
-                    } else if (fMissingData) {
-                        // If we're missing data, then add back to m_blocks_unlinked,
-                        // so that if the block arrives in the future we can try adding
-                        // to setBlockIndexCandidates again.
-                        m_blockman.m_blocks_unlinked.insert(
-                            std::make_pair(pindexFailed->pprev, pindexFailed));
-                    }
-                    setBlockIndexCandidates.erase(pindexFailed);
-                    pindexFailed = pindexFailed->pprev;
+    // return early
+    if (setBlockIndexCandidates.empty()) {
+        return nullptr;
+    }
+
+    auto temp_set = setBlockIndexCandidates;
+    for (auto* pindexNew : temp_set) {
+        if (pindexNew == bestCandidate || !TestBlockIndex(pindexNew)) {
+            continue;
+        }
+
+        if (bestCandidate == nullptr) {
+            bestCandidate = pindexNew;
+            continue;
+        }
+
+        int popComparisonResult = 0;
+
+        if (Params().isPopActive(bestCandidate->nHeight)) {
+            popComparisonResult = VeriBlock::compareForks(*bestCandidate, *pindexNew);
+        } else {
+            popComparisonResult = CBlockIndexWorkComparator()(bestCandidate, pindexNew) ? -1 : 1;
+        }
+
+        // even if next candidate is pop equal to current pindexNew, it is likely to have higher work
+        if (popComparisonResult <= 0) {
+            // candidate is either has POP or WORK better
+            bestCandidate = pindexNew;
+        }
+    }
+
+    return bestCandidate;
+}
+
+bool CChainState::TestBlockIndex(CBlockIndex* pindexTest)
+{
+    CBlockIndex* testWalkBlock = pindexTest;
+    bool fInvalidAncestor = false;
+    while (testWalkBlock && !m_chain.Contains(testWalkBlock)) {
+        assert(testWalkBlock->HaveTxsDownloaded() || testWalkBlock->nHeight == 0);
+
+        // Pruned nodes may have entries in setBlockIndexCandidates for
+        // which block files have been deleted.  Remove those as candidates
+        // for the most work chain if we come across them; we can't switch
+        // to a chain unless we have all the non-active-chain parent blocks.
+        bool fFailedChain = testWalkBlock->nStatus & BLOCK_FAILED_MASK;
+        bool fMissingData = !(testWalkBlock->nStatus & BLOCK_HAVE_DATA);
+        if (fFailedChain || fMissingData) {
+            // Candidate chain is not usable (either invalid or missing data)
+            if (fFailedChain && (pindexBestInvalid == nullptr || pindexTest->nChainWork > pindexBestInvalid->nChainWork))
+                pindexBestInvalid = pindexTest;
+            CBlockIndex* pindexFailed = pindexTest;
+            // Remove the entire chain from the set.
+            while (testWalkBlock != pindexFailed) {
+                if (fFailedChain) {
+                    pindexFailed->nStatus |= BLOCK_FAILED_CHILD;
+                } else if (fMissingData) {
+                    // If we're missing data, then add back to m_blocks_unlinked,
+                    // so that if the block arrives in the future we can try adding
+                    // to setBlockIndexCandidates again.
+                    m_blockman.m_blocks_unlinked.insert(
+                        std::make_pair(pindexFailed->pprev, pindexFailed));
+                }
+                setBlockIndexCandidates.erase(pindexFailed);
+                pindexFailed = pindexFailed->pprev;
+            }
+            setBlockIndexCandidates.erase(testWalkBlock);
+            fInvalidAncestor = true;
+            break;
+        }
+        testWalkBlock = testWalkBlock->pprev;
+    }
+    return !fInvalidAncestor;
+}
```
[method CChainState::PruneBlockIndexCandidates](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2709)
```cpp
     // Note that we can't delete the current block itself, as we may need to return to it later in case a
     // reorganization to a better block fails.
-    std::set<CBlockIndex*, CBlockIndexWorkComparator>::iterator it = setBlockIndexCandidates.begin();
-    while (it != setBlockIndexCandidates.end() && setBlockIndexCandidates.value_comp()(*it, m_chain.Tip())) {
-        setBlockIndexCandidates.erase(it++);
-    }
+    //std::set<CBlockIndex*, CBlockIndexWorkComparator>::iterator it = setBlockIndexCandidates.begin();
+    //while (it != setBlockIndexCandidates.end() && setBlockIndexCandidates.value_comp()(*it, m_chain.Tip())) {
+    //    setBlockIndexCandidates.erase(it++);
+    //}
+
+    // VeriBlock
+    auto temp_set = setBlockIndexCandidates;
+    for (const auto& el : temp_set) {
+        if (el->pprev != nullptr) {
+            setBlockIndexCandidates.erase(el->pprev);
+        }
```
[method CChainState::ActivateBestChain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2856)
```cpp
-    CBlockIndex *pindexMostWork = nullptr;
+    CBlockIndex* pindexBestChain = nullptr;
```
[method CChainState::ActivateBestChain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2856)
```cpp
                 ConnectTrace connectTrace(mempool); // Destructed before cs_main is unlocked

-                if (pindexMostWork == nullptr) {
-                    pindexMostWork = FindMostWorkChain();
+                if (pblock && pindexBestChain == nullptr) {
+                    auto* blockindex = LookupBlockIndex(pblock->GetHash());
+                    assert(blockindex);
+
+                    auto tmp_set = setBlockIndexCandidates;
+                    for (auto* candidate : tmp_set) {
+                        // if candidate has txs downloaded & currently arrived block is ancestor of `candidate`
+                        if (candidate->HaveTxsDownloaded() && TestBlockIndex(candidate) && candidate->GetAncestor(blockindex->nHeight) == blockindex) {
+                            // then do pop fr with candidate, instead of blockindex
+                            pindexBestChain = VeriBlock::compareTipToBlock(candidate);
+                        }
+                    }
+                }
+
+                if (pindexBestChain == nullptr) {
+                    pindexBestChain = FindBestChain();
                 }

                 // Whether we have anything to do at all.
-                if (pindexMostWork == nullptr || pindexMostWork == m_chain.Tip()) {
+                if (pindexBestChain == nullptr || pindexBestChain == m_chain.Tip()) {
                     break;
                 }

+                assert(pindexBestChain);
+                // if pindexBestHeader is a direct successor of pindexBestChain, pindexBestHeader is still best.
+                // otherwise pindexBestChain is new best pindexBestHeader
+                if (pindexBestHeader == nullptr || pindexBestHeader->GetAncestor(pindexBestChain->nHeight) != pindexBestChain) {
+                    pindexBestHeader = pindexBestChain;
+                }
+
                 bool fInvalidFound = false;
                 std::shared_ptr<const CBlock> nullBlockPtr;
-                if (!ActivateBestChainStep(state, chainparams, pindexMostWork, pblock && pblock->GetHash() == pindexMostWork->GetBlockHash() ? pblock : nullBlockPtr, fInvalidFound, connectTrace)) {
+                if (!ActivateBestChainStep(state, chainparams, pindexBestChain, pblock && pblock->GetHash() == pindexBestChain->GetBlockHash() ? pblock : nullBlockPtr, fInvalidFound, connectTrace)) {
                     // A system error occurred
                     return false;
                 }
```
[method CChainState::ActivateBestChain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2856)
```cpp
                 if (fInvalidFound) {
                     // Wipe cache, we may need another branch now.
-                    pindexMostWork = nullptr;
+                    pindexBestChain = nullptr;
                 }
                 pindexNewTip = m_chain.Tip();
```
[method CChainState::ActivateBestChain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2856)
```cpp
         if (ShutdownRequested())
             break;
-    } while (pindexNewTip != pindexMostWork);
+    } while (pindexNewTip != pindexBestChain);
+
     CheckBlockIndex(chainparams.GetConsensus());
```
[method CChainState::InvalidateBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3017)
```cpp
         while (it != m_blockman.m_block_index.end()) {
-            if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->HaveTxsDownloaded() && !setBlockIndexCandidates.value_comp()(it->second, m_chain.Tip())) {
+            if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->HaveTxsDownloaded()
+                // VeriBlock: setBlockIndexCandidates now stores all tips
+                /*&& !setBlockIndexCandidates.value_comp()(it->second, m_chain.Tip())*/
+            ) {
                 setBlockIndexCandidates.insert(it->second);
             }
```
[method CChainState::InvalidateBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3017)
```cpp
         InvalidChainFound(to_mark_failed);
     }

+    PruneBlockIndexCandidates();
+
     // Only notify about a new block tip if the active chain was modified.
```
[method CChainState::ResetBlockFailureFlags](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3162)
```cpp
     int nHeight = pindex->nHeight;
+    auto blockHash = pindex->GetBlockHash().asVector();
+    VeriBlock::GetPop().getAltBlockTree().revalidateSubtree(blockHash, altintegration::BLOCK_FAILED_BLOCK, false);
```
[method CChainState::ResetBlockFailureFlags](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3162)
```cpp
             setDirtyBlockIndex.insert(it->second);
-            if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->HaveTxsDownloaded() && setBlockIndexCandidates.value_comp()(m_chain.Tip(), it->second)) {
+            if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->HaveTxsDownloaded()
+                // VeriBlock: setBlockIndexCandidates now stores all tips
+                /*&& setBlockIndexCandidates.value_comp()(m_chain.Tip(), it->second)*/
+            ) {
                 setBlockIndexCandidates.insert(it->second);
             }
```
[method CChainState::ResetBlockFailureFlags](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3162)
```cpp
     }

+    PruneBlockIndexCandidates();
+
     // Remove the invalidity flag from all ancestors too.
     while (pindex != nullptr) {
         if (pindex->nStatus & BLOCK_FAILED_MASK) {
```
[method BlockManager::AddToBlockIndex](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3209)
```cpp
     pindexNew->RaiseValidity(BLOCK_VALID_TREE);
-    if (pindexBestHeader == nullptr || pindexBestHeader->nChainWork < pindexNew->nChainWork)
+    // VeriBlock: if pindexNew is a successor of pindexBestHeader, and pindexNew has higher chainwork, then update pindexBestHeader
+    if (pindexBestHeader == nullptr || ((pindexBestHeader->nChainWork < pindexNew->nChainWork) &&
+                                           (pindexNew->GetAncestor(pindexBestHeader->nHeight) == pindexBestHeader)))
         pindexBestHeader = pindexNew;
```
[method CChainState::ReceivedBlockTransactions](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3247)
```cpp
-            if (m_chain.Tip() == nullptr || !setBlockIndexCandidates.value_comp()(pindex, m_chain.Tip())) {
+            /*if (m_chain.Tip() == nullptr || !setBlockIndexCandidates.value_comp()(pindex, m_chain.Tip())) {
                 setBlockIndexCandidates.insert(pindex);
-            }
+            }*/
+            setBlockIndexCandidates.insert(pindex);
             std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> range = m_blockman.m_blocks_unlinked.equal_range(pindex);
```
[method CChainState::ReceivedBlockTransactions](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3247)
```cpp
     }
+
+    PruneBlockIndexCandidates();
 }
```
[method CChainState::AcceptBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3796)
```cpp
     if (fAlreadyHave) return true;
-    if (!fRequested) {  // If we didn't ask for it:
-        if (pindex->nTx != 0) return true;    // This is a previously-processed block that was pruned
-        if (!fHasMoreOrSameWork) return true; // Don't process less-work chains
-        if (fTooFarAhead) return true;        // Block height is too high
+    if (!fRequested) {                     // If we didn't ask for it:
+        if (pindex->nTx != 0) return true; // This is a previously-processed block that was pruned
+        if (fTooFarAhead) return true;     // Block height is too high
```
[method CChainState::CheckBlockIndex](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L4905)
```cpp
-        if (!CBlockIndexWorkComparator()(pindex, m_chain.Tip()) && pindexFirstNeverProcessed == nullptr) {
-            if (pindexFirstInvalid == nullptr) {
-                // If this block sorts at least as good as the current tip and
-                // is valid and we have all data for its parents, it must be in
-                // setBlockIndexCandidates.  m_chain.Tip() must also be there
-                // even if some data has been pruned.
-                if (pindexFirstMissing == nullptr || pindex == m_chain.Tip()) {
-                    assert(setBlockIndexCandidates.count(pindex));
-                }
-                // If some parent is missing, then it could be that this block was in
-                // setBlockIndexCandidates but had to be removed because of the missing data.
-                // In this case it must be in m_blocks_unlinked -- see test below.
-            }
-        } else { // If this block sorts worse than the current tip or some ancestor's block has never been seen, it cannot be in setBlockIndexCandidates.
-            assert(setBlockIndexCandidates.count(pindex->pprev) == 0);
-        }
+        //VeriBlock
+        //if (!CBlockIndexWorkComparator()(pindex, m_chain.Tip()) && pindexFirstNeverProcessed == nullptr) {
+        //    if (pindexFirstInvalid == nullptr) {
+        //        // If this block sorts at least as good as the current tip and
+        //        // is valid and we have all data for its parents, it must be in
+        //        // setBlockIndexCandidates.  m_chain.Tip() must also be there
+        //        // even if some data has been pruned.
+        //        if (pindexFirstMissing == nullptr || pindex == m_chain.Tip()) {
+        //            assert(setBlockIndexCandidates.count(pindex));
+        //        }
+        //        // If some parent is missing, then it could be that this block was in
+        //        // setBlockIndexCandidates but had to be removed because of the missing data.
+        //        // In this case it must be in m_blocks_unlinked -- see test below.
+        //    }
+        //} else { // If this block sorts worse than the current tip or some ancestor's block has never been seen, it cannot be in setBlockIndexCandidates.
+        //    assert(setBlockIndexCandidates.count(pindex->pprev) == 0);
+        //}
```

# 3. Add Pop fork resolution unit test.

Pop fork resolution test: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/forkresolution_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/forkresolution_tests.cpp). Copy this file to your project.

# 4. Update makefile to run tests.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```cpp
 ### VeriBlock section start
 # path is relative to src
 VBK_TESTS = \
   vbk/test/unit/e2e_poptx_tests.cpp \
   vbk/test/unit/block_validation_tests.cpp \
   vbk/test/unit/vbk_merkle_tests.cpp \
-  vbk/test/unit/pop_reward_tests.cpp
+  vbk/test/unit/pop_reward_tests.cpp \
+  vbk/test/unit/forkresolution_tests.cpp
```

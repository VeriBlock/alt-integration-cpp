# Pop rewards {#integration_9_rewards}

[TOC]

# Overview

Modify rewarding algorithm. Basic PoW rewards are extended with Pop rewards for the Pop miners.

# 1. Modify ALT params (CChainParams). Add two new VeriBlock parameters for the Pop rewards.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h)

[class CChainParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h#L50)
```cpp
     bool isPopActive(uint64_t height) const {
         return height >= consensus.VeriBlockPopSecurityHeight;
     }
+
+    uint32_t PopRewardPercentage() const {return mPopRewardPercentage;}
+    int32_t PopRewardCoefficient() const {return mPopRewardCoefficient;}
```
[class CChainParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h#L50)
```cpp
     ChainTxData chainTxData;
+
+    // VeriBlock:
+    // cut this % from coinbase subsidy
+    uint32_t mPopRewardPercentage = 40; // %
+    // every pop reward will be multiplied by this coefficient
+    int32_t mPopRewardCoefficient = 20;
```

# 2. Implement Pop rewards related methods in the pop_service.hpp and pop_service.cpp source files.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp)
```cpp
+PoPRewards getPopRewards(const CBlockIndex& pindexPrev, const CChainParams& params);
+void addPopPayoutsIntoCoinbaseTx(CMutableTransaction& coinbaseTx, const CBlockIndex& pindexPrev, const CChainParams& params);
+bool checkCoinbaseTxWithPopRewards(const CTransaction& tx, const CAmount& nFees, const CBlockIndex& pindexPrev, const CChainParams& params, CValidationState& state);
+CAmount getCoinbaseSubsidy(const CAmount& subsidy, int32_t height, const CChainParams& params);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
+// PoP rewards are calculated for the current tip but are paid in the next block
+PoPRewards getPopRewards(const CBlockIndex& pindexPrev, const CChainParams& params) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    auto& pop = GetPop();
+
+    if (!params.isPopActive(pindexPrev.nHeight)) {
+        return {};
+    }
+
+    auto& cfg = pop.getConfig();
+    if (pindexPrev.nHeight < (int)cfg.alt->getEndorsementSettlementInterval()) {
+        return {};
+    }
+    if (pindexPrev.nHeight < (int)cfg.alt->getPayoutParams().getPopPayoutDelay()) {
+        return {};
+    }
+
+    altintegration::ValidationState state;
+    auto prevHash = pindexPrev.GetBlockHash().asVector();
+    bool ret = pop.getAltBlockTree().setState(prevHash, state);
+    (void)ret;
+    assert(ret);
+
+    auto rewards = pop.getPopPayout(prevHash);
+    int halvings = (pindexPrev.nHeight + 1) / params.GetConsensus().nSubsidyHalvingInterval;
+    PoPRewards result{};
+    // erase rewards, that pay 0 satoshis, then halve rewards
+    for (const auto& r : rewards) {
+        auto rewardValue = r.second;
+        rewardValue >>= halvings;
+        if ((rewardValue != 0) && (halvings < 64)) {
+            CScript key = CScript(r.first.begin(), r.first.end());
+            result[key] = params.PopRewardCoefficient() * rewardValue;
+        }
+    }
+
+    return result;
+}
+
+void addPopPayoutsIntoCoinbaseTx(CMutableTransaction& coinbaseTx, const CBlockIndex& pindexPrev, const CChainParams& params) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    PoPRewards rewards = getPopRewards(pindexPrev, params);
+    assert(coinbaseTx.vout.size() == 1 && "at this place we should have only PoW payout here");
+    for (const auto& itr : rewards) {
+        CTxOut out;
+        out.scriptPubKey = itr.first;
+        out.nValue = itr.second;
+        coinbaseTx.vout.push_back(out);
+    }
+}
+
+bool checkCoinbaseTxWithPopRewards(const CTransaction& tx, const CAmount& nFees, const CBlockIndex& pindexPrev, const CChainParams& params, BlockValidationState& state) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    PoPRewards expectedRewards = getPopRewards(pindexPrev, params);
+    CAmount nTotalPopReward = 0;
+
+    if (tx.vout.size() < expectedRewards.size()) {
+        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-pop-vouts-size",
+            strprintf("checkCoinbaseTxWithPopRewards(): coinbase has incorrect size of pop vouts (actual vouts size=%d vs expected vouts=%d)", tx.vout.size(), expectedRewards.size()));
+    }
+
+    std::map<CScript, CAmount> cbpayouts;
+    // skip first reward, as it is always PoW payout
+    for (auto out = tx.vout.begin() + 1, end = tx.vout.end(); out != end; ++out) {
+        // pop payouts can not be null
+        if (out->IsNull()) {
+            continue;
+        }
+        cbpayouts[out->scriptPubKey] += out->nValue;
+    }
+
+    // skip first (regular pow) payout, and last 2 0-value payouts
+    for (const auto& payout : expectedRewards) {
+        auto& script = payout.first;
+        auto& expectedAmount = payout.second;
+
+        auto p = cbpayouts.find(script);
+        // coinbase pays correct reward?
+        if (p == cbpayouts.end()) {
+            // we expected payout for that address
+            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-pop-missing-payout",
+                strprintf("[tx: %s] missing payout for scriptPubKey: '%s' with amount: '%d'",
+                    tx.GetHash().ToString(),
+                    HexStr(script),
+                    expectedAmount));
+        }
+
+        // payout found
+        auto& actualAmount = p->second;
+        // does it have correct amount?
+        if (actualAmount != expectedAmount) {
+            return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-pop-wrong-payout",
+                strprintf("[tx: %s] wrong payout for scriptPubKey: '%s'. Expected %d, got %d.",
+                    tx.GetHash().ToString(),
+                    HexStr(script),
+                    expectedAmount, actualAmount));
+        }
+
+        nTotalPopReward += expectedAmount;
+    }
+
+    CAmount PoWBlockReward =
+        GetBlockSubsidy(pindexPrev.nHeight, params);
+
+    if (tx.GetValueOut() > nTotalPopReward + PoWBlockReward + nFees) {
+        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS,
+            "bad-cb-pop-amount",
+            strprintf("ConnectBlock(): coinbase pays too much (actual=%d vs limit=%d)", tx.GetValueOut(), PoWBlockReward + nTotalPopReward));
+    }
+
+    return true;
+}
+
+CAmount getCoinbaseSubsidy(const CAmount& subsidy, int32_t height, const CChainParams& params)
+{
+    if (!params.isPopActive(height)) {
+        return subsidy;
+    }
+
+    int64_t powRewardPercentage = 100 - params.PopRewardPercentage();
+    CAmount newSubsidy = powRewardPercentage * subsidy;
+    return newSubsidy / 100;
+}
```

@note Rewarding algorithm should fall back to the original ALT rewarding if Pop security is not activated.
@note It is recommended to apply native halving rules to Pop rewards.

# 3. Modify GetBlockSubsidy() to accept CChainParams instead of consensus params.

We have to check the Pop activation height when calculating Pop rewards. Therefore some methods should be modified to accept chain parameters instead of consensus parameters.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.h)
```cpp
 bool ActivateBestChain(BlockValidationState& state, const CChainParams& chainparams, std::shared_ptr<const CBlock> pblock = std::shared_ptr<const CBlock>());
-CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams);
+CAmount GetBlockSubsidy(int nHeight, const CChainParams& params);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)
```cpp
-CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams)
+CAmount GetBlockSubsidy(int nHeight, const CChainParams& params)
 {
-    int halvings = nHeight / consensusParams.nSubsidyHalvingInterval;
+    int halvings = nHeight / params.GetConsensus().nSubsidyHalvingInterval;
     // Force block reward to zero when right shift is undefined.
     if (halvings >= 64)
         return 0;
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp)

[method BlockAssembler::CreateNewBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp#L96)
```cpp
     coinbaseTx.vout[0].scriptPubKey = scriptPubKeyIn;
-    coinbaseTx.vout[0].nValue = nFees + GetBlockSubsidy(nHeight, chainparams.GetConsensus());
+    coinbaseTx.vout[0].nValue = nFees + GetBlockSubsidy(nHeight, chainparams);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp)

[method getblockstats](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp#L1722)
```cpp
     ret_all.pushKV("outs", outputs);
-    ret_all.pushKV("subsidy", GetBlockSubsidy(pindex->nHeight, Params().GetConsensus()));
+    ret_all.pushKV("subsidy", GetBlockSubsidy(pindex->nHeight, Params()));
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bench/duplicate_inputs.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bench/duplicate_inputs.cpp)

[method DuplicateInputs](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bench/duplicate_inputs.cpp#L17)
```cpp
     coinbaseTx.vout[0].scriptPubKey = SCRIPT_PUB;
-    coinbaseTx.vout[0].nValue = GetBlockSubsidy(nHeight, chainparams.GetConsensus());
+    coinbaseTx.vout[0].nValue = GetBlockSubsidy(nHeight, chainparams);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp)

[method TestBlockSubsidyHalvings](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp#L27)
```cpp
     for (int nHalvings = 0; nHalvings < maxHalvings; nHalvings++) {
-        int nHeight = nHalvings * consensusParams.nSubsidyHalvingInterval;
-        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
+        int nHeight = nHalvings * params.GetConsensus().nSubsidyHalvingInterval;
+        CAmount nSubsidy = GetBlockSubsidy(nHeight, params);
         BOOST_CHECK(nSubsidy <= nInitialSubsidy);
         BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
         nPreviousSubsidy = nSubsidy;
     }
-    BOOST_CHECK_EQUAL(GetBlockSubsidy(maxHalvings * consensusParams.nSubsidyHalvingInterval, consensusParams), 0);
+    BOOST_CHECK_EQUAL(GetBlockSubsidy(maxHalvings * params.GetConsensus().nSubsidyHalvingInterval, params), 0);
```
[method BOOST_AUTO_TEST_CASE(subsidy_limit_test)](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp#L58)
```cpp
     CAmount nSum = 0;
     for (int nHeight = 0; nHeight < 14000000; nHeight += 1000) {
-        CAmount nSubsidy = GetBlockSubsidy(nHeight, chainParams->GetConsensus());
+        CAmount nSubsidy = GetBlockSubsidy(nHeight, *chainParams);
```

# 4. Modify mining process in the CreateNewBlock function. Insert VeriBlock PoPRewards into the coinbase transaction, add some validation rules to the validation.cpp.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method GetBlockSubsidy](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L1207)
```cpp
     CAmount nSubsidy = 50 * COIN;
     // Subsidy is cut in half every 210,000 blocks which will occur approximately every 4 years.
+    nSubsidy = VeriBlock::getCoinbaseSubsidy(nSubsidy, nHeight, params);
+
     nSubsidy >>= halvings;
```

[method CChainState::ConnectBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L1879)
```cpp
-    int64_t nTime3 = GetTimeMicros(); nTimeConnect += nTime3 - nTime2;
-    LogPrint(BCLog::BENCH, "      - Connect %u transactions: %.2fms (%.3fms/tx, %.3fms/txin) [%.2fs (%.2fms/blk)]\n", (unsigned)block.vtx.size(), MILLI * (nTime3 - nTime2), MILLI * (nTime3 - nTime2) / block.vtx.size(), nInputs <= 1 ? 0 : MILLI * (nTime3 - nTime2) / (nInputs-1), nTimeConnect * MICRO, nTimeConnect * MILLI / nBlocksTotal);

-    CAmount blockReward = nFees + GetBlockSubsidy(pindex->nHeight, chainparams.GetConsensus());
-    if (block.vtx[0]->GetValueOut() > blockReward) {
-        LogPrintf("ERROR: ConnectBlock(): coinbase pays too much (actual=%d vs limit=%d)\n", block.vtx[0]->GetValueOut(), blockReward);
-        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-amount");
+    int64_t nTime3 = GetTimeMicros();
+    nTimeConnect += nTime3 - nTime2;
+    LogPrint(BCLog::BENCH, "      - Connect %u transactions: %.2fms (%.3fms/tx, %.3fms/txin) [%.2fs (%.2fms/blk)]\n", (unsigned)block.vtx.size(), MILLI * (nTime3 - nTime2), MILLI * (nTime3 - nTime2) / block.vtx.size(), nInputs <= 1 ? 0 : MILLI * (nTime3 - nTime2) / (nInputs - 1), nTimeConnect * MICRO, nTimeConnect * MILLI / nBlocksTotal);
+
+    assert(pindex->pprev && "previous block ptr is nullptr");
+    if (!VeriBlock::checkCoinbaseTxWithPopRewards(*block.vtx[0], nFees, *pindex->pprev, chainparams, state)) {
+        return false;
+    }
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp)

[method BlockAssembler::CreateNewBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp#L96)
```cpp
     coinbaseTx.vin[0].scriptSig = CScript() << nHeight << OP_0;
+
+    VeriBlock::addPopPayoutsIntoCoinbaseTx(coinbaseTx, *pindexPrev, chainparams);
+
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp)

[method BOOST_AUTO_TEST_CASE(subsidy_limit_test)](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/validation_tests.cpp#L58)
```cpp
     CAmount nSum = 0;
-    for (int nHeight = 0; nHeight < 14000000; nHeight += 1000) {
+    // skip first 1000 blocks to make sure POP security is ON
+    for (int nHeight = 1000; nHeight < 14000000; nHeight += 1000) {
```

# 5. Add tests for the Pop rewards.

Pop rewards test: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/pop_reward_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/pop_reward_tests.cpp). Copy this file to your project.

@note Test expects Pop reward in the second coinbase transaction. Update the test according to the ALT rewarding scheme.

# 6. Update makefile to run tests.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```cpp
 ### VeriBlock section start
 # path is relative to src
 VBK_TESTS = \
   vbk/test/unit/e2e_poptx_tests.cpp \
   vbk/test/unit/block_validation_tests.cpp \
-  vbk/test/unit/vbk_merkle_tests.cpp
+  vbk/test/unit/vbk_merkle_tests.cpp \
+  vbk/test/unit/pop_reward_tests.cpp
```

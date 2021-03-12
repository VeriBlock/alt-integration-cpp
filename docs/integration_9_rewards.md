# Pop rewards. {#integration_9_rewards}

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
@note It is advised to apply native halving rules to Pop rewards.
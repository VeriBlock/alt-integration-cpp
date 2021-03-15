# Choosing fork point {#integration_3_forkpoint}

[TOC]

# Overview

The only way to enable VeriBlock security for already running blockchains is to make a fork. For this reason we will provide a height of the fork point.

# 1. Add block height from which PopSecurity is enabled into the consensus parameters.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h)

[struct Params](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h#L45)
```cpp
     uint256 nMinimumChainWork;
     uint256 defaultAssumeValid;
+
+    // VeriBlock
+    uint64_t VeriBlockPopSecurityHeight;
```

# 2. Define VeriBlockPopSecurityHeight variable.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp)

[class CMainParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp#L71)
```cpp
+
+        // VeriBlock
+        // TODO: should determine the correct height
+        consensus.VeriBlockPopSecurityHeight = -1;
+
+        // The best chain should have at least this much work.
         consensus.nMinimumChainWork = uint256S("0x00");
```
[class CTestNetParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp#L154)
```cpp
+
+        // VeriBlock
+        // TODO: should determine the correct height
+        consensus.VeriBlockPopSecurityHeight = -1;

         // The best chain should have at least this much work.
         consensus.nMinimumChainWork = uint256S("0x00");
```
[class CRegTestParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp#L242)
```cpp
+
+        // VeriBlock
+        // TODO: should determine the correct height
+        consensus.VeriBlockPopSecurityHeight = -1;

         // The best chain should have at least this much work.
         consensus.nMinimumChainWork = uint256S("0x00");
```

# 3. Create a function in the chainparams which detects if the Pop security is enabled.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h)

[class CChainParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.h#L50)
```cpp
     const CCheckpointData& Checkpoints() const { return checkpointData; }
     const ChainTxData& TxData() const { return chainTxData; }
+
+    // VeriBlock
+    bool isPopActive(uint64_t height) const {
+        return height >= consensus.VeriBlockPopSecurityHeight;
+    }
+
```

# 4. Update the validation of blocks. If PoPSecurity is disabled POP_BLOCK_VERSION_BIT should not be set.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method ContextualCheckBlockHeader](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L3524)
```cpp
+    // VeriBlock validation
+    if ((block.nVersion & VeriBlock::POP_BLOCK_VERSION_BIT) && !params.isPopActive(nHeight)) {
+    {
+        return state.Invalid(BlockValidationResult::BLOCK_INVALID_HEADER, strprintf("bad-pop-version(0x%08x)", block.nVersion),
+         strprintf("block contains PopData before PopSecurity has been enabled"));
+    }
+
     return true;
```

# 5. Change height of the Pop security fork point in the regtest. It allows to properly run Pop tests.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp)

[method CRegTestParams::CRegTestParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/chainparams.cpp#L242)
```cpp
+        consensus.VeriBlockPopSecurityHeight = 200;

         // The best chain should have at least this much work.
         consensus.nMinimumChainWork = uint256S("0x00");
```
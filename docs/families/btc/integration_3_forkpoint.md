# Choosing fork point {#integration_3_forkpoint}

[TOC]

# Overview

The only way to enable VeriBlock security for already running blockchains is to make a hard fork. For this reason we will provide a height of the fork point.

# 1. Add block height for Pop Security activation.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h)

[struct Params](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/consensus/params.h#L45)
```cpp
     uint256 nMinimumChainWork;
     uint256 defaultAssumeValid;
+
+    // VeriBlock
+    uint64_t VeriBlockPopSecurityHeight;
```

@warning The value must be selected carefully. If current height is `X`, never set height less or equal to `X`.

@note Set `VeriBlockPopSecurityHeight` equal to 200 in regtest, so that existing tests can be re-used.

# 2. Create a function in the chainparams which detects if the Pop security is enabled.

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

# 3. Update the block version validation.

`POP_BLOCK_VERSION_BIT` should not be set before PoP Activation Height:

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

Technically, this check is stateless, so can be moved to `CheckBlockHeader` if wanted.

[Next Section](./integration_4_config.md)

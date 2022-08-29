# Adding configuration {#integration_4_config}

[TOC]

# Overview

Before using structures of the VeriBlock library, we should define some VeriBlock specific parameters. We have to add new Config class which inherits from the altintegration::AltChainParams.
But first we will add functions that contain API for the library interactions.

# 1. Create two new source files: pop_common.hpp, pop_common.cpp.

POP configuration header: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_common.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_common.hpp). Copy this file to your project.

POP configuration source: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_common.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_common.cpp). Copy this file to your project.

# 2. Add bootstraps blocks.

@note veriblock-pop-cpp library maintains all blocks of Bitcoin and VeriBlock starting at certain set of blocks - "bootstrap blocks".

Bootstrap blocks header: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.h). Copy this file to your project.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.cpp)
```cpp
// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vbk/bootstraps.h>

namespace VeriBlock {

const int testnetVBKstartHeight=860529;
const int testnetBTCstartHeight=1832624;

const std::vector<std::string> testnetBTCblocks = {};

const std::vector<std::string> testnetVBKblocks = {};

} // namespace VeriBlock
```

@note Bootstrap blocks, both BTC and VBK, should contain as recent data as possible. APM will have to maintain connectivity of blocks. Therefore it will have to mine all missing blocks starting from bootstrap.

@note We will fill bootstraps.cpp at the end of the integration document.

# 3. Create AltChainParamsBTC class with VeriBlock configuration of the ALT blockchain.

POP configuration loader header: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.hpp). Copy this file to your project.

@note Make sure to change `altchainId` to uniquely identify ALT blockchain data.

[struct AltChainParamsBTC](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.hpp#L33)
```cpp
    int64_t getIdentifier() const noexcept override
    {
        return <altchainId>;
    }
```

@note `altchainId` is 8-byte ID, sent with every endorsement of ALT block in VBK. Used by VeriBlock to group potentially relevant endorsements. It is not critical if `altchainId` is reused by multiple Altchains, but this is not preferable.

POP configuration loader source: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.cpp). Copy this file to your project.

@note Modify [checkBlockHeader](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.cpp#L20) to validate ALT block proof-of-work.

Util file with some useful functions for the VeriBlock integration: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/util.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/util.hpp). Copy this file to your project.

# 4. Update the initialization of the bitcoind, bitcoin-wallet, etc to setup VeriBlock config.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoind.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoind.cpp)
```cpp
 #include <functional>
+#include <vbk/params.hpp>
```
[method AppInit](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoind.cpp#L44)
```cpp
         try {
+            if(gArgs.GetChainName() == CBaseChainParams::MAIN) {
+                throw std::runtime_error("Mainnet is disabled. Use testnet.");
+            }
             SelectParams(gArgs.GetChainName());
+            VeriBlock::selectPopConfig(gArgs, gArgs.GetChainName(), gArgs.GetChainName());
         } catch (const std::exception& e) {
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-tx.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-tx.cpp)
```cpp
 #include <stdio.h>

 #include <boost/algorithm/string.hpp>
+#include <vbk/params.hpp>
```
[method AppInitRawTx](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-tx.cpp#L81)
```cpp
     try {
         SelectParams(gArgs.GetChainName());
+        VeriBlock::selectPopConfig(gArgs, gArgs.GetChainName(), gArgs.GetChainName());
     } catch (const std::exception& e) {
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/interfaces/node.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/interfaces/node.cpp)
```cpp
+#include <vbk/params.hpp>
 #include <univalue.h>
```
[class NodeImpl](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/interfaces/node.cpp#L56)
```cpp
     bool softSetBoolArg(const std::string& arg, bool value) override { return gArgs.SoftSetBoolArg(arg, value); }
-    void selectParams(const std::string& network) override { SelectParams(network); }
+    void selectParams(const std::string& network) override { SelectParams(network); VeriBlock::selectPopConfig(gArgs);}
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-wallet.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-wallet.cpp)
```cpp
 #include <wallet/wallettool.h>
+#include <vbk/params.hpp>
```
[method WalletAppInit](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/bitcoin-wallet.cpp#L37)
```cpp
     // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
     SelectParams(gArgs.GetChainName());
+    VeriBlock::selectPopConfig(gArgs, gArgs.GetChainName(), gArgs.GetChainName());
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp)
```cpp
 #include <validationinterface.h>

+#include <vbk/params.hpp>
```
[method BasicTestingSetup::BasicTestingSetup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L69)
```cpp
     ClearDatadirCache();
     SelectParams(chainName);
+    VeriBlock::selectPopConfig("regtest", "regtest", true);
```

# 5. Update test chain setup to allow adding block to specific previous block.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.h)
```cpp
 #include <txmempool.h>

+#include <vbk/pop_service.hpp>
```
[struct TestChain100Setup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.h#L107)
```cpp
     CBlock CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns,
-                                 const CScript& scriptPubKey);
+                                 const CScript& scriptPubKey, bool* isBlockValid = nullptr);
+
+    CBlock CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, uint256 prevBlock,
+                                 const CScript& scriptPubKey, bool* isBlockValid = nullptr);
+
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp)
```cpp
-// Create a new block with just given transactions, coinbase paying to
-// scriptPubKey, and try to add it to the current chain.
-CBlock TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
-{
+CBlock TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, uint256 prevBlock,
+                             const CScript& scriptPubKey, bool* isBlockValid) {
+
+    CBlockIndex* pPrev = nullptr;
+    {
+        LOCK(cs_main);
+        pPrev = LookupBlockIndex(prevBlock);
+        assert(pPrev && "CreateAndProcessBlock called with unknown prev block");
+    }
+
     const CChainParams& chainparams = Params();
```
[method TestChain100Setup::CreateAndProcessBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L188)
```cpp
         unsigned int extraNonce = 0;
-        IncrementExtraNonce(&block, ::ChainActive().Tip(), extraNonce);
+        IncrementExtraNonce(&block, pPrev, extraNonce);
```
[method TestChain100Setup::CreateAndProcessBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L188)
```cpp
     std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
-    ProcessNewBlock(chainparams, shared_pblock, true, nullptr);
+
+    bool isValid = ProcessNewBlock(chainparams, shared_pblock, true, nullptr);
+    if(isBlockValid != nullptr) {
+        *isBlockValid = isValid;
+    }
```
```cpp
+// Create a new block with just given transactions, coinbase paying to
+// scriptPubKey, and try to add it to the current chain.
+CBlock TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey, bool* isBlockValid)
+{
+    return CreateAndProcessBlock(txns, ChainActive().Tip()->GetBlockHash(), scriptPubKey, isBlockValid);
+}
```

# 6. Update makefiles. Add new source files.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```cpp
 libbitcoin_common_a_SOURCES = \
+  vbk/pop_common.hpp \
+  vbk/pop_common.cpp \
+  vbk/params.cpp \
+  vbk/bootstraps.cpp \
```

[Next Section](./integration_5_persistence.md)

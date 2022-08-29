# New RPC calls {#integration_12_rpc}

[TOC]

# Overview

We have to provide Pop aware RPC calls to let users interact with node. We'll add new functions to the RPC.

Here is the list of the provided Pop RPC calls:
- `getpopparams`
- `submitpopatv`
- `submitpopvtb`
- `submitpopvbk`
- `getpopdatabyheight`
- `getpopdatabyhash`
- `getvbkblock`
- `getbtcblock`
- `getvbkbestblockhash`
- `getbtcbestblockhash`
- `getvbkblockhash`
- `getbtcblockhash`
- `getrawatv`
- `getrawvtb`
- `getrawvbkblock`
- `getrawpopmempool`

Besides new RPC calls some of the original calls will be extended to provide additional Pop information.

Here is the list of extended RPC calls:
- `getblock`
- `getblockchaininfo`
- `getmininginfo`
- `getblocktemplate`

@note Description of the RPC calls can be found in the source code. For example [here](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/rpc_register.cpp#L112) is the description of `getpopdatabyheight` RPC call.

# 1. Add RPC service files: rpc_register.hpp, rpc_register.cpp.

Pop RPC header: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/rpc_register.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/rpc_register.hpp). Copy this file to your project.

Pop RPC source: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/rpc_register.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/rpc_register.cpp). Copy this file to your project.

# 2. Add JSON adaptor for the library that allows converting from altintegration::MempoolResult to UniValue object.

Univalue conversions: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/adaptors/univalue_json.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/adaptors/univalue_json.hpp). Copy this file to your project.

# 3. Update serialize.h to use new JSON convertor.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/serialize.h)
```cpp
 #include <span.h>

+#include <vbk/adaptors/univalue_json.hpp>
+
 #include <veriblock/pop.hpp>
```

# 4. Add all new RPC functions to the RPC server.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/register.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/register.h)
```cpp
 #define BITCOIN_RPC_REGISTER_H

+#include "vbk/rpc_register.hpp"
```
[method RegisterAllCoreRPCCommands](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/register.h#L27)
```cpp
     RegisterMiningRPCCommands(t);
     RegisterRawTransactionRPCCommands(t);
+    VeriBlock::RegisterPOPMiningRPCCommands(t);
 }
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/client.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/client.cpp)

[field vRPCConvertParams](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/client.cpp#L29)
```cpp
 static const CRPCConvertParam vRPCConvertParams[] =
 {
+    // VeriBlock
+    { "getbtcblockhash", 0, "height"},
+    { "getvbkblockhash", 0, "height"},
+    { "getrawatv", 1, "verbose"},
+    { "getrawvtb", 1, "verbose"},
+    { "getrawvbkblock", 1, "verbose"},
+    { "getpopdatabyheight", 0, "block_height"},
+    // end VeriBlock
     { "setmocktime", 0, "timestamp" },
```

# 5. Extend original RPC calls with Pop data.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp)
```cpp
 #include <mutex>

+#include <vbk/adaptors/univalue_json.hpp>
+#include <vbk/pop_common.hpp>
```
[method getblock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp#L834)
```cpp
-    return blockToJSON(block, tip, pblockindex, verbosity >= 2);
+    UniValue json = blockToJSON(block, tip, pblockindex, verbosity >= 2);
+
+    {
+        auto& pop = VeriBlock::GetPop();
+        LOCK(cs_main);
+        auto index = pop.getAltBlockTree().getBlockIndex(block.GetHash().asVector());
+        VBK_ASSERT(index);
+        UniValue obj(UniValue::VOBJ);
+
+        obj.pushKV("state", altintegration::ToJSON<UniValue>(*index));
+        obj.pushKV("data", altintegration::ToJSON<UniValue>(block.popData, verbosity >= 2));
+        json.pushKV("pop", obj);
+    }
+
+    return json;
```
[method getblockchaininfo](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/blockchain.cpp#L1217)
```cpp
     BuriedForkDescPushBack(softforks, "csv", consensusParams.CSVHeight);
     BuriedForkDescPushBack(softforks, "segwit", consensusParams.SegwitHeight);
+    //VeriBlock
+    BuriedForkDescPushBack(softforks, "pop_security", consensusParams.VeriBlockPopSecurityHeight);
     BIP9SoftForkDescPushBack(softforks, "testdummy", consensusParams, Consensus::DEPLOYMENT_TESTDUMMY);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/mining.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/mining.cpp)

[method getblocktemplate](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/mining.cpp#L338)
```cpp
             "  \"bits\" : \"xxxxxxxx\",              (string) compressed target of next block\n"
             "  \"height\" : n                      (numeric) The height of the next block\n"
+            "  \"default_witness_commitment\" : \"xxxx\" (string) coinbase witness commitment \n"
+            "  \"pop_context\" : \n"
+            "    \"serialized\": \"xxx\"     (string) serialized version of AuthenticatedContextInfoContainer\n"
+            "    \"stateRoot\" : \"xxx\"     (string) Hex-encoded StateRoot=sha256d(txRoot, popDataRoot)\n"
+            "    \"context\"   : {\n"
+            "      \"height\"    : 123       (numeric) Current block height.\n"
+            "      \"firstPreviousKeystone\": \"xxx\"  (string) First previous keystone of current block.\n"
+            "      \"secondPreviousKeystone\": \"xxx\" (string) Second previous keystone of current block.\n"
+            "      }\n"
+            "    }\n"
+            "  \"pop_data_root\" : \"xxxx\"   (string) Merkle Root of PopData\n"
+            "  \"pop_data\" : { \"atvs\": [], \"vtbs\": [], \"vbkblocks\": [] }   (object) Valid POP data that must be included in next block in order they appear here (vbkblocks, vtbs, atvs).\n"
+            "  \"pop_payout\" : [                 (array) List of POP payouts that must be addedd to next coinbase in order they appear in array.\n"
+            "    \"payout_info\": \"...\",\n"
+            "    \"amount\": xxx\n"
+            "   ]\n"
             "}\n"
```
[method getblocktemplate](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/mining.cpp#L338)
```cpp
     if(!g_rpc_node->connman)
         throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

-    if (g_rpc_node->connman->GetNodeCount(CConnman::CONNECTIONS_ALL) == 0)
-        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, PACKAGE_NAME " is not connected!");
+    // VERIBLOCK: when node does not have other peers, this disables certain RPCs. Disable this condition for now.
+    //  if (g_rpc_node->connman->GetNodeCount(CConnman::CONNECTIONS_ALL) == 0)
+    //      throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, PACKAGE_NAME " is not connected!");
```
[method getblocktemplate](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/rpc/mining.cpp#L338)
```cpp
+    //VeriBlock Data
+    const auto popDataRoot = pblock->popData.getMerkleRoot();
+    result.pushKV("pop_data_root", HexStr(popDataRoot.begin(), popDataRoot.end()));
+    result.pushKV("pop_data", altintegration::ToJSON<UniValue>(pblock->popData, /*verbose=*/true));
+    using altintegration::ContextInfoContainer;
+    auto ctx = ContextInfoContainer::createFromPrevious(VeriBlock::GetAltBlockIndex(pindexPrev), VeriBlock::GetPop().getConfig().getAltParams());
+    result.pushKV("pop_first_previous_keystone", HexStr(ctx.keystones.firstPreviousKeystone));
+    result.pushKV("pop_second_previous_keystone", HexStr(ctx.keystones.secondPreviousKeystone));
+
+    // pop rewards
+    UniValue popRewardsArray(UniValue::VARR);
+    VeriBlock::PoPRewards popRewards = VeriBlock::getPopRewards(*pindexPrev, Params());
+    for (const auto& itr : popRewards) {
+        UniValue popRewardValue(UniValue::VOBJ);
+        popRewardValue.pushKV("payout_info", HexStr(itr.first.begin(), itr.first.end()));
+        popRewardValue.pushKV("amount", itr.second);
+        popRewardsArray.push_back(popRewardValue);
+    }
+    result.pushKV("pop_rewards", popRewardsArray);
+
     return result;
```

# 6. Add RPC unit test.

Pop RPC test: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/rpc_service_tests.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/test/unit/rpc_service_tests.cpp). Copy this file to your project.

# 7. Add RPC service and test to the makefile.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```cpp
libbitcoin_server_a_SOURCES = \
   policy/settings.cpp \
   pow.cpp \
   rest.cpp \
+  vbk/rpc_register.hpp \
+  vbk/rpc_register.cpp \
   vbk/p2p_sync.hpp \
   vbk/p2p_sync.cpp \
   rpc/blockchain.cpp \
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```cpp
 ### VeriBlock section start
 # path is relative to src
 VBK_TESTS = \
   vbk/test/unit/e2e_poptx_tests.cpp \
   vbk/test/unit/block_validation_tests.cpp \
   vbk/test/unit/vbk_merkle_tests.cpp \
   vbk/test/unit/pop_reward_tests.cpp
   vbk/test/unit/pop_reward_tests.cpp \
-  vbk/test/unit/forkresolution_tests.cpp
+  vbk/test/unit/forkresolution_tests.cpp \
+  vbk/test/unit/rpc_service_tests.cpp
```

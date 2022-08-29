# Pop mempool support {#integration_6_mempool}

[TOC]

# Overview

Next step is to add the Pop mempool support for the ALT blockchain. We should implement methods for submitting Pop payloads to the mempool, fetching payloads during the block mining and removing payloads after successfully submitting a block to the blockchain.

# 1. Implement mempool related methods in the pop_service.hpp and pop_service.cpp source files.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp)
```cpp
+//! mempool methods
+altintegration::PopData getPopData();
+void removePayloadsFromMempool(const altintegration::PopData& popData);
+void addDisconnectedPopdata(const altintegration::PopData& popData);
+
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
+altintegration::PopData getPopData() EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    return GetPop().mempool->getPop();
+}
+
+void removePayloadsFromMempool(const altintegration::PopData& popData) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    AssertLockHeld(cs_main);
+    GetPop().mempool->removeAll(popData);
+}
+
+void addDisconnectedPopdata(const altintegration::PopData& popData) EXCLUSIVE_LOCKS_REQUIRED(cs_main)
+{
+    disconnected_popdata.push_back(popData);
+}
```

# 2. Add PopData during block mining.

Update `CreateNewBlock()` in the miner.cpp.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp)
```cpp
 #include <util/system.h>
 #include <util/validation.h>

+#include <vbk/pop_service.hpp>
+#include <vbk/util.hpp>
```
[method BlockAssembler::CreateNewBlock](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/miner.cpp#L96)
```cpp
+    // VeriBlock: add PopData into the block
+    if (chainparams.isPopActive(nHeight))
+    {
+        pblock->popData = VeriBlock::getPopData();
+    }
+
     if (!pblock->popData.empty()) {
         pblock->nVersion |= VeriBlock::POP_BLOCK_VERSION_BIT;
     }

     int64_t nTime1 = GetTimeMicros();
```

# 3. Remove PopData after successfully submitting to the blockchain.

Modify `ConnectTip()` and `DisconnectTip()` methods in the validation.cpp.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)

[method CChainState::DisconnectTip](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2429)
```cpp
+    // VeriBlock
+    VeriBlock::addDisconnectedPopdata(block.popData);
+
     m_chain.SetTip(pindexDelete->pprev);

     UpdateTip(pindexDelete->pprev, chainparams);
```
[method CChainState::ConnectTip](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L2557)
```cpp
     disconnectpool.removeForBlock(blockConnecting.vtx);
+
+    // VeriBlock: remove from pop_mempool
+    VeriBlock::removePayloadsFromMempool(blockConnecting.popData);
+
     // Update m_chain & related variables.
     m_chain.SetTip(pindexNew);
     UpdateTip(pindexNew, chainparams);
```

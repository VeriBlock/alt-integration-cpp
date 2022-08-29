# Adding persistence {#integration_5_persistence}

[TOC]

# Overview

ALT block storage should be modified to store VeriBlock related data.

# 1. Add PayloadsProvider.

We should add a `PayloadsProvider` for the VeriBlock library. The main idea of such class is that we reuse the existing ALT blockchain database. Our library allows to use the native implementation of the database. We implement it with `PayloadsProvider` class which is inherited from the altintegration::PayloadsStorage class.

Payloads provider: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/adaptors/payloads_provider.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/adaptors/payloads_provider.hpp). Copy this file to your project.

Block provider: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/adaptors/block_provider.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/adaptors/block_provider.hpp). Copy this file to your project.

@note These adaptors rely heavily on Bitcoin structures such as `CDBWrapper` and `CDBIterator`. Other blockchains should use own adaptors or other means for storing VeriBlock related data.

# 2. Create wrappers for the persistence API.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp)
```cpp
// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SRC_VBK_POP_SERVICE_HPP
#define BITCOIN_SRC_VBK_POP_SERVICE_HPP

#include "pop_common.hpp"
#include <vbk/adaptors/payloads_provider.hpp>
#include <vbk/util.hpp>

class CBlockTreeDB;
class CDBBatch;
class CDBIterator;
class CDBWrapper;

namespace VeriBlock {

void InitPopContext(CDBWrapper& db);

//! returns true if all tips are stored in database, false otherwise
bool hasPopData(CBlockTreeDB& db);
altintegration::PopData getPopData();
void saveTrees(CDBBatch* batch);
bool loadTrees(CDBWrapper& db);

} // namespace VeriBlock

#endif //BITCOIN_SRC_VBK_POP_SERVICE_HPP
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp)
```cpp
// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <consensus/validation.h>
#include <dbwrapper.h>
#include <shutdown.h>
#include <validation.h>
#include <vbk/adaptors/payloads_provider.hpp>
#include <veriblock/storage/util.hpp>

#ifdef WIN32
#include <boost/thread/interruption.hpp>
#endif //WIN32

#include "pop_service.hpp"
#include <utility>
#include <vbk/adaptors/block_provider.hpp>
#include <vbk/pop_common.hpp>

namespace VeriBlock {

void InitPopContext(CDBWrapper& db)
{
    auto payloads_provider = std::make_shared<PayloadsProvider>(db);
    SetPop(payloads_provider);
}

bool hasPopData(CBlockTreeDB& db)
{
    return db.Exists(tip_key<altintegration::BtcBlock>()) &&
           db.Exists(tip_key<altintegration::VbkBlock>()) &&
           db.Exists(tip_key<altintegration::AltBlock>());
}

void saveTrees(CDBBatch* batch)
{
    AssertLockHeld(cs_main);
    VeriBlock::BlockBatch b(*batch);
    altintegration::SaveAllTrees(*GetPop().altTree, b);
}
bool loadTrees(CDBWrapper& db)
{
    altintegration::ValidationState state;

    BlockReader reader(db);
    if (!altintegration::LoadAllTrees(GetPop(), reader, state)) {
        return error("%s: failed to load trees %s", __func__, state.toString());
    }

    return true;
}
```

@note You can copy provided [pop_service.hpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.hpp) and [pop_service.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/pop_service.cpp) files and comment out everything but storage related API.

# 3. Initialize VeriBlock storage during ALT blockchain initialization process.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp)
```cpp
+#include <vbk/pop_service.hpp>
```
[method Shutdown](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp#L177)
```cpp
     threadGroup.interrupt_all();
     threadGroup.join_all();
+    VeriBlock::StopPop();
```
[method AppInitMain](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/init.cpp#L1209)
```cpp
                 pblocktree.reset();
                 pblocktree.reset(new CBlockTreeDB(nBlockTreeDBCache, false, fReset));
+                VeriBlock::InitPopContext(*pblocktree);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/txdb.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/txdb.cpp)
```cpp
 #include <boost/thread.hpp>
+#include <vbk/pop_service.hpp>
```
[method CBlockTreeDB::WriteBatchSync](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/txdb.cpp#L226)
```cpp
     }
+
+    // write BTC/VBK/ALT blocks
+    VeriBlock::saveTrees(&batch);
     return WriteBatch(batch, true);
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp)
```cpp
 #include <warnings.h>

+#include <vbk/pop_service.hpp>
+#include <vbk/pop_common.hpp>
+#include <vbk/util.hpp>
```
[method BlockManager::LoadBlockIndex](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L4177)
```cpp
         return false;

+    bool hasPopData = VeriBlock::hasPopData(blocktree);
+
+    if (!hasPopData) {
+        LogPrintf("BTC/VBK/ALT tips not found... skipping block index loading\n");
+        return true;
+    }
+
     // Calculate nChainWork
```
[method BlockManager::LoadBlockIndex](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/validation.cpp#L4177)
```cpp
         if (pindex->IsValid(BLOCK_VALID_TREE) && (pindexBestHeader == nullptr || CBlockIndexWorkComparator()(pindexBestHeader, pindex)))
             pindexBestHeader = pindex;
     }
+
+    // get best chain from ALT tree and update vBTC's best chain
+    {
+        AssertLockHeld(cs_main);
+
+        // load blocks
+        if(!VeriBlock::loadTrees(blocktree)) {
+            return false;
+        }
     }
```

# 4. Update the constructor of the TestingSetup struct.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp)

[method TestingSetup::TestingSetup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L102)
```cpp
     pblocktree.reset(new CBlockTreeDB(1 << 20, true));
+    VeriBlock::InitPopContext(*pblocktree);
```
[method TestingSetup::~TestingSetup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L146)
```cpp
     threadGroup.interrupt_all();
     threadGroup.join_all();
+    VeriBlock::StopPop();
```
[method TestChain100Setup::TestChain100Setup](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/test/util/setup_common.cpp#L162)
```cpp
+    assert(ChainActive().Tip() != nullptr);
+    assert(ChainActive().Tip()->nHeight == 100);
+    assert(BlockIndex().size() == 101);
```

# 5. Restore VeriBlock block hash from the storage - do not recalculate it if possible.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/dbwrapper.h](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/dbwrapper.h)
```cpp
+// VeriBlock: put VBK block hash in the block index
+template <>
+inline bool CDBIterator::GetValue(altintegration::BlockIndex<altintegration::VbkBlock>& value)
+{
+    leveldb::Slice slValue = piter->value();
+    try {
+        CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
+        ssValue.Xor(dbwrapper_private::GetObfuscateKey(parent));
+        std::pair<char, altintegration::VbkBlock::hash_t> key;
+        if (!GetKey(key)) return false;
+        UnserializeWithHash(ssValue, value, key.second);
+    } catch (const std::exception&) {
+        return false;
+    }
+    return true;
+}
```

# 6. Add pop_service.cpp to the makefile.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```cpp
 libbitcoin_server_a_SOURCES = \
+  vbk/pop_service.hpp \
+  vbk/pop_service.cpp \
   addrdb.cpp \
```

var integration =
[
    [ "Integrate POP into BTC fork", "integration.html#autotoc_md95", null ],
    [ "Create new fork from existing BTC+POP chain", "integration.html#autotoc_md96", null ],
    [ "Modifying build setup", "integration_1_build.html", [
      [ "Overview", "integration_1_build.html#autotoc_md29", null ],
      [ "1. Build and install veriblock-pop-cpp library.", "integration_1_build.html#autotoc_md30", null ],
      [ "2. Add veriblock-pop-cpp library dependency.", "integration_1_build.html#autotoc_md31", null ]
    ] ],
    [ "Adding PopData", "integration_2_popdata.html", [
      [ "Overview", "integration_2_popdata.html#autotoc_md32", null ],
      [ "1. Helper for the block hash serialization.", "integration_2_popdata.html#autotoc_md33", null ],
      [ "2. Define POP_BLOCK_VERSION_BIT flag.", "integration_2_popdata.html#autotoc_md34", null ],
      [ "3. Add new PopData field into the BlockTransactions, CBlockHeaderAndShortTxIDs, PartiallyDownloadedBlock and update their serialization/deserialization.", "integration_2_popdata.html#autotoc_md35", null ],
      [ "4. Update PartiallyDownloadedBlock initializing - set PopData field.", "integration_2_popdata.html#autotoc_md36", null ],
      [ "5. Update setting up the PopData fields during the net processing.", "integration_2_popdata.html#autotoc_md37", null ],
      [ "6. Update validation rules.", "integration_2_popdata.html#autotoc_md38", null ],
      [ "7. Update the mining code to setup POP_BLOCK_VERSION_BIT if VeriBlock PopData is contained in the block.", "integration_2_popdata.html#autotoc_md39", null ],
      [ "8. Overload serialization operations for the VeriBlock PopData and other VeriBlock entities.", "integration_2_popdata.html#autotoc_md40", null ]
    ] ],
    [ "Choosing fork point", "integration_3_forkpoint.html", [
      [ "Overview", "integration_3_forkpoint.html#autotoc_md41", null ],
      [ "1. Add block height for Pop Security activation.", "integration_3_forkpoint.html#autotoc_md42", null ],
      [ "2. Create a function in the chainparams which detects if the Pop security is enabled.", "integration_3_forkpoint.html#autotoc_md43", null ],
      [ "3. Update the block version validation.", "integration_3_forkpoint.html#autotoc_md44", null ]
    ] ],
    [ "Adding configuration", "integration_4_config.html", [
      [ "Overview", "integration_4_config.html#autotoc_md45", null ],
      [ "1. Create two new source files: pop_common.hpp, pop_common.cpp.", "integration_4_config.html#autotoc_md46", null ],
      [ "2. Add bootstraps blocks.", "integration_4_config.html#autotoc_md47", null ],
      [ "3. Create AltChainParamsBTC class with VeriBlock configuration of the ALT blockchain.", "integration_4_config.html#autotoc_md48", null ],
      [ "4. Update the initialization of the bitcoind, bitcoin-wallet, etc to setup VeriBlock config.", "integration_4_config.html#autotoc_md49", null ],
      [ "5. Update test chain setup to allow adding block to specific previous block.", "integration_4_config.html#autotoc_md50", null ],
      [ "6. Update makefiles. Add new source files.", "integration_4_config.html#autotoc_md51", null ]
    ] ],
    [ "Adding persistence", "integration_5_persistence.html", [
      [ "Overview", "integration_5_persistence.html#autotoc_md52", null ],
      [ "1. Add PayloadsProvider.", "integration_5_persistence.html#autotoc_md53", null ],
      [ "2. Create wrappers for the persistence API.", "integration_5_persistence.html#autotoc_md54", null ],
      [ "3. Initialize VeriBlock storage during ALT blockchain initialization process.", "integration_5_persistence.html#autotoc_md55", null ],
      [ "4. Update the constructor of the TestingSetup struct.", "integration_5_persistence.html#autotoc_md56", null ],
      [ "5. Restore VeriBlock block hash from the storage - do not recalculate it if possible.", "integration_5_persistence.html#autotoc_md57", null ],
      [ "6. Add pop_service.cpp to the makefile.", "integration_5_persistence.html#autotoc_md58", null ]
    ] ],
    [ "Pop mempool support", "integration_6_mempool.html", [
      [ "Overview", "integration_6_mempool.html#autotoc_md59", null ],
      [ "1. Implement mempool related methods in the pop_service.hpp and pop_service.cpp source files.", "integration_6_mempool.html#autotoc_md60", null ],
      [ "2. Add PopData during block mining.", "integration_6_mempool.html#autotoc_md61", null ],
      [ "3. Remove PopData after successfully submitting to the blockchain.", "integration_6_mempool.html#autotoc_md62", null ]
    ] ],
    [ "AltTree management", "integration_7_alttree.html", [
      [ "Overview", "integration_7_alttree.html#autotoc_md63", null ],
      [ "1. Implement AltTree related methods in the pop_service.hpp and pop_service.cpp source files.", "integration_7_alttree.html#autotoc_md64", null ],
      [ "2. Update block processing in the ConnectBlock(), DisconnectBlock(), UpdateTip(), LoadGenesisBlock(), AcceptBlockHeader(), AcceptBlock(), TestBlockValidity().", "integration_7_alttree.html#autotoc_md65", null ],
      [ "3. Show Pop related info when node starts.", "integration_7_alttree.html#autotoc_md66", null ],
      [ "4. Update Pop logging support.", "integration_7_alttree.html#autotoc_md67", null ],
      [ "5. Add unit tests to test the functionality we have added before.", "integration_7_alttree.html#autotoc_md68", null ],
      [ "6. Add test case which tests the VeriBlock Pop behaviour: e2e_poptx_tests.cpp.", "integration_7_alttree.html#autotoc_md69", null ],
      [ "7. Update makefile to enable new unit test.", "integration_7_alttree.html#autotoc_md70", null ]
    ] ],
    [ "Pop Merkle trees", "integration_8_merkle.html", [
      [ "Overview", "integration_8_merkle.html#autotoc_md71", null ],
      [ "1. VeriBlock Merkle root related functions are implemented in the merkle.hpp and merkle.cpp.", "integration_8_merkle.html#autotoc_md72", null ],
      [ "2. Use extended block weight calculation method that appends Pop data size.", "integration_8_merkle.html#autotoc_md73", null ],
      [ "3. Extend ValidationState class for better veriblock-pop-cpp error processing.", "integration_8_merkle.html#autotoc_md74", null ],
      [ "4. Update the mining process with Pop Merkle root calculation.", "integration_8_merkle.html#autotoc_md75", null ],
      [ "5. Since Pop Merkle root algorithm depends on the blockchain, we should move Merkle root validation from the CheckBlock() to the ContextualCheckBlock().", "integration_8_merkle.html#autotoc_md76", null ],
      [ "6. Add helper genesis_common.cpp file that allows Genesis block generation.", "integration_8_merkle.html#autotoc_md77", null ],
      [ "7. Add new tests: block_validation_tests.cpp, vbk_merkle_tests.cpp.", "integration_8_merkle.html#autotoc_md78", null ],
      [ "8. Add Pop Merkle trees code to the makefile.", "integration_8_merkle.html#autotoc_md79", null ],
      [ "9. Update makefile to run tests.", "integration_8_merkle.html#autotoc_md80", null ]
    ] ],
    [ "Pop rewards", "integration_9_rewards.html", [
      [ "Overview", "integration_9_rewards.html#autotoc_md81", null ],
      [ "1. Modify ALT params (CChainParams). Add two new VeriBlock parameters for the Pop rewards.", "integration_9_rewards.html#autotoc_md82", null ],
      [ "2. Implement Pop rewards related methods in the pop_service.hpp and pop_service.cpp source files.", "integration_9_rewards.html#autotoc_md83", null ],
      [ "3. Modify GetBlockSubsidy() to accept CChainParams instead of consensus params.", "integration_9_rewards.html#autotoc_md84", null ],
      [ "4. Modify mining process in the CreateNewBlock function. Insert VeriBlock PoPRewards into the coinbase transaction, add some validation rules to the validation.cpp.", "integration_9_rewards.html#autotoc_md85", null ],
      [ "5. Add tests for the Pop rewards.", "integration_9_rewards.html#autotoc_md86", null ],
      [ "6. Update makefile to run tests.", "integration_9_rewards.html#autotoc_md87", null ]
    ] ],
    [ "Pop fork resolution", "integration_10_fr.html", [
      [ "Overview", "integration_10_fr.html#autotoc_md4", null ],
      [ "1. Add fork resoultion functions to the pop_service.cpp and pop_service.hpp.", "integration_10_fr.html#autotoc_md5", null ],
      [ "2. Update validation.cpp to support Pop fork resolution.", "integration_10_fr.html#autotoc_md6", null ],
      [ "3. Add Pop fork resolution unit test.", "integration_10_fr.html#autotoc_md7", null ],
      [ "4. Update makefile to run tests.", "integration_10_fr.html#autotoc_md8", null ]
    ] ],
    [ "Update P2P protocol", "integration_11_p2p.html", [
      [ "Overview", "integration_11_p2p.html#autotoc_md9", null ],
      [ "1. Add P2P service files: p2p_sync.hpp, p2p_sync.cpp.", "integration_11_p2p.html#autotoc_md10", null ],
      [ "2. Introduce new protocol version.", "integration_11_p2p.html#autotoc_md11", null ],
      [ "3. Allow node to download chain with less chainWork.", "integration_11_p2p.html#autotoc_md12", null ],
      [ "4. Update Ping and Pong calls to provide best block hash.", "integration_11_p2p.html#autotoc_md13", null ],
      [ "5. Offer and process Pop data.", "integration_11_p2p.html#autotoc_md14", null ],
      [ "6. Subscribe the library to the mempool events.", "integration_11_p2p.html#autotoc_md15", null ],
      [ "7. Add P2P service to the makefile.", "integration_11_p2p.html#autotoc_md16", null ]
    ] ],
    [ "New RPC calls", "integration_12_rpc.html", [
      [ "Overview", "integration_12_rpc.html#autotoc_md17", null ],
      [ "1. Add RPC service files: rpc_register.hpp, rpc_register.cpp.", "integration_12_rpc.html#autotoc_md18", null ],
      [ "2. Add JSON adaptor for the library that allows converting from altintegration::MempoolResult to UniValue object.", "integration_12_rpc.html#autotoc_md19", null ],
      [ "3. Update serialize.h to use new JSON convertor.", "integration_12_rpc.html#autotoc_md20", null ],
      [ "4. Add all new RPC functions to the RPC server.", "integration_12_rpc.html#autotoc_md21", null ],
      [ "5. Extend original RPC calls with Pop data.", "integration_12_rpc.html#autotoc_md22", null ],
      [ "6. Add RPC unit test.", "integration_12_rpc.html#autotoc_md23", null ],
      [ "7. Add RPC service and test to the makefile.", "integration_12_rpc.html#autotoc_md24", null ]
    ] ],
    [ "Setup ABFI", "integration_13_setup_abfi.html", [
      [ "Overview", "integration_13_setup_abfi.html#autotoc_md25", null ],
      [ "Altchain BFI API", "integration_13_setup_abfi.html#autotoc_md26", null ],
      [ "Altchain BFI last-finalized-btc endpoint", "integration_13_setup_abfi.html#autotoc_md27", null ],
      [ "Configuring and Building Altchain BFI From Source.", "integration_13_setup_abfi.html#autotoc_md28", null ]
    ] ],
    [ "Testing of POP-enabled BTC fork", "btctesting.html", [
      [ "Summary", "btctesting.html#autotoc_md88", null ],
      [ "Overview", "btctesting.html#autotoc_md89", null ],
      [ "Unit tests", "btctesting.html#autotoc_md90", null ],
      [ "BTC functional tests", "btctesting.html#autotoc_md91", null ],
      [ "POP functional tests", "btctesting.html#autotoc_md92", null ],
      [ "View APM get-operation", "btctesting.html#autotoc_md93", null ],
      [ "Altchain BFI", "btctesting.html#autotoc_md94", null ]
    ] ]
];
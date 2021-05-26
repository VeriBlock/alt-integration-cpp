# Testing of POP-enabled BTC fork {#btctesting}
​
[TOC]
​
# Summary
To show that the pop-enabled altchain most likely works, once could screenshots (with supporting links) of:
​
1. All certification tests passing.
![certification_tests](./img/certificationtests.png)

2. A single get-opereration from an APM.
![apm_getoperation_screenshot](./img/apmgetoperationscreenshot.png)
​
3. A running network with multiple APM instances. For example, multiple pop tx with different reward addresses on the VBK explorer.
@todo add a screenshot

4. Altchain explorer screenshot showing recent ATV and VTB.
![vbtc_explorer_screenshot](./img/vbtcexplorerscreenshot.png)

​
# Overview
​
Here we describe which POP related tests should be added and should pass to be confident that POP security works fine. All tests can be separated to the following categories:
- `unit tests`
- `BTC functional tests`
- `POP functional tests`
- `View APM get-operation`
​
# Unit tests
​
First thing is to make sure that all original BTC tests pass and work well.
​
On the Unix sytem you can do it with the following command: 
```bash
make check
``` 
​
Before starting the POP integration it is recommended to fix tests if some of them do not work properly.
​
Here is the full list of the POP unit tests:
- `e2e_poptx_tests.cpp`
- `pop_reward_tests.cpp`
- `vbk_merkle_tests.cpp`
- `block_validation_tests.cpp`
- `rpc_service_tests.cpp`
- `genesis_block_tests.cpp`
- `forkresolution_tests.cpp`
​
 
You can see the corresponding changes in the Makefile: [https://github.com/VeriBlock/vbk-ri-btc/blob/026fba4e80bc114c68c636e3c9cfc6af855c9c94/src/Makefile.test.include#L117](https://github.com/VeriBlock/vbk-ri-btc/blob/026fba4e80bc114c68c636e3c9cfc6af855c9c94/src/Makefile.test.include#L117)
​
These tests are available in the ```vbk/tests/unit``` directory.
​
@note Unit tests should pass prior to running BTC and POP functional tests.
​
# BTC functional tests
​
Original Bitcoin code has functional tests that reside in the ```test/functional``` directory. 
​
`bitcoind` should be built prior to running functional tests. ```VBITCOIND_PATH``` environment variable should be set to the path to the `bitcoind` daemon:
```bash
export VBITCOIND_PATH=<path to the daemon (bitcoind)>
```
​
Tests are started with the following command:
```bash
python test/functional/test_runner.py
```
​
# POP functional tests
​
These tests validate that all POP functionality works fine. All important parts of the POP security are covered by these tests. Therefore it is important to make sure that all tests pass before starting Mainnet or even Testnet.
​
VeriBlock Python framework should be installed to make these tests run. Follow the VeriBlock Python testing guide [here](https://github.com/VeriBlock/alt-integration-cpp/blob/master/pypoptools/README.md).
​
Now the following POP test runner can be copied to the source code: [https://github.com/VeriBlock/vbk-ri-btc/blob/master/test/integration/test_runner.py](https://github.com/VeriBlock/vbk-ri-btc/blob/master/test/integration/test_runner.py)
​
A ```node``` which acts as an adaptor for `bitcoind` has been implemented. This adaptor able to ```run```, ```stop```, ```restart``` `bitcoind` and execute RPC functions: [https://github.com/VeriBlock/alt-integration-cpp/blob/master/pypoptools/pypoptesting/altchain_node_adaptors/vbitcoind_node.py](https://github.com/VeriBlock/alt-integration-cpp/blob/master/pypoptools/pypoptesting/altchain_node_adaptors/vbitcoind_node.py)
​
For debugging purposes it is possible to run a single test from the test set:
```bash
python test_runner.py NodeBasicSyncTest
​
```
​
The result of successfully passing tests should look like this:
​
```bash
TEST                  | STATUS    | DURATION

NodeBasicSyncTest     | ✓ Passed  | 90 s
NodeConnectTest       | ✓ Passed  | 28 s
NodeGenerateTest      | ✓ Passed  | 8 s
NodePreflightTest     | ✓ Passed  | 15 s
NodeRestartTest       | ✓ Passed  | 32 s
NodeStartTest         | ✓ Passed  | 18 s
NodeStopTest          | ✓ Passed  | 17 s
PopActivateTest       | ✓ Passed  | 19 s
PopE2ETest            | ✓ Passed  | 41 s
PopForkResolutionTest | ✓ Passed  | 436 s
PopInitTest           | ✓ Passed  | 98 s
PopMempoolGetpopTest  | ✓ Passed  | 326 s
PopMempoolReorgTest   | ✓ Passed  | 38 s
PopMempoolSyncTest    | ✓ Passed  | 64 s
PopParamsTest         | ✓ Passed  | 8 s
PopSyncTest           | ✓ Passed  | 31 s

ALL                   | ✓ Passed  | 1269 s (accumulated) 
Runtime: 1273 s
```
​
# View APM get-operation
Run getoperation and view an e2e pop transaction.
@todo add a screenshot


​
# Altchain BFI
The latest [ABFI build](https://hub.docker.com/r/veriblock/altchain-bfi/tags?page=1&ordering=last_updated) can be downloaded through docker

### Altchain BFI configuration
The altchain BFI can be configured through the **application.conf** file which is inside the **bin** folder.

Configuration example:
```
bfi {
  altchainId = vbtc

  blockChainNetwork = testnet

  nodeCoreRpcHost = "127.0.0.1:10500"

  api {
    port = 4568
    notificationsTest = false
  }

  forkThreatThreshold = 2
  forkThreatRatioThreshold = 0.8

  debugOutput = false
}

securityInheriting {
  vbtc: {
    payoutAddress: "0x"
    pluginKey: btc
    id: 3860170
    name: "vBitcoin"
    host: "http://localhost:18332"
    auth: {
      username: "test"
      password: "test"
    }
  }
}
```

Alternatively you can use the next environment variables to configure the ABFI:

|Configuration name          | Environment variable           | Default           | Description                                                                   |
|----------------------------|--------------------------------|-------------------|-------------------------------------------------------------------------------|
| altchainId                 | ALTCHAIN_ID                    | undefined         | The altchain id, it must be configured                                        |   
| blockChainNetwork          | NETWORK                        | testnet           | The altchain network, there are three options: mainnet, testnet and regtest   |
| blockChainRegtestHost      | REGTEST_HOST                   | undefined         | The altchain regtest host (the network should be specified as regtest)        |
| nodeCoreRpcHost            | NODECORE_HOST                  | 127.0.0.1:10500   | The NodeCore ip where the ABFI will connects to                               |
| notificationsTest          | HTTP_API_NOTIFICATIONS_TEST    | false             | Enables the /notifications-test endpoint to trigger test notifications        |
| forkThreatThreshold        | FORK_THREAT_THRESHOLD          | 2                 |                                                                               |
| forkThreatRatioThreshold   | FORK_THREAT_RATIO_THRESHOLD    | 0.8               |                                                                               |
| debugOutput                | DEBUG_OUTPUT                   | false             | Generate debug log files

You can add chains to the securityInheriting block, for example:

| Configuration name | Description                                                                                                              |
|--------------------|--------------------------------------------------------------------------------------------------------------------------|
| payoutAddress      | This configuration should be set in order to make the ABFI work but it will be removed, set it to "0x"                   |
| pluginKey          | The plugin to load                                                                                                       |
| id                 | The chain id                                                                                                             |
| name               | The chain name                                                                                                           |
| host               | API url from the coin daemon                                                                                             |
| auth: username     | The username used on the daemon authentication (if any), it can be disabled by commenting the whole auth config block.   |
| auth: password     | The password used on the daemon authentication (if any), it can be disabled by commenting the whole auth config block.   |

### Altchain BFI API

The next endpoints are accessible at the ABFI API (http://localhost:8080 by default):

| Endpoint                               | Description                                                                            |
|----------------------------------------|----------------------------------------------------------------------------------------|
| /api/chains                            | Get the last 20 best blocks from the blockchain and any fork blocks at the same height |
| /api/chains/best                       | Get the best chain information                                                         |
| /api/chains/blocks/{hash}              | Get the SI block for the given hash                                                    |
| /api/chains/blocks/best/{height}       | Get the SI best block for the given height                                             |
| /api/chains/blocks/last-finalized      | Get the last finalized block                                                           |
| /api/chains/blocks/last-finalized-btc  | Get the last block finalized by btc                                                    |

### Altchain BFI last-finalized-btc endpoint
In order to get responses there should be at least one APM mining at the altchain network also it could take a while (hours), here is a response example for that endpoint:
<details>
  <summary>Example</summary>

````json
{
  "id" : "B6D1B863FD54FA855B",
  "height" : 11440,
  "hash" : "000000001E802E0F5AB2CA888290A707B131E3F1DA4A8BB6D1B863FD54FA855B",
  "previousHash" : "0000000AE242C1C892AEA8A3A2682F228289F75F5946530D766CA766BD7768A1",
  "previousKeystone" : "00000001E1A35BD73304BD632DB553613650D73C85153831C58C1B500C773E27",
  "secondKeystone" : "000000027A1B454934C0BFBFF7DD3F40C869882700C6D5817D571A90E9D1AF35",
  "known" : true,
  "popVerified" : true,
  "verifiedIn" : [ {
    "height" : 1576960,
    "hash" : "0000000E68FA6A30244E8532D01CFEBB81BAEB44D6450E28"
  } ],
  "endorsedIn" : [ {
    "height" : 1576960,
    "hash" : "0000000E68FA6A30244E8532D01CFEBB81BAEB44D6450E28"
  }, {
    "height" : 1576966,
    "hash" : "0000000A6040FF891E758D33E1DE54CAA29A4F6AB84E5D30"
  }, {
    "height" : 1576972,
    "hash" : "00000007480E7A9E697EF558B079B1ACDE8882C0EF1B3606"
  }, {
    "height" : 1576980,
    "hash" : "0000000DF194B1596BA7D370D648C7AE16040C0D7EB9CA44"
  }, {
    "height" : 1576982,
    "hash" : "0000001364EBF5245B4A675C10C17F18BE37F635F8670187"
  }, {
    "height" : 1576983,
    "hash" : "0000000D7E0EC709ADFC57AF215FC889A2A1E25E28E04206"
  }, {
    "height" : 1576994,
    "hash" : "000000025C294CA53B43F54FC69C585AAE4C3CD0D416AAA7"
  }, {
    "height" : 1576996,
    "hash" : "000000143032BD8EFB60F1634C9A1F6AFB41E415B6A09E11"
  }, {
    "height" : 1576998,
    "hash" : "000000084D8751DDE79A6ADDEBEB750E95A1CD4E43B5BC6F"
  }, {
    "height" : 1577003,
    "hash" : "000000130E12451DD48F579A4A4F52AD08FAB653885C89DE"
  }, {
    "height" : 1577004,
    "hash" : "0000000D7D6700CF31C70D157CE1F7F362265C6D66AA2CEA"
  }, {
    "height" : 1577005,
    "hash" : "0000000AE903A898C5F2B6A81EBEB4810B1D08590BE22242"
  }, {
    "height" : 1577006,
    "hash" : "000000041D9555371540E9E2C1C0EAC249F63E9FC31C0AD8"
  }, {
    "height" : 1577007,
    "hash" : "00000013377AE0B4A9CB10B176844CAE7ACE051C055DAD70"
  }, {
    "height" : 1577009,
    "hash" : "000000011253477867C52CDE9FE93BA85F2A182643B00FCA"
  } ],
  "spFinality" : 629,
  "bitcoinFinality" : 19,
  "isAttackInProgress" : false
}
````

</details>

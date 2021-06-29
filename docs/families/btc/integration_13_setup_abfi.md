# Setup ABFI {#integration_13_setup_abfi}

[TOC]

# Overview

The Altchain Bitcoin Finality Indicator (ABFI) service provides an API that returns a number indicating the security of a transaction. This number is called bitcoinFinality and is found in the returned json data.

You can install this service and configure it for your altchain. To get ABFI, you can download this docker image: https://hub.docker.com/r/veriblock/altchain-bfi

Additional setup instructions are available at the docker link. If you are building ABFI from source instead of using the prebuilt docker image, then follow the instructions at the bottom of this page.

Once the ABFI service is running, make HTTP requests to the service through its IP address. For example, use the following URL in your browser to make a request.
```
http://[ip_to_altchain_bfi]:4568/api/chains/blocks/last-finalized-btc
```

# Altchain BFI API

The following endpoints are accessible at the ABFI API. Modify the URL just shown to make these requests.

| Endpoint                               | Description                                                                            |
|----------------------------------------|----------------------------------------------------------------------------------------|
| /api/chains                            | Get the last 20 best blocks from the blockchain and any fork blocks at the same height |
| /api/chains/best                       | Get the best chain information                                                         |
| /api/chains/blocks/{hash}              | Get the SI block for the given hash                                                    |
| /api/chains/blocks/best/{height}       | Get the SI best block for the given height                                             |
| /api/chains/blocks/last-finalized      | Get the last finalized block                                                           |
| /api/chains/blocks/last-finalized-btc  | Get the last block finalized by btc                                                    |

# Altchain BFI last-finalized-btc endpoint

In order to get responses there should be at least one APM mining at the altchain network also it could take a while (hours), here is a response example for that endpoint:

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

# Configuring and Building Altchain BFI From Source.

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

Alternatively you can use the following environment variables to configure the ABFI:

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

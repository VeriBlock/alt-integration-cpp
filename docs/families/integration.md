# Integration {#integration}

[TOC]

## Integrate POP into BTC fork

@note The documentation below is based on vBTC commit [`026fba4e80bc114c68c636e3c9cfc6af855c9c94`](https://github.com/VeriBlock/vbk-ri-btc/commit/026fba4e80bc114c68c636e3c9cfc6af855c9c94).

1. @subpage integration_1_build
2. @subpage integration_2_popdata
3. @subpage integration_3_forkpoint
4. @subpage integration_4_config
5. @subpage integration_5_persistence
6. @subpage integration_6_mempool
7. @subpage integration_7_alttree
8. @subpage integration_8_merkle
9. @subpage integration_9_rewards
10. @subpage integration_10_fr
11. @subpage integration_11_p2p
12. @subpage integration_12_rpc
13. @subpage integration_13_setup_abfi
14. @subpage btctesting

## Create new fork from existing BTC+POP chain

It is very easy to do.

#### 1. Fork vBTC

https://github.com/VeriBlock/vbk-ri-btc/

#### 2. Change naming as needed.

Search for all places where `vBitcoin` or `vBTC` is used, and replace with yours.

#### 3. Change network ID

vBTC uses network id to distinguish between different networks. 
Network ID is `pchMessageStart` bytes from [`chainparams.cpp`](https://github.com/bitcoin/bitcoin/blob/master/src/chainparams.cpp#L100-L103). 
They must be unique per chain/per network. 

@warning Don't forget to update these bytes in functional tests - https://github.com/VeriBlock/vbk-ri-btc/blob/master/test/functional/test_framework/mininode.py#L115-L119

#### 4. Change POP Altchain ID

Altchain ID must be unique among different POP-enabled chains. Set it to random 8-byte number.

https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/params.hpp#L33-L36

List of public POP-enabled chains: https://pop-coins.netlify.app/ (see field `ID`).

@note Technically it is possible for multiple chains to have same Altchain ID, but VeriBlock on-chain analysis will be broken, as multiple chains will be using same ID.

#### 5. Rebootstrap

See [rebootstrap guide](../rebootstrap.md).

#### 6. Make sure all unit and functional tests pass.

See [BTC testing guide](./btc/testing.md). 
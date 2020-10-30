# POP Integration {#integration}

This page describes steps needed to integrate POP protocol into **any** blockchain.

[TOC]

# Terms

- Altchain (ALT) - abstract blockchain that inherits security of Bitcoin.
- VeriBlock (VBK) - VeriBlock blockchain https://www.veriblock.org/.
- ATV - Cryptographic proof of Altchain block header publication in VeriBlock.
- VTB - Cryptographic proof of VeriBlock block header publication in Bitcoin.
- VbkBlock - VeriBlock block header.
- Payloads - ATV, VTB, VbkBlock sometimes can referred to as "payloads".
- APM - Altchain POP Miner.
- VPM - VeriBlock POP Miner.
- Security Providing Chain (SP Chain) - blockchain, which stores endorsements and is used for providing security for SI chain.
- Security Inheriting Chain (SI Chain) - blockchain, which consumes endorsements and utilizes properties of SP chain to improve its security.
- Block Of Proof - block in SP chain, which stores endorsement of block from SI chain.
- Endorsed Block - block in SI chain, for which we want to improve POP score. More endorsements of a block - higher POP score of this block.
- Containing Block - block in SI chain, which stores proof of endorsement.
- VBK TX - VeriBlock transaction.
- BTC TX - Bitcoin transaction.
- VBK POP TX - VeriBlock POP transaction.
- POP Payout Delay - PopPayoutsParams::getPopPayoutDelay() - POP payout will occur after this amount of blocks after endorsed block.
- Endorsement Settlement Interval - PopPayoutsParams::getEndorsementSettlementInterval() - validity window for ATV.
- POP MemPool - memory pool for POP-related payloads. Represents the content of the "next" block after current tip.
- SPV - simplified payment verification.
- Finalized/Finalization - this term can be applied to a block or transaction. A block is finalized if it can not be reorganized with very high probability (99.99%). For Bitcoin, finality is equal to 11 blocks. For VeriBlock, finality is 2000 blocks.

# Overview

![Network Topology](./img/topology.png)

POP security is built around "endorsing" blocks. 
When miners want to increase likelyhood of finalizing a block, they "endorse" it by creating "endorsing transaction" in SP chain.

In example above, we see 3 chains (top-down): Bitcoin, VeriBlock, Altchain.

In Altchain, we want to finalize block A100, then:
1. APM creates VBK TX, which contains PublicationData - an entity, which contains Endorsed Block header, and Payout Information - for rewarding APM for this endorsement.
2. This VBK TX is sent to VeriBlock. 
3. VBK POW miners create block V54 - this block will include this VBK TX from step above.
4. APM builds cryptographic proof that endorsement of Altchain block is added to main chain. This proof is named ATV (Altchain to VeriBlock publication) and includes: VBK TX, Context (VBK blocks connecting V54 to V51 - V52..V53), Block Of Proof (V54), Merkle Path that proves that VBK TX is really inside V54.
5. Then all this data is sent to Altchain, and added to POP MemPool.
6. When next ALT block is created by POW miner, this ATV is included into block A104.
7. If `POP payout delay` is 50, then payout for this endorsement will occur in block A150.

@note ATV is contextually valid within `Endorsement Settlement Interval`, so if A100 is endorsed, and Settlement Interval is 50, then ATV can be added in any new block in range A101..A150. If ATV is not added to main chain within this range, it will expire and will be removed from POP MemPool.

Continuing this example, VPM does exactly same algorithm but with VBK blocks.

@note Endorsement Settlement Interval for VeriBlock is 400 blocks, POP Payout Delay is also 400 blocks.

Block V54 contains proof for A100, and B16 contains proof for block V56. 
Since V56 is after V54, and V56 is finalized, we finalize all previous blocks, including V54, thus
Altchain transitively inherits security of Bitcoin.

@note We count endorsements, whose according block of proofs **are on main chains** of according blockchains. An endorsement whose block of proof is not on the main chain of SP chain is not eligible for POP Payout.

You can also notice that VTBs and VBK blocks are sent to Altchain. 
This is needed to communicate Bitcoin and VeriBlock consensus information to Altchain.

# Adding POP protocol

## 1. Select POP parameters

<code>
<pre>
\#include <veriblock/config.hpp>

// define config somewhere.
// it does not have to be globally accessible 
altintegration::Config config;
</pre>
</code>

Then, go to [POP Parameters](@ref popparameters) page to select correct parameters for your chain.

### 1.1 Derive AltChainParams and overwrite pure virtual methods

Create derived class, and implement pure virtual methods.

Then, overwrite default values in AltChainParams and PopPayoutsParams if needed. 
They are protected in PopPayoutsParams, so derive that class and set values once in constructor, then use instance of derived type in configs.

<code>
<pre>
struct AltChainParamsImpl : public altintegration::AltChainParams {
	// ...
    // derive pure virtual methods
    // ...

    AltChainParamsImpl() {
        // if needed, change default value of 'protected' params
        this->mKeystoneInterval = 50;
        // ...  
    }
};

// set this config into global Config
config.alt = std::make_shared<AltChainParamsImpl>();
</pre>
</code>

<table>
<caption>POP Parameters which must be defined by Altchain</caption>
<tr>
    <th>Parameter</th>
    <th>Description</th>
</tr>
<tr>
    <td>`Altchain ID`</td>
    <td>
    8-byte ID, sent with every endorsement of ALT block in VBK. 
    Used by VeriBlock to group potentially relevant endorsements. 
    It is not critical if ID is reused by multiple Altchains, but this 
    is not preferable.
    </td>
</tr>
<tr>
    <td>`Altchain Bootstrap Block`</td>
    <td>
    Bootstrap block is a very first block in ALT **which can be endorsed**.
    
    It is recommended to enable POP protocol during hard fork following this protocol: 
    1. Pick ALT block for hard fork, `X`
    2. Once current height is at `X`, wait `F` blocks (11 blocks in BTC) to finalize `X`
    3. Once current height is at `X+F`, enable POP protocol, and set block `X` as Bootstrap block.
    
    </td>
</tr>
<tr>
    <td>`GetHash`</td>
    <td>
    This function accepts serialized block header, and returns its hash.
    It is not guaranteed that input is always valid serialized block header.
    If input is empty or invalid block header, calculate hash anyway.
    
    This function **should not throw**.
    </td>
</tr>
</table>

### 1.2 Select Network parameters

AltBlockTree maintains all blocks of Bitcoin and VeriBlock starting at certain block - "bootstrap block".

Altchains must select VBK/BTC parameters which depend on currently active Altchain network.
Technically, there's no limitation on combinations main/test/regtest for ALT/VBK/BTC, but our recommendations are:
- For ALT mainnet, pick VBK mainnet and BTC mainnet.
- For ALT testnet, pick VBK testnet and BTC testnet.
- For ALT regtest, pick VBK regtest and BTC regtest.

@warning If you pick ALT mainnet, VBK testnet, BTC testnet, you will inherit security from **BTC testnet**, which **may decrease** security of Altchain.

### 1.3 Select Bootstrap blocks

In order to be able to validate BTC/VBK blocks properly, you must specify at least `K` blocks, where
- `K=2016` blocks for BTC
- `K=100` blocks for VBK

Bootstrapping protocol for BTC/VBK:
1. Select BTC/VBK network (mainnet, testnet, regtest...).
2. Download `K` **finalized** BTC/VBK block headers, serialize and encode them into hex.
3. Hardcode these blocks somewhere in your code:

    <code>
    <pre>
    //// bootstrap.h
    
    // example is ONLY for testnet. You need different bootstrap blocks for testnet/mainnet.
    
    // height of first block in vector below
    extern const int testnetVBKstartHeight;
    extern const std::vector<std::string> testnetVBKblocks;
    
    // height of first block in vector below
    extern const int testnetBTCstartHeight;
    extern const std::vector<std::string> testnetBTCblocks;
    
    //// bootstrap.cpp
    const int testnetVBKstartHeight=824724;
    const int testnetBTCstartHeight=1829048;
    
    const std::vector<std::string> testnetBTCblocks = {
     // hexencoded serialized block header at height 1829048
     "000000200674554E0A885FEAF780DE4E0970E3D9E50C7B0F0449B0DE0F0D000000000000F745F98B556C76AEB9F54CBD02547B3952E5C62C82EC93BAD501F8F01965C5C3E96C495FF0FF0F1A52CB338C",
     // ... after 2015 blocks
     // hexencoded serialized block header at height 1829048 + 2016
     "00000020133B7DF1D1D225A5BCA092ED8B9307EB8F52D56316B1FC766B0300000000000033AF87150B792B24A4A18B75428034E809CBAF2370A2E001C271D69700BF339F9FEF4F5FFCFF031A8EEE86D1"
    };
    
    const std::vector<std::string> testnetVBKblocks = {
     // hexencoded serialized block header at height 824724	
     "000C9594000240CA0959F4BE1023E2E65259A98BEFBCB675160185C43BB44957401197A294E4AC8D4CDD48CC9AC84A35586E61015F4F073C050226BAA301C11B",
     // ... after 2015 blocks
     "000C9D73000240B02B29D3B0F8C15C33FDFE9CBC4C2F8E9830BB0FB026DD8B643D8C334115601E7D1A342BED48D4C89632B9B78B5F4FF3C10502B5672F24D57E"     
    };
    
    </pre>
    </code>

4. Set these blocks into `Config`:

    <code>
    <pre>
    \#include <veriblock/blockchain/btc_chain_params.hpp>
    \#include <veriblock/blockchain/vbk_chain_params.hpp>
    
    // add case for mainnet
    if(btcNetwork == "test") {   
       config.setBTC(testnetBTCstartHeight, testnetBTCblocks, std::make_shared<altintegration::BtcChainParamsTest>());
    } else if(btcNetwork == "regtest") {
       config.setBTC(0, {/* no blocks */}, std::make_shared<altintegration::BtcChainParamsRegTest>());
    }
    
    // do same for mainnet
    if(vbkNetwork == "test") {
       config.setVBK(testnetVBKstartHeight, testnetVBKblocks, std::make_shared<altintegration::VbkChainParamsTest>());
    } else if(vbkNetwork == "regtest") {
       config.setVBK(0, {/* no blocks */}, std::make_shared<altintegration::VbkChainParamsRegTest>());
    } 
    
    </pre>
    </code>

<table>
<caption>Recommended number of blocks for bootstrapping BTC/VBK chains.</caption>
<tr>
    <th>Altchain Network</th>
    <th>Number of BTC bootstrap blocks</th>
    <th>Number of VBK bootstrap blocks</th>
</tr>
<tr>
    <td>`mainnet`</td>
    <td>2016</td>
    <td>2016 (min - 100)</td>
</tr>
<tr>
    <td>`testnet`</td>
    <td>2016</td>
    <td>2016 (min - 100)</td>
</tr>
<tr>
    <td>`regtest`</td>
    <td>0 (bootstrap with genesis block)</td>
    <td>0 (bootstrap with genesis block)</td>
</tr>
</table>

## 2. Implement PayloadsProvider and BlockBatchAdaptor

See comments on these interfaces. 

In this guide we assume derived class is called `PayloadsProviderImpl`.

## 3. Create PopContext instance

PopContext is main class-context for all data structures in this library.

<code>
<pre>
\#include <veriblock/storage/payloads_provider.hpp>
\#include <veriblock/pop_context.hpp>

std::shared_ptr<altintegration::PayloadsProvider> provider = std::make_shared<PayloadsProviderImpl>();
std::shared_ptr<altintegration::PopContext> popctx = altintegration::PopContext::create(config, provider);

// example of making popctx globally available
altintegration::PopContext& GetPopContext() {
    return *popctx;
}
</pre>
</code>

Make `popctx` globally available in your program.

## 4. Add PopData to Block structure

Typical Block consists of Block Header and Block Body.
In Bitcoin Block Body consists of [vector of transactions](https://github.com/bitcoin/bitcoin/blob/master/src/primitives/block.h#L66).

To enable POP, every block must carry additional POP-related data, we call it `altintegration::PopData`.

### 4.1 Add PopData to Block

PopData must become part of network/on-disk serialization of a Block Body. 

It is up to Altchain developers to decide if PopData is stored as entity or as bytes. 
To serialize an entity from library use `auto bytes = entity.toRaw()`, and to deserialize use `auto entity = T::fromRaw(bytes)`.

<code>
<pre>
struct CBlock {
    // regular transactions
    std::vector<CTransactionRef> vtx;
    
    // NEW: added POP-related data
    altintegration::PopData popData;
};
</pre>
</code>

### 4.2 Cryptographically authenticate PopData in Block Header

Altchain developers must ensure PopData is cryptographically authenticated in Block Header.

Regular transactions are typically cryptographically authenticated by inserting Merkle Root into Block Header, we will call it **Original Merkle Root**.

Merkle Root which is inserted into Block Header is now called **Top Level Merkle Root**.

![](./img/toplevelmroot.png)

**Algorithm to calculate Top Level Merkle Root**: 
1. Build Merkle Tree from individual items of PopData (those are VBK blocks, marked as VBK1 on the picture, VTBs and ATVs), resulting Merkle Root is called **Pop Root**.
2. Insert PopRoot into vector of leaves and calculate **Tx Merkle Root**
3. Calculate `ContextHash = HASH(BlockHeightBE || PreviousKeystoneHash || SecondPreviousKeystoneHash)`
   
   where
   - `BlockHeightBE` is 4 bytes of block height, serialized in Big-Endian
   - `PreviousKeystoneHash` is a hash of a Previous Keystone Block. Its height can be calculated with altintegration::getPreviousKeystone(int, int) 
   - 
   
4. Calculate `TopLevelMerkleRoot = HASH(TxMerkleRoot || ContextHash)`


## 5. Add POP-related RPC calls

<table>
<caption>New POP-related RPC calls that enable POP mining</caption>
<tr>
    <th>Name</th>
    <th>Input arguments</th>
    <th>Output arguments</th>
    <th>Example with vbitcoin-cli</th>
    <th>C++ code to get this value</th>
</tr>
<tr>
    <td>`getbtcbestblockhash` `getvbkbestblockhash`</td>
    <td>None</td>
    <td>String</td>
    <td>
<code>
<pre>
$ vbitcoin-cli getbtcbestblockhash
0000000000000170cc07da29a58fdfd7b2e53fd850fd43a1abbdb8ba6e5fd4e4
</pre>
</code>
    </td>
    <td>
<code>
<pre>
// replace btc() with vbk() for VBK tree
std::string btcbestblockhash = GetPopContext().altTree->btc().getBestChain().tip()->getHash().toHex();
</pre>
</code>
    </td>
</tr>

<tr>
    <td>`getbtcblock` `getvbkblock`</td>
    <td>Integer</td>
    <td>Object</td>
    <td>
<code>
<pre>
$ vbitcoin-cli getvbkblock 000000000cfb89eecc94396b7988de1ae2f253941b3ee6d9
{
  "chainWork": "0000000000000000000000000000000000000000000000000005684cae2a0000",
  "containingEndorsements": [
    "af690200fa5004f1bb929a720d8aba3e39667127f56a2dbeb5f3df0ecadc3789"
  ],
  "endorsedBy": [
  ],
  "height": 846727,
  "header": {
    "id": "7988de1ae2f253941b3ee6d9",
    "hash": "000000000cfb89eecc94396b7988de1ae2f253941b3ee6d9",
    "height": 846727,
    "version": 2,
    "previousBlock": "998945315c905c1ec31cb602",
    "previousKeystone": "5a31163e6490a445c9",
    "secondPreviousKeystone": "aa4395a38db7293bb8",
    "merkleRoot": "0133e83230c5af2fe92e69d6a1b0d46f",
    "timestamp": 1599681012,
    "difficulty": 84038094,
    "nonce": 2134427040
  },
  "status": 97,
  "ref": 1,
  "stored": {
    "vtbids": [
      "af690200fa5004f1bb929a720d8aba3e39667127f56a2dbeb5f3df0ecadc3789"
    ]
  }
}

</pre>
</code>
    </td>
    <td>
<code>
<pre>
// replace vbk() with btc() for BTC tree
auto* index = GetPopContext().altTree->vbk().getBlockIndex(hash);
if(index) {
    // block exists
    // convert this into JSON
    UniValue val = altintegration::ToJSON<UniValue>(*index);
} else {
    // block does not exist
}
</pre>
</code>
    </td>
</tr>

<tr>
    <td>`getbtcblockhash` `getvbkblockhash`</td>
    <td>Integer</td>
    <td>String</td>
    <td>
<code>
<pre>
$ vbitcoin-cli getvbkblockhash 846727
...

</pre>
</code>
    </td>
    <td>
<code>
<pre>
... TODO
</pre>
</code>
    </td>
</tr>

</table>

## 6. Implement p2p sync of payloads in MemPools.

## 7. Integrate POP rewards

## 8. Integrate POP fork resolution


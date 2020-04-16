# POP Integration Guide

### Introduction to POP

TODO

### Terms

- *POP* - Proof Of Proof protocol, which adds additional comparison criteria to altchain fork resolution algorithm. Using that, altchain is able to select chain that has most publications, and "better" publications. If chains have equal publications, then it uses regular native altchain's consensus (POW/POS/...). Alongside with new fork resolution criteria, POP miners are incentivized by altchain to create *publications*, as in case of successful publication, POP miner gets paid by an altchain.  
- *ATV* - Altchain to VeriBlock publication.
- *VTB* - VeriBlock to Bitcoin publication.
- *Payloads* - POP payloads. For VBK it is a `VTB`. For Altchain it is a `std::tuple<ATV, std::vector<VTB>>`  
- *SP chain* (Security Providing chain) - a blockchain that stores block headers from *SI chain*. 
- *SI chain* (Security Inheriting chain) - a blockchain that is protected by *SP chain*.
- *Publication (endorsement)* - an entity, that contains cryptographic proof that published block header of *SI chain* exists in *SP chain*.
- *Endorsed block* - altchain block that exists in *SI chain* and is published in *SP chain*.
- *POP miner* - a process that arbitrarily selects *endorsed block* from *SI chain*, creates a transaction in *SP chain*, which contains its block header, then creates a second transaction in *SI chain* with cryptographic proof that transaction with *endorsed block* exists in *SP chain*.
- *Keystone* - every N-th block. For keystone interval = 10, blocks 0, 10, 20..., 100, 110 are keystones.
- *Context* - a list of BTC and/or VBK blocks.

### Steps

#### 1. Every Full Block of Altchain should contain a list of valid ATVs and list of valid VTBs
Possible ways to implement:

##### 1.1 new transaction type - POP Transaction. 
This tx will contain at most one ATV, and 0..K VTBs. K is some upper limit, which depends on maximum transaction size. 
Whether this TX spends coins or no, signed or not - is up to altchain to decide. 
This TX will be a part of blockchain as any other TX, it will carry all necessary info to execute all of the POP-related algorithms.

In BTC you can add two new commands `OP_POPATV` and `OP_POPVTB`, which will be popping last byte array from the Script stack, and checking all cryptographic proofs.
POP payloads can be attached as an output of regular tx (e.g. "pop output") - for example, if output sends 0 to address `0000..vbk-pop-output` then interpret this output as POP output.

Pros: 
- no need to modify a block structure.
- no need to manage additional mempool for payloads
- payloads sync protocol is already implemented (as payloads are stored in txes, txes are synced via standard altchain's protocol)

Cons: 
- hard to track dependencies between transactions (depends on altchain)
- hard to do data deduplication - there could be multiple VTBs that carry exactly same context, and both of them make into POP tx, and increase size of blockchain.
- all existing code which works with transactions will have to distinguish between POP/non-POP, this may lead to many "ifs" in the code.
- necessity to validate POP transactions in your code (depends on altchain)

##### 1.2 add POP-meta into Full Block
Add a special container to a Block. 
In BTC this container can be created by POW miners, hash of that container (can be a merkle root) can be stored in one of non-paying outputs, similar to segwit commitment.

Container may look like:
```
struct PopMeta {
  std::vector<altintegration::AltPayloads> payloads;
};

struct CBlock {
  ...
  PopMeta pop;
};
```

This data will be a part of the block, so it has to be serialized/deserialized. 

Pros:
- easy to do data deduplication
- easy to implement and integrate separate mempool for Payloads (will likely be a part of altintegration) - it can do data deduplication and track all dependencies, by doing topological sort.
- easy to handle payloads inside the code - all existing tx processing code will remain unchanged

Cons:
- since we no longer store payloads in transactions, we have to store them in some additional mempool
- necessity to implement new p2p protocol to sync payloads between these mempools

#### 2. Add 2 new rpc calls

 - `submitpop <ATV> [VTB, VTB, ... VTB]` - POP miners will use it to submit POP payloads into altchain. It MUST accept exactly one ATV, and 0..K VTBs.
 - `getpopdata <blockhash or blockheight>` - POP miners will use it to pick "endorsed block" header, and POP-related data.

#### 3.  Add POP-related configuration options
Every POP-enabled altchain maintains SPV view on BTC and VBK blockchains. 
Considering that that, you have to bootstrap (e.g. init) Altchain's BTC blockchain with 2016 blocks, and 100 VBK blocks (one retargeting period) and keep adding blocks as part of Payloads context. 
This is necessary to build a view on BTC/VBK blockchains to correctly verify validity of Publications.

Altchain has to pick configuration options (TODO):
- BTC config:
	- at least 2016 BTC blocks
	- height of first BTC bootstrap block
- VBK config: 
	- at least 100 VBK blocks
	- height of first VBK bootstrap block
- Altchain config (see [`struct AltChainParams`](https://github.com/VeriBlock/alt-integration-cpp/blob/master/include/veriblock/blockchain/alt_chain_params.hpp#L10))

Best way is to hardcode this data into altchain.

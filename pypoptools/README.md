# pypopminer

### Install

Make sure boost-python is installed, and python3 is installed.
```bash
python3 setup.py install
```
Or if you build the whole library from source
```bash
# clone this repo and cd inside
mkdir build && cd build
cmake .. -DWITH_PYPOPTOOLS=TRUE
make -j
sudo make install

# you should see these lines
-- Installing: $USER/.local/lib/python3.8/site-packages/pypopminer/pypopminer.so
-- Up-to-date: $USER/.local/lib/python3.8/site-packages/pypopminer/__init__.py
```



Now pypopminer is installed.

### Use

```python
>> > from pypoptools import MockMiner, PublicationData
>> > m = MockMiner()
>> > m.vbkTip
VbkBlock
{
    height = 0, version = 2, prev = 000000000000000000000000, ks1 = 000000000000000000, ks2 = 000000000000000000, mroot = a2ea7c29ef7915db412ebd4012a9c617, timestamp = 1553699987, diff = 16842752, nonce = 0}
>> > m.btcTip
BtcBlock
{version = 1, prev = 0000000000000000000000000000000000000000000000000000000000000000, merkleRoot = 4
a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b, timestamp = 1296688602, bits = 545259519, nonce = 2}
>> > m.btcTip.toHex()
'0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494dffff7f2002000000'
```

Endorse VBK block
```python
>>> endorsed = m.vbkTip
# or
>>> endorsed = VbkBlock()
>>> endorsed.version = ... # fill all fields manually
>>>
>>> m.endorseVbkBlock(endorsed, lastKnownBtcHash, numberOfVtbs)
# or this:
# use prevBtcHash and prevVbkHash to determine where VTB/blockOfProof will be mined on chains
>>> m.endorseVbkBlock(endorsed, prevBtcHash, prevVbkHash, lastKnownBtcHash, numberOfVtbs)
```

Endorse ALT block:
```python
# to endorse altblock, create PublicationData
>>> p = PublicationData()
>>> p.identifier = 0x3ae6ca
# endorsed header in hex
>>> p.header = '0011223344'
>>> p.payoutInfo = '55667788'
>>> p
PublicationData{id=3860170, header=0011223344, payoutInfo=55667788}
>>> lastKnownVbkHash = '...' # in vbtc functional tests use node.getpopdata(height) to get it
>>> payloads = m.endorseAltBlock(p, lastKnownVbkHash)
>>> payloads
Payloads{atv, vtbs}
>>> payloads.atv
'0135bb01011667b55493722b4807bea7bb8ed2835d990885f3fe51c30203e80001070113033ae6ca010500112233440100010455667788473045022100a6e715b6905d622c3130b992daba751bada98fa7479523b238abfb7b5b7fc08102202086a06bd931e6ea251317cb7390adfd98b83fb0bdc0a1ebac4cbb484621a7fe583056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a806110fa05d9014a102a0513dd354ec50400000000040000000020631a0bad3e1087de13766f728e634ce7e4aa92c273bfcadec9c4e51b96323b3b0400000000400000000100026ad38f8a5995053b4b04d16c000000000000000000000000000000000000631a0bad3e1087de13766f728e634ce75ec7e6a101010000000000000100'
>>> payloads.vtbs
[]
```

### Pop RPCs in vBTC

##### `txid = submitpop <atvhex:str> [<vtbhex:str>...]`
```python
>>> txid = node.submitpop(payloads.atv, payloads.vtbs)
```

##### `data = getpopdata <height:int>`
```python
>>> response = node.getpopdata(5)
>> response
{
  "block_header": "00000020ec20db8d3d1cbe4810ef62f0938938283a64af30e3272a2b89ce6a3be44707087d26e4851031c640ff39b3412b2fb817c615d6c335d02614071cca2109934546a7e7c75effff7f2000000000",
  "raw_contextinfocontainer": "000000055ce92ce6d527f09870890b96d8f6a4e4351929305f4567db547710fa8398c4550000000000000000000000000000000000000000000000000000000000000000004b20f8704619bee607de3d42454ba4bb29ee2a485358860e716c41765a46f6",
  "last_known_veriblock_blocks": [
    "5113a60099c9f24260476a546ad38f8a5995053b4b04d16c"
  ],
  "last_known_bitcoin_blocks": [
    "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"
  ]
}
```

### vBTC functional tests

By default MockMiner is using BTC/VBK regtest.

By default vBTC node is bootstrapped with BTC/VBK regtest blocks, so it should be safe to use both with default params.

#### Construct payout info

```python
# generate new address
addr = node.getnewaddress()
# get pubkey for that address
pubkey = node.getaddressinfo(addr)['pubkey']
pkh = hash160(hex_str_to_bytes(pubkey))
script = CScript([OP_DUP, OP_HASH160, pkh, OP_EQUALVERIFY, OP_CHECKSIG])
payoutInfo = script.hex()

# to get a Key
    return Key(privkey=node.dumpprivkey(addr),
               pubkey=pubkey,
               p2pkh_script=CScript([OP_DUP, OP_HASH160, pkh, OP_EQUALVERIFY, OP_CHECKSIG]).hex(),
               p2pkh_addr=key_to_p2pkh(pubkey),
               p2wpkh_script=CScript([OP_0, pkh]).hex(),
               p2wpkh_addr=key_to_p2wpkh(pubkey),
               p2sh_p2wpkh_script=CScript([OP_HASH160, hash160(CScript([OP_0, pkh])), OP_EQUAL]).hex(),
               p2sh_p2wpkh_redeem_script=CScript([OP_0, pkh]).hex(),
               p2sh_p2wpkh_addr=key_to_p2sh_p2wpkh(pubkey))
```

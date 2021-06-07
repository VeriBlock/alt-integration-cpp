# Change Blockchain Explorer Params {#integration_14_explorer}

[TOC]

# Overview

Clone https://github.com/VeriBlock/vbk-ri-btc-explorer repo with explorer source code.

Create json file at `app/coins/youraltcain.js` (you should get other files as example). This file contain params of your altchain. Fill that file with genesis, chain and other params of your altchain. For example to get info from genesis coinbase TX you can print HEX encoded TX data to log in genesis block creation function (`CreateGenesisBlock`) and then use RPC `decoderawtransaction` to get info from this TX.

In file `app/coins.js` add your altchain instance.

After that steps you can run explorer:
1. Build explorer: `npm install`
2. Run your node with `-txindex` and `-server` params.
3. Run explorer: `npm run`
4. Open explorer page (default local link is [http://127.0.0.1:3002/](http://127.0.0.1:3002/))


# How to rebootstrap BTC and VBK blockchains {#rebootstrap}

#### Legend

- VBK - VeriBlock
- BTC - Bitcoin
- Finalized - blocks that have 100% chance of being final, e.g. which will never reorganize. For BTC it is same to assume that all blocks behind 11 blocks from current tip are final. For VBK, 2000.

#### Description

Rebootstrapping is a process of changing bootstrap blocks stored in altchain.

@note Example from vBTC: https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.cpp

To properly validate every ATV and VTB, Altchain should maintain a SPV view on BTC and VBK blockchains.
Every blockchain starts with a genesis block - a single hand-crafted block, which is a "root" of a tree.

Similarly, VBK and BTC blockchains should start with a root block.
Since we don't start at block 0, we should specify at least 1 `difficulty adjustment interval` worth of contiguous blocks, otherwise we won't be able to statefully validate their difficulty.
For BTC we require at least 2016 blocks, and for VBK we require at least 100 blocks.

#### Protocol

@warning For Altchain `mainnet` you ALWAYS should use VBK `mainnet` and BTC `mainnet`, otherwise there will be serious security issues.
@warning For Altchain Testnet, use VBK `testnet` and BTC `testnet`.
@warning For automated testing/`regtest`, use VBK `regtest_progpow` and BTC `regtest`.

1. Fetch 2016 finalized **contiguous** BTC headers, and save height of first header.
    - Find `btcStartHeight`: if last BTC block height is 1M, then use heights `1M - F - 2016` where `F` is number of confirmations to consider block final, and `2016` is the number of blocks in single BTC difficulty adjustment interval. In our example `btcStartHeight=1M-11-2016=997973`
    - Find hash of block on `btcStartHeight`, for example `000000000d9fcff7c03867d8d6331506cb9ed885be4b098d9df1ec6c552b54f9`
    - Use `getblockheader <hash>` RPC to get actual block header content. Serialized block header is always 80 bytes.
2. Fetch 100 finalized **contiguous** VBK headers, and save height of first header.
3. Hardcode `btcStartHeight`, `btcHeaders`, `vbkStartHeight`, `vbkHeaders` in `bootstraps.cpp`: https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/vbk/bootstraps.cpp

@warning ALL peers MUST have identical bootstrap blocks, otherwise they will fork, therefore we hardcode them in Altchain daemon. Similarly to having different genesis blocks of the same coin.

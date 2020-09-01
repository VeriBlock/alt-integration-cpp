# AltBlockTree {#altblocktree}

Represents simplified view on Altchain's block tree, maintains VBK tree and
BTC tree. VBK blocks and BTC blocks are added through Payloads (VTB, VbkBlock).

Helps Altchain to compare Altchain blocks by POP score (adding Endorsements
in VBK chain increases POP score) in POP Fork Resolution and to calculate POP
Rewards - payout to POP miners.

AltBlockTree is initialized with Altchain, Veriblock and Bitcoin parameters, as well as with PayloadsProvider implementation.
```c++
// use mainnet for all chains, for instance
AltChainParamsMain altp;
VbkChainParamsMain vbkp;
BtcChainParamsMain btcp;
// your implementation of PayloadsProvider
PayloadsProviderImpl provider;
AltBlockTree tree(altp, vbkp, btcp, provider);
```

After initialization AltBlockTree does not contain any blocks. 
**Users MUST bootstrap AltBlockTree** - add initial/genesis/root block of Altchain.

We will refer to this block as **bootstrap block**.
Bootstrap block is a first block in Altchain that can be endorsed, but can not contain POP body (PopData).

If `bootstrap()` fails, your block provided in struct AltChainParams::getBootstrapBlock() is invalid.

ValidationState will contain detailed info about failure reason, if any.

```c++
ValidationState state;
bool ret = tree.bootstrap(state);
assert(ret && "bootstrap is unsuccessful");
```

@invariant bootstrap block is immediately finalized - it can not be reorganized.

Whenever any new **full block** (block with header, block body and PopData) is accepted, 
users must first add block header to AltBlockTree, then add PopData to this block.
 
```c++
bool onNewFullBlock(Block block) {
    ValidationState state;
    
    // first, add block header
    if(!tree.acceptBlockHeader(block.getHeader(), state)) {
        // block header is invalid. Use state to get info about failure.
        return false;
    }
    
    // then, attach block body to block header
    tree.acceptBlock(block.getHash(), block.getPopData());
    
    // ...
``` 

AltBlockTree::acceptBlock() connects block immediately if all previous blocks are connected, or just adds block body to AltBlock when one of previous blocks is not connected.

After that, users can check if this block is connected:
```c++
    // ...
    // this returns nullptr if block can not be found
    auto* blockindex = tree.getBlockIndex(block.getHash());
    assert(blockindex && "we added this block to a tree, so it must exist");
    
    auto candidates = tree.getConnectedTips(*blockindex);
    if(candidates.empty()) {
      // we have no POP FR candidates
      return true; // block have been added
    }
    
    const auto* tip = tree.getBestChain().tip();
    for(const auto* candidate : candidates) {
        // here, we assume that candidate has all txes downloaded and block is fully available  
        
        // compare current tip to   
        int result = tree.comparePopScore(tip->getHash(), candidate->getHash());
```

@note after AltBlockTree::comparePopScore call tree always corresponds to winner chain

```c++
        if(result < 0) {
            // candidate has better POP score
            // tree already switched to candidate chain
            UpdateTip(candidate->getHash());
            // NOTE: update `tip`, otherwise old tip will be passed to first arg, and comparePopScore will die on assert
            tip = candidate;
            return true;
        } else if (result == 0) {
            // tip POP score == candidate POP score
            // tree tip is unchanged
            // fallback to chain-native Fork Resolution algorithm
            
            // in BTC, for example, compare blocks by chainwork
            if(tip->nChainWork < candidate->nChainWork) {
                // candidate has better chainwork
                UpdateTip(candidate->getHash());
            }
            return true;
        } else {
            // candidate is invalid or has worse POP score
            // tree tip is unchanged
            return true;
        }
    } // end of forloop
} // end of OnNewFullBlock
``` 

@invariant AltBlockTree::comparePopScore always compares current AltBlockTree tip to other block. To avoid confusion, you must specify tip explicitly. If incorrect tip is passed, function dies on assert.
@invariant AltBlockTree::comparePopScore always leaves AltBlockTree switched to winner chain.
@invariant Current active chain of AltBlockTree always corresponds to an empty tree with all applied blocks from first bootstrap block to current tip, i.e. currently applied active chain.

When tip is changed, Altchain MUST change state of AltBlockTree:

@note use AltBlockTree::setState() to switch from current best chain to new block. It is very expensive operation if there's large reorg. If altchain is already at this state, this is no-op.

```c++
void UpdateTip(uint256 bestHash) {
  ValidationState state;
  // setState returns true if all blocks on path (tip...bestHash] are valid
  bool ret = tree.setState(bestHash, state);
  assert(ret && "this block won FR, so it must be valid");
}
```
@invariant Current tip of your Altchain tree MUST correspond to `tree.getBestChain().tip()`, so calling AltBlockTree::setState() ensures they are in sync.

@see PayloadsProvider
@see AltChainParams
@see VbkChainParams
@see BtcChainParams
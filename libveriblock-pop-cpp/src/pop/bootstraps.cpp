// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/pop/bootstraps.hpp>

namespace altintegration {

// corresponds to "regtest_progpow" VBK network
VbkBlock GetRegTestVbkBlock() {
  //{
  //  "height": 0,
  //  "version": 2,
  //  "previousBlock": "000000000000000000000000",
  //  "previousKeystone": "000000000000000000",
  //  "secondPreviousKeystone": "000000000000000000",
  //  "merkleRoot": "80D7178046D25CA9AD283C5AF587A7C5",
  //  "timestamp": 1603044490,
  //  "difficulty": 0x1010000,
  //  "nonce": 0
  //}
  VbkBlock block;
  block.setHeight(0);
  block.setVersion(2);
  block.setMerkleRoot(uint128::fromHex("80D7178046D25CA9AD283C5AF587A7C5"));
  block.setTimestamp(1603044490);
  block.setDifficulty(0x1010000);
  block.setNonce(0);

  return block;
}

BtcBlock GetRegTestBtcBlock() {
  BtcBlock block(
      /*version=*/1,
      /*prev=*/uint256(),
      /*mroot=*/
      uint256::fromHex(
          "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"),
      /*time=*/1296688602,
      /*bits=*/0x207fffff,
      /*nonce=*/2);

  VBK_ASSERT(
      block.getHash() ==
      uint256::fromHex("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca59"
                       "0b1a11466e2206"));

  return block;
}

}  // namespace altintegration
// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/bootstraps.hpp>

namespace altintegration {

VbkBlock GetRegTestVbkBlock() {
  //  {
  //  "height": 0,
  //  "version": 2,
  //  "previousBlock": "000000000000000000000000",
  //  "previousKeystone": "000000000000000000",
  //  "secondPreviousKeystone": "000000000000000000",
  //  "merkleRoot": "a2ea7c29ef7915db412ebd4012a9c617",
  //  "timestamp": 1553699987,
  //  "difficulty": 117586646,
  //  "nonce": 1924857207
  //}
  VbkBlock block;
  block.height = 0;
  block.version = 2;
  block.merkleRoot = uint128::fromHex("a2ea7c29ef7915db412ebd4012a9c617");
  block.timestamp = 1553699987;
  block.difficulty = 16842752;
  block.nonce = 0;

  return block;
}

BtcBlock GetRegTestBtcBlock() {
  BtcBlock block;
  block.version = 1;
  block.timestamp = 1296688602;
  block.nonce = 2;
  block.bits = 0x207fffff;
  block.merkleRoot = uint256::fromHex(
      "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");

  VBK_ASSERT(
      block.getHash() ==
      uint256::fromHex("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca59"
                       "0b1a11466e2206"));

  return block;
}

}  // namespace altintegration
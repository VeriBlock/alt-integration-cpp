#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/stateless_validation.hpp"

using namespace VeriBlock;

TEST(StetlessValidation, checkBtcBlock_when_valid_test) {
  BtcBlock block;
  block.version = 536870912;
  block.previousBlock = uint256(
      "00000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def7"_unhex);
  block.merkleRoot = uint256(
      "5e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f8"_unhex);
  block.timestamp = 1555501858;
  block.bits = 436279940;
  block.nonce = 2599551022;

  ValidationState state;
  ASSERT_TRUE(checkBtcBlock(block, state));
}

TEST(StatelessValidation, checkBtcBlock_when_invalid_test) {
  BtcBlock block;
  block.version = 536870912;
  block.previousBlock = uint256(
      "00000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def7"_unhex);
  block.merkleRoot = uint256(
      "5e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f8"_unhex);
  block.timestamp = 1555501858;
  block.bits = 436279940;
  block.nonce = 1;

  ValidationState state;
  ASSERT_FALSE(checkBtcBlock(block, state));
}

TEST(StatelessValidation, checkVbkBlock_when_valid_test) {
  VbkBlock block;
  block.height = 5000;
  block.version = 2;
  block.previousBlock = uint144("94E7DC3E3BE21A96ECCF0FBD"_unhex);
  block.previousKeystone = uint72("F5F62A3331DC995C36"_unhex);
  block.secondPreviousKeystone = uint72("B0935637860679DDD5"_unhex);
  block.merkleRoot = uint128("DB0F135312B2C27867C9A83EF1B99B98"_unhex);
  block.timestamp = 1553699987;
  block.difficulty = 117586646;
  block.nonce = 1924857207;

  ValidationState state;
  // TODO
  // ASSERT_TRUE(checkVbkBlock(block, state));
}

#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/stateless_validation.hpp"

using namespace VeriBlock;

TEST(StetlessValidation, checkBtcBlock_when_valid_test) {
  BtcBlock block;
  block.version = 536870912;
  block.previousBlock = Sha256Hash(
      "00000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def7"_unhex);
  block.merkleRoot = Sha256Hash(
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
  block.previousBlock = Sha256Hash(
      "00000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def7"_unhex);
  block.merkleRoot = Sha256Hash(
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
  block.previousBlock = VBlakePrevBlockHash("94E7DC3E3BE21A96ECCF0FBD"_unhex);
  block.previousKeystone = VBlakePrevKeystoneHash("F5F62A3331DC995C36"_unhex);
  block.secondPreviousKeystone =
      VBlakePrevKeystoneHash("B0935637860679DDD5"_unhex);
  block.merkleRoot =
      VbkMerkleRootSha256Hash("67C9A83EF1B99B981ACBE73C1380F6DD"_unhex);
  block.timestamp = 1553699987;
  block.difficulty = 117586646;
  block.nonce = 1924857207;
}

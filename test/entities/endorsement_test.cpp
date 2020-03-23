#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "veriblock/entities/endorsement.hpp"

using namespace altintegration;

static const BtcEndorsement btcDefaultEndorsement{
    uint256(
        "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex),
    uint192(std::vector<uint8_t>(23, 3)),
    uint192(std::vector<uint8_t>(12, 2)),
    uint256(
        "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex),
    {}};

static const VbkEndorsement vbkDefaultEndorsement{
    uint256(
        "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex),
    {1, 2, 3},
    {4, 6, 12},
    uint192(std::vector<uint8_t>(12, 2)),
    std::vector<uint8_t>(16, 5)};

TEST(BtcEndorsement, RoundTrip) {
  std::vector<uint8_t> bytes = btcDefaultEndorsement.toVbkEncoding();
  BtcEndorsement deserialized =
      BtcEndorsement::fromVbkEncoding(std::string(bytes.begin(), bytes.end()));

  EXPECT_EQ(deserialized.id, btcDefaultEndorsement.id);
  EXPECT_EQ(deserialized.endorsedHash, btcDefaultEndorsement.endorsedHash);
  EXPECT_EQ(deserialized.containingHash, btcDefaultEndorsement.containingHash);
  EXPECT_EQ(deserialized.blockOfProof, btcDefaultEndorsement.blockOfProof);
}

TEST(VbkEndorsement, RoundTrip) {
  std::vector<uint8_t> bytes = vbkDefaultEndorsement.toVbkEncoding();
  VbkEndorsement deserialized =
      VbkEndorsement::fromVbkEncoding(std::string(bytes.begin(), bytes.end()));

  EXPECT_EQ(deserialized.id, vbkDefaultEndorsement.id);
  EXPECT_EQ(deserialized.endorsedHash, vbkDefaultEndorsement.endorsedHash);
  EXPECT_EQ(deserialized.containingHash, vbkDefaultEndorsement.containingHash);
  EXPECT_EQ(deserialized.blockOfProof, vbkDefaultEndorsement.blockOfProof);
  EXPECT_EQ(deserialized.payoutInfo, vbkDefaultEndorsement.payoutInfo);
}

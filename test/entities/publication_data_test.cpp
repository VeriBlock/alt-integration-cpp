#include "veriblock/entities/publication_data.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

static const std::string defaultPublicationEncoded =
    "0100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301"
    "117061796f757420696e666f206279746573";

static const PublicationData defaultPublication{
    0, "header bytes"_v, "payout info bytes"_v, "context info bytes"_v};

TEST(PublicationData, Deserialize) {
  auto pub = ParseHex(defaultPublicationEncoded);
  auto stream = ReadStream(pub);
  auto decoded = PublicationData::fromRaw(stream);

  EXPECT_EQ(decoded.identifier, defaultPublication.identifier);
  EXPECT_EQ(decoded.header, defaultPublication.header);
  EXPECT_EQ(decoded.payoutInfo, defaultPublication.payoutInfo);
  EXPECT_EQ(decoded.contextInfo, defaultPublication.contextInfo);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

TEST(PublicationData, Serialize) {
  WriteStream stream;
  defaultPublication.toRaw(stream);
  auto pubBytes = stream.data();
  auto pubEncoded = HexStr(pubBytes);
  EXPECT_EQ(pubEncoded, defaultPublicationEncoded);
}

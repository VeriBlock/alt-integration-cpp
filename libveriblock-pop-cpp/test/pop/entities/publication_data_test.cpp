// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/entities/publication_data.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

static const std::string defaultPublicationEncoded =
    "0100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301"
    "117061796f757420696e666f206279746573";

static const PublicationData defaultPublication{
    0, "header bytes"_v, "payout info bytes"_v, "context info bytes"_v};

TEST(PublicationData, DeserializeFromVbkEncoding) {
  auto decoded =
      AssertDeserializeFromHex<PublicationData>(defaultPublicationEncoded);

  EXPECT_EQ(decoded.identifier, defaultPublication.identifier);
  EXPECT_EQ(decoded.header, defaultPublication.header);
  EXPECT_EQ(decoded.payoutInfo, defaultPublication.payoutInfo);
  EXPECT_EQ(decoded.contextInfo, defaultPublication.contextInfo);
}

TEST(PublicationData, Serialize) {
  WriteStream stream;
  defaultPublication.toVbkEncoding(stream);
  auto pubBytes = stream.data();
  auto pubEncoded = HexStr(pubBytes);
  EXPECT_EQ(pubEncoded, defaultPublicationEncoded);
}

TEST(PublicationData, RoundTrip) {
  auto decoded =
      AssertDeserializeFromHex<PublicationData>(defaultPublicationEncoded);
  EXPECT_EQ(decoded.identifier, defaultPublication.identifier);

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto pubBytes = outputStream.data();
  auto pubReEncoded = HexStr(pubBytes);
  EXPECT_EQ(pubReEncoded, defaultPublicationEncoded);
}

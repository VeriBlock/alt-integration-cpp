#include "veriblock/entities/publication_data.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

TEST(PublicationData, Deserialize) {
  auto pub =
      "0100010C6865616465722062797465730112636F6E7465787420696E666F20627974657301117061796F757420696E666F206279746573"_unhex;
  auto stream = ReadStream(pub);
  auto decoded = PublicationData::fromRaw(stream);

  EXPECT_EQ(decoded.identifier, 0);
  EXPECT_EQ(decoded.header, "header bytes"_v);
  EXPECT_EQ(decoded.contextInfo, "context info bytes"_v);
  EXPECT_EQ(decoded.payoutInfo, "payout info bytes"_v);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}

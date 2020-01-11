#include "veriblock/entities/vbk_merkle_path.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

TEST(VbkMerklePath, Deserialize) {
  auto merklePath =
      "04000000010400000000201FEC8AA4983D69395010E4D18CD8B943749D5B4F575E88A375DEBDC5ED22531C0400000002200000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000000"_unhex;
  auto stream = ReadStream(merklePath);
  auto decoded = VbkMerklePath::fromRaw(stream);

  std::vector<Sha256Hash> expectedLayers{
      "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
      "0000000000000000000000000000000000000000000000000000000000000000"_unhex,
  };
  EXPECT_EQ(decoded.index, 0);
  EXPECT_EQ(decoded.treeIndex, 1);
  EXPECT_EQ(decoded.subject.toHex(),
            "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c");
  EXPECT_EQ(decoded.layers, expectedLayers);

  EXPECT_FALSE(stream.hasMore(1)) << "stream has more data";
}
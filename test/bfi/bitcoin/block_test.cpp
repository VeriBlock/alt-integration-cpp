// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/bfi/bitcoin/block.hpp"

using namespace altintegration;
using namespace altintegration::btc;

TEST(BlockHeader, serde_test) {
  BlockHeader header{
      2,
      uint256::fromHex(
          "b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000"),
      uint256::fromHex(
          "9d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab31471"),
      1415239972,
      404472624,
      1678286846};
  WriteStream writer;

  Serialize(writer, header);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f0864");

  BlockHeader decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(header, decoded);
}

TEST(Block, serde_test_witouth_pop_data) {
  TxIn in{
      OutPoint{uint256::fromHex("7b1eabe0209b1fe794124575ef807057c77ada2138"
                                "ae4fa8d6c4de0398a14f3f"),
               0},
      ParseHex("4830450221008949f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1"
               "a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268e082"
               "a725f8aeae0573fb12ff866a5f01"),
      0xffffffff,
      {ParseHex("ff4cff")},
  };
  TxOut out{(Amount)4999990000,
            ParseHex("76a914cbc20a7664f2f69e5355aa427045bc15e7c6c77288ac")};
  Transaction tx{{in}, {out}, 1, 0};

  Block block{
      2,
      uint256::fromHex(
          "b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000"),
      uint256::fromHex(
          "9d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab31471"),
      1415239972,
      404472624,
      1678286846};
  block.vtx = {tx};
  WriteStream writer;

  Serialize(writer, block);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f086401010000000001017b1eabe0209b1fe7941245"
            "75ef807057c77ada2138ae4fa8d6c4de0398a14f3f000000004948304502210089"
            "49f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1a7150deb388ab89350220"
            "79656090d7f6bac4c9a94e0aad311a4268e082a725f8aeae0573fb12ff866a5f01"
            "ffffffff01f0ca052a010000001976a914cbc20a7664f2f69e5355aa427045bc15"
            "e7c6c77288ac0103ff4cff00000000");

  Block decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(block, decoded);
}

TEST(Block, serde_test_with_pop_data) {
  std::string defaultPopDataEncoded =
      "00000001010141000013880002449c60619294546ad825af03b0935637860679ddd55ee4"
      "fd"
      "21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000000000101"
      "01"
      "0000000102046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013"
      "35"
      "0002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a1"
      "56"
      "28b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1f"
      "b6"
      "94a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402"
      "20"
      "0cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7"
      "ca"
      "24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf070"
      "9c"
      "395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00"
      "00"
      "0000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac00000000000000"
      "00"
      "536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09"
      "ef"
      "ac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1"
      "86"
      "4df04216615cf92083f40000000002019f040000067b040000000c040000000400000020"
      "20"
      "4d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1a"
      "be"
      "8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952"
      "af"
      "133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc19"
      "43"
      "af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b36806112"
      "9f"
      "30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6"
      "dc"
      "31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4e"
      "cc"
      "2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a"
      "74"
      "453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad"
      "17"
      "202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba"
      "22"
      "9acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbc"
      "c1"
      "a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c020000000000000"
      "00"
      "000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e99"
      "7d"
      "74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc001035000"
      "00"
      "80200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255e"
      "ef"
      "bc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c"
      "17"
      "e11874af500000402000000000000000000013535112250e115e2896e4f602c353d83944"
      "30"
      "80398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739"
      "e0"
      "8d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b1"
      "7d"
      "d3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b"
      "64"
      "3406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef0"
      "4e"
      "47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb"
      "00"
      "7a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b810400"
      "0a"
      "03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8"
      "a8"
      "a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df990400000000"
      "04"
      "0000000d202a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0de"
      "c6"
      "0400000006205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f596939"
      "97"
      "872020d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c0"
      "6f"
      "e913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350"
      "ee"
      "b8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e9020000000000000000000"
      "00"
      "000000000000000000000000000000000000000000002036252dfc621de420fb083ad9d8"
      "76"
      "7cba627eddeec64e421e9576cee21297dd0a41000013700002449c60619294546ad825af"
      "03"
      "b0935637860679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b"
      "92"
      "11070211cafff01c3f0101010000000101580101166772f51ab208d32771ab1506970eeb"
      "66"
      "4462730b838e0203e800010701370100010c6865616465722062797465730112636f6e74"
      "65"
      "787420696e666f20627974657301117061796f757420696e666f20627974657346304402"
      "20"
      "398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c"
      "00"
      "c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c758305630100607"
      "2a"
      "8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048"
      "f8"
      "fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c"
      "14"
      "c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d"
      "5b"
      "4f575e88a375debdc5ed22531c0400000002200000000000000000000000000000000000"
      "00"
      "000000000000000000000000000020000000000000000000000000000000000000000000"
      "00"
      "0000000000000000000041000013880002449c60619294546ad825af03b0935637860679"
      "dd"
      "d55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000"
      "00"
      "0001";

  PopData popData = AssertDeserializeFromHex<PopData>(defaultPopDataEncoded);

  TxIn in{
      OutPoint{uint256::fromHex("7b1eabe0209b1fe794124575ef807057c77ada2138"
                                "ae4fa8d6c4de0398a14f3f"),
               0},
      ParseHex("4830450221008949f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1"
               "a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268e082"
               "a725f8aeae0573fb12ff866a5f01"),
      0xffffffff,
      {ParseHex("ff4cff")},
  };
  TxOut out{(Amount)4999990000,
            ParseHex("76a914cbc20a7664f2f69e5355aa427045bc15e7c6c77288ac")};
  Transaction tx{{in}, {out}, 1, 0};

  Block block{
      2,
      uint256::fromHex(
          "b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000"),
      uint256::fromHex(
          "9d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab31471"),
      1415239972,
      404472624,
      1678286846};
  block.vtx = {tx};
  block.popData = popData;
  WriteStream writer;

  Serialize(writer, block);

  ASSERT_EQ(writer.hex(),
            "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f086401010000000001017b1eabe0209b1fe7941245"
            "75ef807057c77ada2138ae4fa8d6c4de0398a14f3f000000004948304502210089"
            "49f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1a7150deb388ab89350220"
            "79656090d7f6bac4c9a94e0aad311a4268e082a725f8aeae0573fb12ff866a5f01"
            "ffffffff01f0ca052a010000001976a914cbc20a7664f2f69e5355aa427045bc15"
            "e7c6c77288ac0103ff4cff00000000");

  Block decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_NE(block, decoded);

  // update block version
  block.setVersion(block.getVersion() | POP_BLOCK_VERSION_BIT);

  writer = WriteStream{};

  Serialize(writer, block);

  ASSERT_EQ(writer.hex(),
            "02000800b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000"
            "0000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3"
            "147124d95a5430c31b18fe9f086401010000000001017b1eabe0209b1fe7941245"
            "75ef807057c77ada2138ae4fa8d6c4de0398a14f3f000000004948304502210089"
            "49f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1a7150deb388ab89350220"
            "79656090d7f6bac4c9a94e0aad311a4268e082a725f8aeae0573fb12ff866a5f01"
            "ffffffff01f0ca052a010000001976a914cbc20a7664f2f69e5355aa427045bc15"
            "e7c6c77288ac0103ff4cff00000000fd3f08" +
                defaultPopDataEncoded);

  reader = ReadStream{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(block, decoded);
}

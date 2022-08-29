// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/bfi/bitcoin/transaction.hpp"

#include <gtest/gtest.h>

using namespace altintegration;
using namespace altintegration::btc;

TEST(Transaction, serde_test) {
  TxIn in{OutPoint{uint256::fromHex("7b1eabe0209b1fe794124575ef807057c77ada2138"
                                    "ae4fa8d6c4de0398a14f3f"),
                   0},
          ParseHex("4830450221008949f0cb400094ad2b5eb399d59d01c14d73d8fe6e96df1"
                   "a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268e082"
                   "a725f8aeae0573fb12ff866a5f01"),
          0xffffffff,
          {}};
  TxOut out{(Amount)4999990000,
            ParseHex("76a914cbc20a7664f2f69e5355aa427045bc15e7c6c77288ac")};
  Transaction tx{{in}, {out}, 1, 0};
  WriteStream writer;

  Serialize(writer, tx);

  ASSERT_EQ(writer.hex(),
            "01000000017b1eabe0209b1fe794124575ef807057c77ada2138ae4fa8d6c4de03"
            "98a14f3f00000000494830450221008949f0cb400094ad2b5eb399d59d01c14d73"
            "d8fe6e96df1a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268"
            "e082a725f8aeae0573fb12ff866a5f01ffffffff01f0ca052a010000001976a914"
            "cbc20a7664f2f69e5355aa427045bc15e7c6c77288ac00000000");

  Transaction decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(tx, decoded);
}

TEST(Transaction, serde_witness_test) {
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
  WriteStream writer;

  Serialize(writer, tx);

  ASSERT_EQ(
      writer.hex(),
      "010000000001017b1eabe0209b1fe794124575ef807057c77ada2138ae4fa8d6c4de0398"
      "a14f3f00000000494830450221008949f0cb400094ad2b5eb399d59d01c14d73d8fe6e96"
      "df1a7150deb388ab8935022079656090d7f6bac4c9a94e0aad311a4268e082a725f8aeae"
      "0573fb12ff866a5f01ffffffff01f0ca052a010000001976a914cbc20a7664f2f69e5355"
      "aa427045bc15e7c6c77288ac0103ff4cff00000000");

  Transaction decoded{};
  ReadStream reader{writer.data()};

  Unserialize(reader, decoded);

  ASSERT_EQ(tx, decoded);
}

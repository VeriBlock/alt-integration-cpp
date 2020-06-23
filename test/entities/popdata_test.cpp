// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/entities/popdata.hpp>

#include "util/test_utils.hpp"
#include "veriblock/literals.hpp"

using namespace altintegration;

TEST(AltPopTx, Deserialize) {
  const auto atvBytes = ParseHex(defaultAtvEncoded);
  auto stream = ReadStream(atvBytes);
  ATV atv = ATV::fromVbkEncoding(stream);

  const auto vtbBytes = ParseHex(defaultVtbEncoded);
  stream = ReadStream(vtbBytes);
  VTB vtb = VTB::fromVbkEncoding(stream);

  PopData expectedPopData = {2, {}, {atv}, {vtb}};
  std::vector<uint8_t> bytes = expectedPopData.toVbkEncoding();

  PopData encodedPopData = PopData::fromVbkEncoding(bytes);

  EXPECT_EQ(encodedPopData, expectedPopData);
}

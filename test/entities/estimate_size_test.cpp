// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>
#include <veriblock/entities/popdata.hpp>

using namespace altintegration;

TEST(Estimate, PublicationData) {
  PublicationData p = AssertDeserializeFromHex<PublicationData>("0100010001000100");
  WriteStream w;
  p.toVbkEncoding(w);
  ASSERT_EQ(w.data().size(), p.estimateSize());
}
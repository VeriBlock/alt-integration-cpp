// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <vector>
#include <veriblock/pop/read_stream.hpp>
#include <veriblock/pop/write_stream.hpp>

TEST(ReadStream, Read) {
  std::vector<uint8_t> buf{0, 1, 2, 3};
  altintegration::ReadStream stream(buf);
  altintegration::ValidationState state;
  uint8_t byte = 0;

  EXPECT_EQ(stream.assertRead(1), std::vector<uint8_t>{0});
  EXPECT_EQ(stream.assertRead(1), std::vector<uint8_t>{1});
  EXPECT_EQ(stream.assertRead(1), std::vector<uint8_t>{2});
  EXPECT_EQ(stream.assertRead(1), std::vector<uint8_t>{3});
  ASSERT_FALSE(stream.readBE<uint8_t>(byte, state));
  ASSERT_EQ(state.GetPath(), "readbe-underflow");
  state.reset();

  stream.reset();

  EXPECT_EQ(stream.assertRead(2), (std::vector<uint8_t>{0, 1}));
  EXPECT_EQ(stream.assertRead(2), (std::vector<uint8_t>{2, 3}));

  stream.reset();

  EXPECT_EQ(stream.assertRead(4), (std::vector<uint8_t>{0, 1, 2, 3}));

  stream.reset();

  std::vector<uint8_t> out(5, 0);
  ASSERT_FALSE(stream.read(5, out.data(), state));
  ASSERT_EQ(state.GetPath(), "read-underflow");
  state.reset();

  stream.reset();

  auto sl1 = stream.assertReadSlice(2);
  EXPECT_EQ(sl1.data(), buf.data());
  EXPECT_EQ(sl1.size(), 2);
  EXPECT_EQ(sl1[0], 0);
  EXPECT_EQ(sl1[1], 1);

  auto sl2 = stream.assertReadSlice(2);
  EXPECT_EQ(sl2.data(), buf.data() + 2);
  EXPECT_EQ(sl2.size(), 2);
  EXPECT_EQ(sl2[0], 2);
  EXPECT_EQ(sl2[1], 3);
}

TEST(ReadStream, BE) {
  std::vector<uint8_t> buf{0x06, 0xfb, 0x0a, 0xfd};
  altintegration::ReadStream stream(buf);

  EXPECT_EQ(stream.assertReadBE<int8_t>(), 0x06);
  EXPECT_EQ(stream.assertReadBE<uint8_t>(), 0xfb);
  stream.reset();
  EXPECT_EQ(stream.assertReadBE<int16_t>(), 1787);
  EXPECT_EQ(stream.assertReadBE<uint16_t>(), 2813);
  stream.reset();
  EXPECT_EQ(stream.assertReadBE<int32_t>(), 117115645);
  stream.reset();
  EXPECT_EQ(stream.assertReadBE<uint32_t>(), 117115645UL);
}

TEST(ReadStream, LE) {
  std::vector<uint8_t> buf{0x06, 0xfb, 0x0a, 0xfd};
  altintegration::ReadStream stream(buf);

  EXPECT_EQ(stream.assertReadLE<int8_t>(), 0x6);
  EXPECT_EQ(stream.assertReadLE<uint8_t>(), 0xfb);
  stream.reset();
  EXPECT_EQ(stream.assertReadLE<int16_t>(), -1274);
  EXPECT_EQ(stream.assertReadLE<uint16_t>(), 64778);
  stream.reset();
  EXPECT_EQ(stream.assertReadLE<int32_t>(), -49612026);
  stream.reset();
  EXPECT_EQ(stream.assertReadLE<uint32_t>(), 4245355270);
}

TEST(ReadStream, Negative) {
  std::vector<uint8_t> buf{0xff, 0xff, 0xff, 0xff};
  altintegration::ReadStream stream(buf);

  EXPECT_EQ(stream.assertReadBE<uint32_t>(), 4294967295ull);
  EXPECT_EQ(stream.remaining(), 0u);
  stream.reset();
  EXPECT_EQ(stream.assertReadBE<int32_t>(), -1);
  EXPECT_EQ(stream.remaining(), 0u);
}

TEST(WriteStream, Write) {
  altintegration::WriteStream stream;
  stream.write(std::vector<uint8_t>{1});
  stream.write(std::vector<uint8_t>{2, 3});
  stream.write(std::vector<uint8_t>{4, 5, 6});
  EXPECT_EQ(stream.data(), (std::vector<uint8_t>{1, 2, 3, 4, 5, 6}));
  stream.write(std::string{"a"});
  EXPECT_EQ(stream.data(), (std::vector<uint8_t>{1, 2, 3, 4, 5, 6, 'a'}));
  std::vector<uint8_t> v{9, 8, 7};
  stream.write(v);
  EXPECT_EQ(stream.data(),
            (std::vector<uint8_t>{1, 2, 3, 4, 5, 6, 'a', 9, 8, 7}));
}

TEST(WriteStream, LE) {
  altintegration::WriteStream stream;
  stream.writeLE<uint8_t>(1);
  stream.writeLE<int8_t>(-1);
  stream.writeLE<uint16_t>(2);
  stream.writeLE<int16_t>(-2);
  stream.writeLE<uint32_t>(3);
  stream.writeLE<int32_t>(-3);
  stream.writeLE<uint64_t>(4);
  stream.writeLE<int64_t>(-4);

  EXPECT_EQ(stream.data().size(),
            sizeof(int8_t) * 2 + sizeof(int16_t) * 2 + sizeof(int32_t) * 2 +
                sizeof(int64_t) * 2);
  EXPECT_EQ(stream.data(),
            (std::vector<uint8_t>{
                1,    0xff, 2,    0,    0xfe, 0xff, 3,    0,    0,    0,
                0xfd, 0xff, 0xff, 0xff, 4,    0,    0,    0,    0,    0,
                0,    0,    0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
}

TEST(WriteStream, BE) {
  altintegration::WriteStream stream;
  stream.writeBE<uint8_t>(1);
  stream.writeBE<int8_t>(-1);
  stream.writeBE<uint16_t>(2);
  stream.writeBE<int16_t>(-2);
  stream.writeBE<uint32_t>(3);
  stream.writeBE<int32_t>(-3);
  stream.writeBE<uint64_t>(4);
  stream.writeBE<int64_t>(-4);

  EXPECT_EQ(stream.data().size(),
            sizeof(int8_t) * 2 + sizeof(int16_t) * 2 + sizeof(int32_t) * 2 +
                sizeof(int64_t) * 2);
  EXPECT_EQ(stream.data(),
            (std::vector<uint8_t>{
                1,    0xff, 0,    2,    0xff, 0xfe, 0,    0,    0,    3,
                0xff, 0xff, 0xff, 0xfd, 0,    0,    0,    0,    0,    0,
                0,    4,    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc}));
}

TEST(Streams, BE_Sized) {
  altintegration::WriteStream stream;
  // nonce is 5 bytes
  uint64_t nonce = 0x0000001122334455;
  stream.writeBE<uint64_t>(nonce, 5);

  ASSERT_EQ(stream.data(),
            (std::vector<uint8_t>{0x11, 0x22, 0x33, 0x44, 0x55}));

  altintegration::ReadStream rs(stream.data());

  uint64_t actual = rs.assertReadBE<uint64_t>(5);
  ASSERT_EQ(nonce, actual) << std::hex << actual;
}

TEST(Streams, big_uint64t) {
  altintegration::WriteStream writer;
  writer.writeLE<uint64_t>(4607182418800017408LL);
  writer.writeLE<uint64_t>(4611686018427387904LL);

  altintegration::ReadStream reader{writer.data()};
  altintegration::ValidationState state;
  uint64_t out = 0;

  reader.readLE<uint64_t>(out, state);
  ASSERT_EQ(out, (uint64_t)4607182418800017408LL);
  reader.readLE<uint64_t>(out, state);
  ASSERT_EQ(out, (uint64_t)4611686018427387904LL);
}
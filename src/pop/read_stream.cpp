// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <veriblock/pop/read_stream.hpp>

namespace altintegration {

//============================================================================================
//                               READSTREAM
//============================================================================================
ReadStream::ReadStream(const void *buff, size_t numOfBytes) {
  assign(buff, numOfBytes);
};

void ReadStream::assign(const void *buff, size_t numOfBytes) {
  VBK_ASSERT(buff);

  m_Buffer = reinterpret_cast<unsigned char *>(const_cast<void *>(buff));
  m_Size = numOfBytes;
}

size_t ReadStream::position() const noexcept { return m_Pos; }

void ReadStream::setPosition(const size_t &pos) noexcept { m_Pos = pos; }

void ReadStream::reset() noexcept { m_Pos = 0; }

ReadStream::ReadStream(const std::vector<uint8_t> &v)
    : m_Buffer(const_cast<uint8_t *>(v.data())), m_Size(v.size()) {}

ReadStream::ReadStream(const std::string &s)
    : m_Buffer((uint8_t *)s.data()), m_Size(s.size()) {}

ReadStream::ReadStream(Slice<const uint8_t> slice)
    : m_Buffer(reinterpret_cast<const uint8_t *>(slice.data())),
      m_Size(slice.size()) {}

Slice<const uint8_t> ReadStream::data() const { return {m_Buffer, m_Size}; }

Slice<const uint8_t> ReadStream::remainingBytes() const {
  return {m_Buffer + m_Pos, m_Size - m_Pos};
}

bool ReadStream::read(size_t size, uint8_t *out, ValidationState &state) {
  if (size == 0) {
    return true;
  }

  VBK_ASSERT(out);

  if (!hasMore(size)) {
    return state.Invalid(
        "read-underflow",
        format("Tried to read {} bytes, while stream has {} bytes",
               size,
               remaining()));
  }

  std::copy(m_Buffer + m_Pos, m_Buffer + m_Pos + size, out);
  m_Pos += size;

  return true;
}

bool ReadStream::readSlice(size_t size,
                           Slice<const uint8_t> &out,
                           ValidationState &state) {
  if (!hasMore(size)) {
    return state.Invalid("readslice-underflow");
  }

  out = Slice<const uint8_t>(m_Buffer + m_Pos, size);
  m_Pos += size;
  return true;
}

std::vector<uint8_t> ReadStream::assertRead(size_t size) {
  if (size == 0) {
    return {};
  }

  std::vector<uint8_t> ret(size, 0);
  ValidationState state;
  bool result = read(size, ret.data(), state);
  VBK_ASSERT_MSG(result, state.toString());
  return ret;
}

Slice<const uint8_t> ReadStream::assertReadSlice(size_t size) {
  Slice<const uint8_t> ret;
  ValidationState state;
  bool result = readSlice(size, ret, state);
  VBK_ASSERT_MSG(result, state.toString());
  return ret;
}

}  // namespace altintegration

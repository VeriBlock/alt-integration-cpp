// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <veriblock/read_stream.hpp>

namespace altintegration {

//============================================================================================
//                               READSTREAM
//============================================================================================
ReadStream::ReadStream(const void *buff, size_t numOfBytes) {
  assign(buff, numOfBytes);
};

void ReadStream::assign(const void *buff, size_t numOfBytes) {
  if (!buff) {
    throw std::invalid_argument("stream: invalid input buffer");
  }

  m_Buffer = reinterpret_cast<unsigned char *>(const_cast<void *>(buff));
  m_Size = numOfBytes;
}

size_t ReadStream::position() const noexcept { return m_Pos; }

void ReadStream::setPosition(const size_t &pos) noexcept { m_Pos = pos; }

size_t ReadStream::remaining() const noexcept { return (m_Size - m_Pos); }

bool ReadStream::hasMore(size_t nbytes) const noexcept {
  return (remaining() >= nbytes);
}

void ReadStream::reset() noexcept { m_Pos = 0; }

ReadStream::ReadStream(const std::vector<uint8_t> &v)
    : m_Buffer(const_cast<uint8_t *>(v.data())), m_Size(v.size()) {}

ReadStream::ReadStream(const std::string &s)
    : m_Buffer((uint8_t *)s.data()), m_Size(s.size()) {}

Slice<const uint8_t> ReadStream::readSlice(size_t size) {
  if (!hasMore(size)) {
    throw std::out_of_range("stream.read(): out of data");
  }

  Slice<const uint8_t> data(m_Buffer + m_Pos, size);
  m_Pos += size;
  return data;
}

ReadStream::ReadStream(Slice<const uint8_t> slice)
    : m_Buffer(reinterpret_cast<const uint8_t *>(slice.data())),
      m_Size(slice.size()) {}

std::vector<uint8_t> ReadStream::read(size_t size) {
  return read<std::vector<uint8_t>>(size);
}

}  // namespace altintegration

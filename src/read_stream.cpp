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

bool ReadStream::read(size_t size,
                      std::vector<uint8_t> &out,
                      ValidationState &state) {
  return read<std::vector<uint8_t>>(size, out, state);
}

std::vector<uint8_t> ReadStream::read(size_t size) {
  return read<std::vector<uint8_t>>(size);
}

bool ReadStream::readSlice(size_t size,
                           Slice<const uint8_t> &out,
                           ValidationState &state) {
  if (!hasMore(size)) {
    return state.Invalid("readslice-buffer-underflow");
  }

  Slice<const uint8_t> data(m_Buffer + m_Pos, size);
  m_Pos += size;
  out = data;
  return true;
}

Slice<const uint8_t> ReadStream::readSlice(size_t size) {
  Slice<const uint8_t> out;
  ValidationState state;
  if (!readSlice(size, out, state)) {
    throw std::out_of_range("stream.read(): out of data");
  }
  return out;
}

ReadStream::ReadStream(Slice<const uint8_t> slice)
    : m_Buffer(reinterpret_cast<const uint8_t *>(slice.data())),
      m_Size(slice.size()) {}

Slice<const uint8_t> ReadStream::data() const {
  return Slice<const uint8_t>(m_Buffer, m_Size);
}

Slice<const uint8_t> ReadStream::remainingBytes() const {
  return Slice<const uint8_t>(m_Buffer + m_Pos, m_Size - m_Pos);
}

}  // namespace altintegration

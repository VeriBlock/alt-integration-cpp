// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/strutil.hpp>
#include <veriblock/pop/write_stream.hpp>

namespace altintegration {

WriteStream::WriteStream(size_t size) { m_data.reserve(size); }

void WriteStream::write(const void *buf, size_t size) {
  const uint8_t *inp = (uint8_t *)buf;
  m_data.insert(m_data.end(), inp, inp + size);
}

const WriteStream::storage_t &WriteStream::data() const noexcept {
  return m_data;
}

std::string WriteStream::hex() const noexcept {
  return HexStr(m_data.begin(), m_data.end());
}

}  // namespace altintegration
#include <veriblock/write_stream.hpp>

namespace VeriBlock {

const std::vector<uint8_t> &WriteStream::data() const noexcept {
  return m_data;
}

void WriteStream::write(const void *buf, size_t size) {
  const uint8_t *inp = (uint8_t *)buf;
  m_data.insert(m_data.end(), inp, inp + size);
}

WriteStream::WriteStream(size_t size) { m_data.reserve(size); }

}  // namespace VeriBlock

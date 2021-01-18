// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "bytestream.hpp"

size_t VbkByteStream::read(uint8_t* buf, size_t toRead) {
  size_t actualToRead = std::min(toRead, remaining());
  if (actualToRead == 0) {
    return 0;
  }
  auto* begin = &data[ptr];
  auto* end = begin + actualToRead;
  VBK_ASSERT(ptr + actualToRead <= data.size());
  VBK_ASSERT(buf);
  std::copy(begin, end, buf);
  ptr += actualToRead;
  return actualToRead;
}
size_t VbkByteStream::remaining() {
  auto size = data.size();
  if (size <= ptr) {
    return 0;
  }
  return size - ptr;
}

VbkByteStream::VbkByteStream(Slice<const uint8_t> slice)
    : data(slice.begin(), slice.end()) {}

VbkByteStream::VbkByteStream(std::vector<uint8_t> bytes)
    : data(std::move(bytes)) {}

size_t VBK_ByteStream_Read(VBK_ByteStream* stream,
                           uint8_t* buffer,
                           size_t toRead) {
  VBK_ASSERT(stream);
  VBK_ASSERT(buffer);
  return stream->read(buffer, toRead);
}

void VBK_ByteStream_Free(VBK_ByteStream* stream) {
  if (stream != nullptr) {
    delete stream;
    stream = nullptr;
  }
}

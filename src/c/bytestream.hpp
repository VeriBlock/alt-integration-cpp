// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BYTESTREAM_HPP
#define VERIBLOCK_POP_CPP_BYTESTREAM_HPP

#include <veriblock/c/utils.h>

#include <cstdint>
#include <vector>
#include <veriblock/assert.hpp>
#include <veriblock/slice.hpp>

using altintegration::Slice;

struct VbkByteStream {
  // byte stream MUST own memory buffer
  VbkByteStream(Slice<const uint8_t> slice);
  VbkByteStream(std::vector<uint8_t> bytes);

  //! @returns 0 if no bytes left, and positive if caller can read some bytes
  size_t remaining();

  //! @returns 0 if no bytes left, and positive - number of bytes actually read.
  //! @invariant returned value is always <= toRead
  size_t read(uint8_t* buf, size_t toRead);

 protected:
  std::vector<uint8_t> data;
  size_t ptr = 0;
};

// use this func in C++ to allocate new VbkByteStream, and return it to
// Go/whatever. It should NOT be called from Go.
VbkByteStream* VbkByteStream_New(std::vector<uint8_t> bytes);

size_t VbkByteStream_Read(VbkByteStream* stream,
                          uint8_t* buffer,
                          size_t toRead);

void VbkByteStream_Free(VbkByteStream* stream);

#endif  // VERIBLOCK_POP_CPP_BYTESTREAM_HPP

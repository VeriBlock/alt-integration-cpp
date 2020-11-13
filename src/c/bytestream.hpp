// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BYTESTREAM_HPP
#define VERIBLOCK_POP_CPP_BYTESTREAM_HPP

#include <veriblock/c/bytestream.h>

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

#endif  // VERIBLOCK_POP_CPP_BYTESTREAM_HPP

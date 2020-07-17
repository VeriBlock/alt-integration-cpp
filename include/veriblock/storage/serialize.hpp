// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_SERIALIZE_HPP
#define VERIBLOCK_POP_CPP_SERIALIZE_HPP

#include <veriblock/blob.hpp>
#include <veriblock/read_stream.hpp>
#include <veriblock/write_stream.hpp>

namespace altintegration {

template <size_t N>
inline void Serialize(WriteStream& stream, const Blob<N>& obj) {
  stream.write(obj);
}

template <size_t N>
inline void Deserialize(ReadStream& stream, Blob<N>& obj) {
  obj = stream.readSlice(N);
}

template <typename T>
void Serialize(WriteStream& stream, const T& obj) {
  obj.toVbkEncoding(stream);
}

template <typename T>
void Deserialize(ReadStream& stream, T& out) {
  out = T::fromVbkEncoding(stream);
}

template <typename T>
void Serialize(WriteStream& stream, const std::pair<char, T>& obj) {
  stream.writeBE<char>(obj.first);
  Serialize(stream, obj.second);
}

template <typename T>
void Deserialize(ReadStream& stream, std::pair<char, T>& out) {
  out.first = stream.readBE<char>();
  Deserialize(stream, out.second);
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_SERIALIZE_HPP

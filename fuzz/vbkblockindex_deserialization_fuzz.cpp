// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/popdata.hpp>

// libFuzzer
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  using namespace altintegration;

  ReadStream stream(Data, Size);
  ValidationState state;
  BlockIndex<VbkBlock> index;
  DeserializeFromVbkEncoding(stream, index, state);

  return 0;
}
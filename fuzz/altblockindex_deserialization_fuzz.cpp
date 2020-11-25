// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/popdata.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>

// libFuzzer
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  using namespace altintegration;

  ReadStream stream(Data, Size);
  ValidationState state;
  BlockIndex<AltBlock> index;
  DeserializeFromVbkEncoding(stream, index, state);

  return 0;
}
// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/popdata.hpp>

#define DEFINE_DESER_FUZZ(type)                       \
  {                                                   \
    type value{};                                     \
    DeserializeFromVbkEncoding(stream, value, state); \
    state.reset();                                    \
    stream.reset();                                   \
  }

// neither of these should fail with memory bug or assert
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  using namespace altintegration;

  ReadStream stream(Data, Size);
  ValidationState state;

  // sorted in ascending order of "compexity/size" of an entity
  DEFINE_DESER_FUZZ(Address);
  DEFINE_DESER_FUZZ(BtcTx);
  DEFINE_DESER_FUZZ(Output);
  DEFINE_DESER_FUZZ(PublicationData);
  DEFINE_DESER_FUZZ(Coin);
  DEFINE_DESER_FUZZ(VbkMerklePath);
  DEFINE_DESER_FUZZ(AltBlock);
  DEFINE_DESER_FUZZ(BtcBlock);
  DEFINE_DESER_FUZZ(VbkBlock);
  DEFINE_DESER_FUZZ(PopState<VbkEndorsement>);
  DEFINE_DESER_FUZZ(PopState<AltEndorsement>);
  DEFINE_DESER_FUZZ(VbkEndorsement);
  DEFINE_DESER_FUZZ(AltEndorsement);
  DEFINE_DESER_FUZZ(VbkPopTx);
  DEFINE_DESER_FUZZ(VbkTx);
  DEFINE_DESER_FUZZ(BtcBlockAddon);
  DEFINE_DESER_FUZZ(VbkBlockAddon);
  DEFINE_DESER_FUZZ(AltBlockAddon);
  DEFINE_DESER_FUZZ(BlockIndex<AltBlock>);
  DEFINE_DESER_FUZZ(BlockIndex<VbkBlock>);
  DEFINE_DESER_FUZZ(BlockIndex<BtcBlock>);
  DEFINE_DESER_FUZZ(VTB);
  DEFINE_DESER_FUZZ(ATV);
  DEFINE_DESER_FUZZ(PopData);

  return 0;
}
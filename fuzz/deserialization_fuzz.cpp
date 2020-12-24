// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <type_traits>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/keystone_container.hpp>
#include <veriblock/entities/context_info_container.hpp>

#define XSTR(s) STR(s)
#define STR(s) #s

template <typename T>
class hasEstimateSize {
  typedef char one;
  struct two {
    char x[2];
  };

  template <typename C>
  static one test(decltype(&C::estimateSize));
  template <typename C>
  static two test(...);

 public:
  enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

template <typename T>
typename std::enable_if<hasEstimateSize<T>::value, size_t>::type
getEstimateSize(T& t) {
  return t.estimateSize();
}

auto getEstimateSize(...) { return 0ul; }

#define DEFINE_DESER_FUZZ(type)                                                \
  {                                                                            \
    type value;                                                                \
    if (DeserializeFromVbkEncoding(stream, value, state)) {                    \
      WriteStream w;                                                           \
      value.toVbkEncoding(w);                                                  \
      type value2;                                                             \
      if (!DeserializeFromVbkEncoding(w.data(), value2, state)) {              \
        /* we serialized an entity but were not able to deserialize it back */ \
        VBK_ASSERT_MSG(false,                                                  \
                       "type=%s\nbytes=%s\nstate=%s",                          \
                       STR(type),                                              \
                       HexStr(w.data()),                                       \
                       state.toString());                                      \
      }                                                                        \
      /* if given type provides .estimateSize() method */                      \
      /* compare its output to size of toVbkEncoding output */                 \
      if constexpr (hasEstimateSize<type>::value) {                            \
        auto expectedSize = w.data().size();                                   \
        auto estimatedSize1 = getEstimateSize(value);                          \
        auto estimatedSize2 = getEstimateSize(value2);                         \
        VBK_ASSERT_MSG(                                                        \
            expectedSize == estimatedSize1 &&                                  \
                estimatedSize1 == estimatedSize2,                              \
            "type=%s\nbytes=%s\nexpected=%d\nestimated1=%d\nestimated2=%d\n",  \
            STR(type),                                                         \
            HexStr(w.data()),                                                  \
            expectedSize,                                                      \
            estimatedSize1,                                                    \
            estimatedSize2);                                                   \
      }                                                                        \
    }                                                                          \
    state.reset();                                                             \
    stream.reset();                                                            \
  }

// neither of these should fail with memory bug or assert
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
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
  DEFINE_DESER_FUZZ(KeystoneContainer);
  DEFINE_DESER_FUZZ(ContextInfoContainer);
  DEFINE_DESER_FUZZ(AuthenticatedContextInfoContainer);
  DEFINE_DESER_FUZZ(AltBlock);
  DEFINE_DESER_FUZZ(BtcBlock);
  DEFINE_DESER_FUZZ(VbkBlock);
  DEFINE_DESER_FUZZ(VbkEndorsement);
  DEFINE_DESER_FUZZ(AltEndorsement);
  DEFINE_DESER_FUZZ(VbkPopTx);
  DEFINE_DESER_FUZZ(VbkTx);
  DEFINE_DESER_FUZZ(PopState<VbkEndorsement>);
  DEFINE_DESER_FUZZ(PopState<AltEndorsement>);
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
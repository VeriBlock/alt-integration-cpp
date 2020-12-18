// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/blockchain/pop/pop_state.hpp>

#include "FuzzedDataProvider.hpp"

using namespace altintegration;

const int ID_MIN_SIZE = 2;
const int ID_MAX_SIZE = 4;
const int MIN_ENDORSEMENTS_PER_BLOCK = 0;
const int MAX_ENDORSEMENTS_PER_BLOCK = 1024;

struct DummyEndorsement {
  using id_t = std::vector<uint8_t>;
  id_t id;

  id_t getId() const { return id; }

  void setDirty() const {
    // ignore
  }

  void toVbkEncoding(WriteStream& stream) {
    VBK_ASSERT(id.size() >= ID_MIN_SIZE);
    VBK_ASSERT(id.size() <= ID_MAX_SIZE);
    writeSingleByteLenValue(stream, id);
  }
};

template <>
inline void PopState<DummyEndorsement>::setDirty() { /* ignore */
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                DummyEndorsement& out,
                                ValidationState& state) {
  return readSingleByteLenValue(
      stream, out.id, state, ID_MIN_SIZE, ID_MAX_SIZE);
}

bool ConsumeEndorsement(FuzzedDataProvider& p,
                        std::shared_ptr<DummyEndorsement>& e) {
  if (p.remaining_bytes() < ID_MIN_SIZE) {
    return false;
  }

  e->id = p.ConsumeBytes<uint8_t>(ID_MAX_SIZE);
  return true;
}

bool ConsumePopState(FuzzedDataProvider& p, PopState<DummyEndorsement>& out) {
  auto num = p.ConsumeIntegralInRange(MIN_ENDORSEMENTS_PER_BLOCK,
                                      MAX_ENDORSEMENTS_PER_BLOCK);
  auto n = 0;
  while (n < num) {
    auto e = std::make_shared<DummyEndorsement>();
    if (!ConsumeEndorsement(p, e)) {
      return false;
    }

    out.insertContainingEndorsement(std::move(e));
  }

  return true;
}

// this fuzzer answers the question:
// Is it possible to build a `valid` PopState such that after serialization it
// wont deserialize?
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  FuzzedDataProvider provider(Data, Size);

  PopState<DummyEndorsement> pop;
  if (!ConsumePopState(provider, pop)) {
    // not enough bytes in the stream
    return 0;
  }

  // part 1: serialize
  WriteStream w;
  pop.toVbkEncoding(w);

  // part 2: deserialize. should NOT fail on assert.
  AssertDeserializeFromVbkEncoding<PopState<DummyEndorsement>>(w.data());
  return 0;
}
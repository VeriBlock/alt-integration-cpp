// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/comparator.hpp>

namespace altintegration {

bool operator==(const BtcBlockAddon& a, const BtcBlockAddon& b) {
  // comparing reference counts does not seem like a good idea
  // as the only situation where they would be different is
  // comparing blocks across different trees eg mock miner vs
  // the test tree and in this situation the references and counts
  // are likely to differ
  EXPECT_EQ(a.getRefs(), b.getRefs());
  EXPECT_EQ(a.chainWork, b.chainWork);
  return true;
}
bool operator==(const VbkBlockAddon& a, const VbkBlockAddon& b) {
  EXPECT_EQ(a.chainWork, b.chainWork);
  EXPECT_EQ(a.refCount(), b.refCount());
  EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>());

  using base = PopState<VbkEndorsement>;
  const base& A = a;
  const base& B = b;
  EXPECT_EQ(A, B);
  return true;
}
bool operator==(const AltBlockAddon& a, const AltBlockAddon& b) {
  EXPECT_EQ(a.getPayloadIds<VbkBlock>(), b.getPayloadIds<VbkBlock>());
  EXPECT_EQ(a.getPayloadIds<VTB>(), b.getPayloadIds<VTB>());
  EXPECT_EQ(a.getPayloadIds<ATV>(), b.getPayloadIds<ATV>());

  using base = PopState<AltEndorsement>;
  const base& A = a;
  const base& B = b;
  EXPECT_EQ(A, B);
  return true;
}

bool operator==(const VbkBlockTree& a, const VbkBlockTree& b) {
  EXPECT_EQ(a.getComparator(), b.getComparator());
  using base = VbkBlockTree::base;
  const base& A = a;
  const base& B = b;
  EXPECT_EQ(A, B);
  return true;
}

bool operator==(const AltTree& a, const AltTree& b) {
  EXPECT_EQ(a.getComparator(), b.getComparator());
  using base = AltTree::base;
  const base& A = a;
  const base& B = b;
  EXPECT_EQ(A, B);
  EXPECT_EQ(a.getStorage(), b.getStorage());
  return true;
}

bool operator==(const PayloadsStorage& a, const PayloadsStorage& b) {
  EXPECT_EQ(a.getRepo(), b.getRepo());
  EXPECT_EQ(a.getPayloadsInAlt(), b.getPayloadsInAlt());
  EXPECT_EQ(a.getPayloadsInVbk(), b.getPayloadsInVbk());
  EXPECT_EQ(a.getValidity(), b.getValidity());
  return true;
}

static void readRepositoryInto(
    std::map<std::vector<uint8_t>, std::vector<uint8_t>>& m,
    const Repository& r) {
  auto cursor = r.newCursor();
  cursor->seekToFirst();
  while (cursor->isValid()) {
    m.insert({cursor->key(), cursor->value()});
    cursor->next();
  }
};

bool operator==(const Repository& a, const Repository& b) {
  std::map<std::vector<uint8_t>, std::vector<uint8_t>> mA;
  std::map<std::vector<uint8_t>, std::vector<uint8_t>> mB;

  readRepositoryInto(mA, a);
  readRepositoryInto(mB, b);

  EXPECT_EQ(mA, mB);
  return true;
}

}  // namespace altintegration
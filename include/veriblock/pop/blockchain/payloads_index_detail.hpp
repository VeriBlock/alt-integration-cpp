#ifndef E5300CA9_8C69_4C9C_A7D6_EFCACBB2534D
#define E5300CA9_8C69_4C9C_A7D6_EFCACBB2534D

#include <veriblock/pop/entities/popdata.hpp>

namespace altintegration {
namespace detail {

template <typename PL>
inline void PLIAddBlock(PL& pl, const BlockIndex<AltBlock>& index) {
  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<ATV>()) {
    pl.add(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.add(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VbkBlock>()) {
    pl.add(id.asVector(), hash);
  }
}

template <typename PL>
inline void PLIAddBlock(PL& pl, const BlockIndex<VbkBlock>& index) {
  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.add(id.asVector(), hash);
  }
}

template <typename PL>
inline void PLIAddBlock(PL&, const BlockIndex<BtcBlock>&) {
  // do nothing
}

template <typename PL>
inline void PLIRemoveBlock(PL& pl, const BlockIndex<AltBlock>& index) {
  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<ATV>()) {
    pl.remove(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.remove(id.asVector(), hash);
  }
  for (const auto& id : index.getPayloadIds<VbkBlock>()) {
    pl.remove(id.asVector(), hash);
  }
}

template <typename PL>
inline void PLIRemoveBlock(PL& pl, const BlockIndex<VbkBlock>& index) {
  const auto& hash = index.getHash();
  for (const auto& id : index.getPayloadIds<VTB>()) {
    pl.remove(id.asVector(), hash);
  }
}

template <typename PL>
inline void PLIRemoveBlock(PL&, const BlockIndex<BtcBlock>&) {
  // do nothing
}

}  // namespace detail
}  // namespace altintegration

#endif /* E5300CA9_8C69_4C9C_A7D6_EFCACBB2534D */

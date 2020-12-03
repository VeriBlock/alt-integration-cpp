// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/entities/keystone_container.hpp>
#include <veriblock/keystone_util.hpp>

namespace altintegration {

KeystoneContainer KeystoneContainer::fromPrevious(
    const BlockIndex<AltBlock>* prev, const uint32_t ki) {
  KeystoneContainer c;

  const auto height = prev == nullptr ? 0 : prev->getHeight() + 1;
  const auto first = getFirstPreviousKeystoneHeight(height, ki);
  const auto second = getSecondPreviousKeystoneHeight(height, ki);

  if (prev != nullptr) {
    auto* firstPreviousKeystone = prev->getAncestor(first);
    if (firstPreviousKeystone != nullptr) {
      c.firstPreviousKeystone = firstPreviousKeystone->getHash();

      // write second previous keystone
      auto secondPreviousKeystone = firstPreviousKeystone->getAncestor(second);
      if (secondPreviousKeystone != nullptr) {
        c.secondPreviousKeystone = secondPreviousKeystone->getHash();
      }
    }
  }

  return c;
}

void KeystoneContainer::write(WriteStream& stream) const {
  stream.write(firstPreviousKeystone);
  stream.write(secondPreviousKeystone);
}
}  // namespace altintegration
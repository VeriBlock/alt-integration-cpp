// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/pop/entities/keystone_container.hpp>
#include <veriblock/pop/keystone_util.hpp>

namespace altintegration {

KeystoneContainer KeystoneContainer::createFromPrevious(
    const BlockIndex<AltBlock>* prev, const uint32_t keystoneInterval) {
  KeystoneContainer c;

  const auto height = prev == nullptr ? 0 : prev->getHeight() + 1;
  const auto first = getPreviousKeystoneHeight(height, keystoneInterval, 0);
  const auto second = getPreviousKeystoneHeight(height, keystoneInterval, 1);

  if (prev != nullptr) {
    const auto* firstPreviousKeystone = prev->getAncestor(first);
    if (firstPreviousKeystone != nullptr) {
      c.firstPreviousKeystone = firstPreviousKeystone->getHash();

      // write second previous keystone
      auto* secondPreviousKeystone = firstPreviousKeystone->getAncestor(second);
      if (secondPreviousKeystone != nullptr) {
        c.secondPreviousKeystone = secondPreviousKeystone->getHash();
      }
    }
  }

  return c;
}

void KeystoneContainer::toVbkEncoding(WriteStream& stream) const {
  writeSingleByteLenValue(stream, firstPreviousKeystone);
  writeSingleByteLenValue(stream, secondPreviousKeystone);
}
size_t KeystoneContainer::estimateSize() const {
  return singleByteLenValueSize(firstPreviousKeystone) +
         singleByteLenValueSize(secondPreviousKeystone);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                KeystoneContainer& container,
                                ValidationState& state) {
  if (!readSingleByteLenValue(stream,
                              container.firstPreviousKeystone,
                              state,
                              MIN_ALT_HASH_SIZE,
                              MAX_ALT_HASH_SIZE)) {
    return state.Invalid("bad-keystone1");
  }

  if (!readSingleByteLenValue(stream,
                              container.secondPreviousKeystone,
                              state,
                              MIN_ALT_HASH_SIZE,
                              MAX_ALT_HASH_SIZE)) {
    return state.Invalid("bad-keystone2");
  }

  return true;
}

}  // namespace altintegration
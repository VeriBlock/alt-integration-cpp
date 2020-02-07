#ifndef ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_
#define ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_

#include <gmock/gmock.h>

#include <veriblock/storage/cursor.hpp>

namespace VeriBlock {

template <typename K, typename V>
struct CursorMock : public Cursor<K, V> {
  ~CursorMock() override = default;

  MOCK_METHOD0_T(seekToFirst, void());
  MOCK_METHOD0_T(seekToLast, void());
  MOCK_METHOD1_T(seek, void(const K&));
  MOCK_METHOD0_T(next, void());
  MOCK_METHOD0_T(prev, void());
  MOCK_CONST_METHOD0_T(isValid, bool());
  MOCK_CONST_METHOD0_T(key, K());
  MOCK_CONST_METHOD0_T(value, V());
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_

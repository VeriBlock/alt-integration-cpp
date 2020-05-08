// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct Interface {
  virtual ~Interface() = default;
  virtual void foo() = 0;
};

struct Mock : public Interface {
  ~Mock() override = default;
  MOCK_METHOD0(foo, void());
};

TEST(Gmock, Works) {
  Mock mock;
  EXPECT_CALL(mock, foo()).Times(1);
  mock.foo();
}

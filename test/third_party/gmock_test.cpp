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
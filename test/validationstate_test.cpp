#include <gtest/gtest.h>

#include <string>

#include <veriblock/validation_state.hpp>

TEST(ValidationState, BasicFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "desc", "Description");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test+desc");
  EXPECT_EQ(state.GetPathParts().size(), 2);
}

TEST(ValidationState, MultipleFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "desc", "Description");
  ret = state.Invalid("test2", "desc2", "Description2");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description2");
  EXPECT_EQ(state.GetPath(), "test2+desc2+test+desc");
  EXPECT_EQ(state.GetPathParts().size(), 4);
}

TEST(ValidationState, IndexFormatting) {
  altintegration::ValidationState state{};
  state.Invalid("desc", "Description").setIndex(1).setStackFunction("test");
  ASSERT_FALSE(state.IsValid());
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test+1+desc");
  EXPECT_EQ(state.GetPathParts().size(), 3);
}
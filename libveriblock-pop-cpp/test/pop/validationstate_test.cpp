// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <string>

#include <veriblock/pop/validation_state.hpp>

TEST(ValidationState, BasicFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "Description");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test");
  EXPECT_EQ(state.GetPathParts().size(), 1);
}

TEST(ValidationState, MultipleFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "Description");
  ret = state.Invalid("test2", "Description2");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description2");
  EXPECT_EQ(state.GetPath(), "test2+test");
  EXPECT_EQ(state.GetPathParts().size(), 2);
}

TEST(ValidationState, IndexFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "Description", 1);
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test+1");
  EXPECT_EQ(state.GetPathParts().size(), 2);
}

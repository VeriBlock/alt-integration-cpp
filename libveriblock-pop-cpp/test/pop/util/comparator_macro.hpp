// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_COMPARATOR_MACRO_HPP
#define VERIBLOCK_POP_CPP_COMPARATOR_MACRO_HPP

#define VBK_EXPECT_EQ(val1, val2, suppress) \
  if (!suppress) {                          \
    EXPECT_EQ(val1, val2);                  \
  }                                         \
  if (val1 != val2) {                       \
    return false;                           \
  }

#define VBK_EXPECT_NE(val1, val2, suppress) \
  if (!suppress) {                          \
    EXPECT_NE(val1, val2);                  \
  }                                         \
  if (val1 == val2) {                       \
    return false;                           \
  }

#define VBK_EXPECT_TRUE(val, suppress) \
  if (!suppress) {                     \
    EXPECT_TRUE(val);                  \
  }                                    \
  if (!val) {                          \
    return false;                      \
  }

#define VBK_EXPECT_FALSE(val, suppress) \
  if (!suppress) {                      \
    EXPECT_FALSE(val);                  \
  }                                     \
  if (val) {                            \
    return false;                       \
  }

#endif
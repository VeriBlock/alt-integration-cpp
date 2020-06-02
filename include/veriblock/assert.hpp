// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ASSERT_HPP
#define VERIBLOCK_POP_CPP_ASSERT_HPP

#ifdef VBK_HAVE_BUILTIN_EXPECT
// tell branch predictor that condition is always true
#define VBK_LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#else
#define VBK_LIKELY(condition) (condition)
#endif

#ifdef NDEBUG

#include <veriblock/fmt.hpp>
#define VBK_ASSERT(x)                                         \
  if (VBK_LIKELY((x))) {                                        \
    (void)(x);                                                \
  } else {                                                    \
    fmt::print(stderr,                                        \
               "Assertion failed at {}:{} inside {}: \n{}\n", \
               __FILE__,                                      \
               __LINE__,                                      \
               __FUNCTION__,                                  \
               #x);                                           \
    std::abort();                                             \
  }

#else
#include <cassert>
#define VBK_ASSERT(x) assert(x);
#endif

#endif  // VERIBLOCK_POP_CPP_ASSERT_HPP

// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ASSERT_HPP
#define VERIBLOCK_POP_CPP_ASSERT_HPP

#include <veriblock/logger.hpp>

#ifdef VBK_HAVE_BUILTIN_EXPECT
// tell branch predictor that condition is always true
#define VBK_LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define VBK_UNLIKELY(condition) \
  __builtin_expect(static_cast<bool>(condition), 0)
#else
#define VBK_LIKELY(condition) (condition)
#define VBK_UNLIKELY(condition) (condition)
#endif

#define VBK_ASSERT_MSG(x, ...)                                               \
  if (!VBK_LIKELY((x))) {                                                    \
    auto msg = fmt::format("Assertion failed at {}:{} inside {}:\n{}\n{}\n", \
                           __FILE__,                                         \
                           __LINE__,                                         \
                           __FUNCTION__,                                     \
                           #x,                                               \
                           fmt::sprintf(__VA_ARGS__));                       \
    /* print to log */ VBK_LOG_CRITICAL(msg);                                \
    /* print to stderr */ fmt::fprintf(stderr, msg);                         \
    /* die */ std::terminate();                                              \
  }

#define VBK_ASSERT(x) VBK_ASSERT_MSG(x, " ");

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define VBK_CHECK_RETURN __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define VBK_CHECK_RETURN _Check_return_
#else
#define VBK_CHECK_RETURN
#endif

#endif  // VERIBLOCK_POP_CPP_ASSERT_HPP

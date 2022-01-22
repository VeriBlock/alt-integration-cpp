// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ASSERT_HPP
#define VERIBLOCK_POP_CPP_ASSERT_HPP

#include "logger.hpp"

#ifdef VBK_HAS_BUILTIN_EXPECT
// tell branch predictor that condition is always true
#define VBK_LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define VBK_UNLIKELY(condition) \
  __builtin_expect(static_cast<bool>(condition), 0)
#else
#define VBK_LIKELY(condition) (condition)
#define VBK_UNLIKELY(condition) (condition)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define VBK_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define VBK_DEPRECATED __declspec(deprecated)
#else
#define VBK_DEPRECATED
#endif

#if defined(__GNUC__) || defined(__clang__)
#define VBK_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define VBK_DEPRECATED_MSG(msg) __declspec(deprecated)
#else
#define VBK_DEPRECATED_MSG
#endif

#define VBK_ASSERT_MSG(x, ...)                                               \
  /* GCOVR_EXCL_START */                                                     \
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
  } /* GCOVR_EXCL_STOP */

#define VBK_ASSERT(x) VBK_ASSERT_MSG(x, " ");

// same as VBK_ASSERT but executes only when compiled in DEBUG mode. similar to
// how <assert.h> works
#if defined(NDEBUG)
#define VBK_ASSERT_MSG_DEBUG(x, ...)
#define VBK_ASSERT_DEBUG(x)
#else
#define VBK_ASSERT_MSG_DEBUG(x, ...) VBK_ASSERT_MSG(x, __VA_ARGS__)
#define VBK_ASSERT_DEBUG(x) VBK_ASSERT(x)
#endif

// Similar to C++17 [[nodiscard]]
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define VBK_CHECK_RETURN __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define VBK_CHECK_RETURN _Check_return_
#else
#define VBK_CHECK_RETURN
#endif

#endif  // VERIBLOCK_POP_CPP_ASSERT_HPP

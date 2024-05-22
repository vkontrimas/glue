#pragma once

#ifdef __GNUC__
#define glue_detail_unreachable() (__builtin_unreachable())
#else
#define glue_detail_unreachable() ((void)0)
#endif

#ifdef __clang__
#define glue_detail_assume(predicate) (__builtin_assume(predicate))
#elif defined(_MSC_VER)
#define glue_detail_assume(predicate) (__assume(predicate))
#else
#define glue_detail_assume(predicate) ((void)0)
#endif

#if (defined(NDEBUG) && !defined(GLUE_RELEASE_ASSERT))
#define glue_assert(predicate) ((void)0)
#else
#include <glog/logging.h>

#include <cstdlib>

namespace glue::detail {
inline void assert_failed(const char* message) {
  LOG(FATAL) << message;
  std::abort();
}
}  // namespace glue::detail
#define glue_assert(predicate)                                           \
  do {                                                                   \
    if (!(predicate)) {                                                  \
      glue::detail::assert_failed("Assertion '" #predicate "' failed."); \
    }                                                                    \
  } while (false)
#endif

#include <glog/logging.h>

#include <cstdlib>

#define glue_check(predicate)                         \
  do {                                                \
    if (!(predicate)) {                               \
      LOG(FATAL) << "Check '" #predicate "' failed."; \
      std::abort();                                   \
    }                                                 \
  } while (false)

#define glue_assume(predicate)     \
  do {                             \
    glue_assert(predicate);        \
    glue_detail_assume(predicate); \
    if (!(predicate)) {            \
      glue_detail_unreachable();   \
    }                              \
  } while (false)

namespace glue {}
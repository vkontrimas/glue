#include "jolt_setup_globals.hpp"

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <glog/logging.h>

#include <cstdarg>

namespace glue {
namespace {
static void trace_impl(const char* fmt, ...) {
  // format
  // (ripped from Jolt/HelloWorld.cpp)
  va_list list;
  va_start(list, fmt);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), fmt, list);
  va_end(list);

  // log
  LOG(INFO) << "JOLT: " << buffer;
}

#ifdef JPH_ENABLE_ASSERTS
static bool assert_failed_impl(const char* expression, const char* message,
                               const char* file, uint line) {
  LOG(FATAL) << "JOLT: " << file << ":" << line << ": (" << expression << ") "
             << (message != nullptr ? message : "");
  return true;
};
#endif  // JPH_ENABLE_ASSERTS

}  // namespace

void setup_jolt_allocator() { JPH::RegisterDefaultAllocator(); }

void setup_jolt_logging() {
  JPH::Trace = trace_impl;
#ifdef JPH_ENABLE_ASSERTS
  JPH::AssertFailed = assert_failed_impl;
#endif
}
}  // namespace glue
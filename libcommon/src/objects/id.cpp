#include <zlib.h>

#include <cstdlib>
#include <glue/objects/id.hpp>

namespace glue::objects {
namespace {
u32 hash_name_weak_crypto(const char* name) {
  // we only need a uniqueness sum with less collisions
  // doesn't have to be cryptographically strong
  const auto crc = crc32(0, nullptr, 0);
  const auto len = std::strlen(name);
  return crc32(crc, reinterpret_cast<const u8*>(name), len);
}
}  // namespace
ObjectID::ObjectID(const char* name) : id_{hash_name_weak_crypto(name)} {}
ObjectID::ObjectID(const std::string& name) : ObjectID{name.c_str()} {}
}  // namespace glue
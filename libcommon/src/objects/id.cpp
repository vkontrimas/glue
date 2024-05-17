#include <glog/logging.h>
#include <zlib.h>

#include <cstdlib>
#include <glue/objects/id.hpp>
#include <unordered_map>

// TODO(vkon): store all these things at build time instead of runtime
namespace glue::objects {
namespace {
std::unordered_map<u32, const char*> gNameTable;
u64 gRandomCounter = 0;
const char* gRandomSalt = "CFSxSLuP";
const char* gRandomName = "unnamed";

u32 hash_name_weak_crypto(const char* name) {
  // we only need a uniqueness sum with less collisions
  // doesn't have to be cryptographically strong
  const auto crc = crc32(0, nullptr, 0);
  const auto len = std::strlen(name);
  return crc32(crc, reinterpret_cast<const u8*>(name), len);
}

u32 hash_name_randomized_weak_crypto() {
  auto crc = crc32(0, nullptr, 0);
  crc = crc32(crc, reinterpret_cast<const u8*>(gRandomName), 7);
  crc = crc32(crc, reinterpret_cast<const u8*>(gRandomSalt), 8);
  crc = crc32(crc, reinterpret_cast<const u8*>(&gRandomCounter),
              sizeof(gRandomCounter));
  ++gRandomCounter;
  return crc;
}
}  // namespace
ObjectID::ObjectID(const char* name) : id_{hash_name_weak_crypto(name)} {
  auto pair = gNameTable.emplace(id_, name);
  if (!pair.second) {
    LOG(ERROR) << "ObjectID collision. Existing ID = " << pair.first->second
               << " New ID = " << name;
  }
}
ObjectID::ObjectID(const std::string& name) : ObjectID{name.c_str()} {}

ObjectID ObjectID::random() {
  const auto hash = hash_name_randomized_weak_crypto();
  auto pair = gNameTable.emplace(hash, gRandomName);
  if (!pair.second) {
    LOG(ERROR) << "ObjectID collision. Existing ID = " << pair.first->second
               << " New ID = " << gRandomName;
  }
  return ObjectID{hash};
}

const char* ObjectID::retrieve_name() const {
  auto it = gNameTable.find(id_);
  if (it == std::end(gNameTable)) {
    return "unknown";
  } else {
    return it->second;
  }
}
}  // namespace glue::objects
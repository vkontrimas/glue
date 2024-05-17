#pragma once

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>

#include <memory>

namespace glue {
// Just handling the JPH::Factory class via RAII
// Extra work to kill singleton
struct JPHFactorySingletonInstance {
  JPHFactorySingletonInstance() : factory_{new JPH::Factory} {
    JPH::Factory::sInstance = factory_.get();
    JPH::RegisterTypes();
  }

  struct Deleter {
    void operator()(JPH::Factory* factory) {
      JPH::Factory::sInstance = nullptr;
      delete factory;
    }
  };

  std::unique_ptr<JPH::Factory, Deleter> factory_;
};

}  // namespace glue
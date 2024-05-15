#pragma once

#include <type_traits>
#include <utility>

namespace glue {
namespace detail {
// We use empty base class optimization to compress the deleter
// when it is empty. Hence the ResourceDeleterHolder base class.
template <typename Deleter, bool = std::is_empty<Deleter>::value>
class ResourceDeleterHolder;

// Case where deleter is stateful (not empty class).
// We hold on to it and return a reference from get_deleter.
template <typename Deleter>
class ResourceDeleterHolder<Deleter, false> {
 public:
  ResourceDeleterHolder() : deleter_{} {}
  explicit ResourceDeleterHolder(Deleter&& deleter)
      : deleter_{std::forward<Deleter>(deleter)} {}

 protected:
  Deleter& get_deleter() { return deleter_; }

 private:
  Deleter deleter_;
};

// Case where the Deleter is stateless (empty class)
// We construct a deleter instance on demand from get_deleter
template <typename Deleter>
class ResourceDeleterHolder<Deleter, true> {
 protected:
  Deleter get_deleter() { return Deleter{}; }
};
}  // namespace detail

template <typename T, typename Deleter>
class Resource : private detail::ResourceDeleterHolder<Deleter> {
 public:
  Resource() : detail::ResourceDeleterHolder<Deleter>{}, data_{} {}

  template <typename cT>
  Resource(cT&& data)
      : detail::ResourceDeleterHolder<Deleter>{},
        data_{std::forward<cT>(data)} {}

  template <typename cT, typename cDeleter>
  Resource(cT&& data, cDeleter&& deleter)
      : detail::ResourceDeleterHolder<Deleter>{std::forward<cDeleter>(deleter)},
        data_{std::forward<cT>(data)} {}

  ~Resource() { this->get_deleter()(data_); }

  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;

  Resource(Resource&& other) : Resource{} { swap(*this, other); }
  Resource& operator=(Resource&& other) {
    swap(*this, other);
    return *this;
  }

  friend void swap(Resource& a, Resource& b) {
    using std::swap;
    swap(a.data_, b.data_);
  }

  T& operator*() { return data_; }
  const T& operator*() const { return data_; }

  T* operator->() { return &data_; }
  const T* operator->() const { return &data_; }

 private:
  T data_;
};

namespace detail::test {
struct TestIntDeleter {
  void operator()(int test) {}
};

struct TestStatefulIntDeleter {
  int state;

  void operator()(int test) {}
};

static_assert(sizeof(Resource<int, TestIntDeleter>) == sizeof(int));
static_assert(sizeof(Resource<int, TestStatefulIntDeleter>) != sizeof(int));
};  // namespace detail::test
}  // namespace glue
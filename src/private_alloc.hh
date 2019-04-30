#ifndef PRIVATE_ALLOCATOR_HH_
#define PRIVATE_ALLOCATOR_HH_

#include <string.h>
#include <sys/mman.h>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "log.h"
#include "util.hh"

// Define them as global and back them with a shared mapping so that multiple
// processes can access the same head_ and used_
static void* head_;
static size_t used_;

template <class T>
class PrivateAllocator {
 public:
  // type definitions
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;

  // rebind PrivateAllocator to type U
  template <class U>
  struct rebind {
    typedef PrivateAllocator<U> other;
  };

  // return address of values
  pointer address(reference value) const { return &value; }
  const_pointer address(const_reference value) const { return &value; }

  // constructors and destructors
  // nothing to do because the PrivateAllocator has no state
  PrivateAllocator() { PageAllocate(); }

  PrivateAllocator(const PrivateAllocator&) {}
  template <class U>
  PrivateAllocator(const PrivateAllocator<U>&) {}
  ~PrivateAllocator() {}

  // return maximum number of elements that can be allocated
  size_type max_size() const {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  // pvAllocate but don't initialize num elements of type T by using our own
  // memory manager
  pointer allocate(size_type num) {
    size_t total_size = ROUND_UP(num * sizeof(T), MIN_ALIGNED_SIZE);

    if (used_ + total_size > PAGE_SIZE) {
      PageAllocate();
    } else {
      used_ = used_ + total_size;
    }
    void* ret = head_;
    head_ = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(head_) +
                                    total_size);
    return static_cast<pointer>(ret);
  }

  // Deallocate storage p of deleted elements
  void deallocate(pointer p, size_type num) {
    // TODO: Actually free the memory
  }

  // initialize elements of allocated storage p with value value
  void construct(pointer p, const T& value) {
    // initialize memory with placement new
    new ((void*)p) T(value);
  }

  // destroy elements of initialized storage p
  void destroy(pointer p) {
    // destroy objects by calling their destructor
    p->~T();
  }

 private:
  void PageAllocate() {
    used_ = 0;
    head_ = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    REQUIRE(head_ != MAP_FAILED) << "mmap failed: " << strerror(errno);
  }
};

// return that all specializations of this PrivateAllocator are
// interchangeable
template <class T1, class T2>
bool operator==(const PrivateAllocator<T1>&, const PrivateAllocator<T2>&) {
  return true;
}
template <class T1, class T2>
bool operator!=(const PrivateAllocator<T1>&, const PrivateAllocator<T2>&) {
  return false;
}

#endif  // PRIVATE_ALLOCATOR_HH_

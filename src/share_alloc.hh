#ifndef SHARE_ALLOCATOR_HH_
#define SHARE_ALLOCATOR_HH_

#include <string.h>
#include <sys/mman.h>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "log.h"
#include "util.hh"

// Define them as global and back them with a shared mapping so that multiple
// processes can access the same share_head_ and share_used_
static void** share_head_;
static size_t* share_used_;

template <class T>
class ShareAllocator {
 public:
  // type definitions
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;

  // rebind ShareAllocator to type U
  template <class U>
  struct rebind {
    typedef ShareAllocator<U> other;
  };

  // return address of values
  pointer address(reference value) const { return &value; }
  const_pointer address(const_reference value) const { return &value; }

  // constructors and destructors
  // nothing to do because the ShareAllocator has no state
  ShareAllocator() {
    InitHead();
    PageAllocate();
  }

  ShareAllocator(const ShareAllocator&) {}
  template <class U>
  ShareAllocator(const ShareAllocator<U>&) {}
  ~ShareAllocator() {}

  // return maximum number of elements that can be allocated
  size_type max_size() const {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  // pvAllocate but don't initialize num elements of type T by using our own
  // memory manager
  pointer allocate(size_type num) {
    size_t total_size = ROUND_UP(num * sizeof(T), MIN_ALIGNED_SIZE);

    if (*share_used_ + total_size > PAGE_SIZE) {
      PageAllocate();
    } else {
      *share_used_ = *share_used_ + total_size;
    }
    void* ret = *share_head_;
    *share_head_ = reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(*share_head_) + total_size);
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
    *share_used_ = 0;
    *share_head_ = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    REQUIRE(*share_head_ != MAP_FAILED) << "mmap failed: " << strerror(errno);
  }

  void InitHead() {
    void* share_head_buffer = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    REQUIRE(share_head_buffer != MAP_FAILED)
        << "mmap failed: " << strerror(errno);
    share_head_ = new (share_head_buffer)(void*)(nullptr);

    void* share_used_buffer = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    REQUIRE(share_used_buffer != MAP_FAILED)
        << "mmap failed: " << strerror(errno);
    share_used_ = new (share_used_buffer) size_t(0);
  }
};

// return that all specializations of this ShareAllocator are
// interchangeable
template <class T1, class T2>
bool operator==(const ShareAllocator<T1>&, const ShareAllocator<T2>&) {
  return true;
}
template <class T1, class T2>
bool operator!=(const ShareAllocator<T1>&, const ShareAllocator<T2>&) {
  return false;
}

#endif  // SHARE_ALLOCATOR_HH_

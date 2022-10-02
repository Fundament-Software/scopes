/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ALLOC_HPP
#define SCOPES_ALLOC_HPP

#include <vector>

namespace scopes {
  constexpr void no_tracking(void* ptr, size_t size) {}

  template<void (*TRACK)(void* ptr, size_t size)>
  struct GreedyAlloc
  {
  public:
    inline GreedyAlloc() : left(0), size(0), heap(nullptr) {
      _grow(1 << 14);
    }
    inline ~GreedyAlloc() { clean(); }

    inline void* alloc(size_t size) {
      if (size > left)
        _grow(size);
      if (size > left) // out of memory
        return nullptr;
      left -= size;
      void* ptr = heap + left;
      TRACK(ptr, size);
      return ptr;
    }

    inline void clean() {
      if (heap != nullptr)
        cleanup.push_back(heap);
      heap = nullptr;
      left = 0;
      size = 0;
      for (auto p : cleanup)
        free(p);
    }

  private:
    inline void _grow(size_t min) {
      if (heap != nullptr)
        cleanup.push_back(heap);
      if (size < min)
        size = min;

      size *= 2;
      left = 0;
      heap = (uint8_t*)malloc(size);
      if (heap != nullptr)
        left = size;
    }

    size_t left;
    size_t size;
    uint8_t* heap;
    std::vector<uint8_t*> cleanup;
  };
}

#endif
/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "gc.hpp"

#include <algorithm>
#include <map>

namespace scopes {

char *g_stack_start;
size_t g_largest_stack_size = 0;

size_t memory_stack_size() {
    char c; char *_stack_addr = &c;
    size_t ss = (size_t)(g_stack_start - _stack_addr);
    g_largest_stack_size = std::max(ss, g_largest_stack_size);
    return ss;
}

// for allocated pointers, register the size of the range
static std::map<void *, size_t> tracked_allocations;

void track(void *ptr, size_t size) {
    tracked_allocations.insert({ptr,size});
}

void *tracked_malloc(size_t size) {
    void *ptr = malloc(size);
    track(ptr, size);
    return ptr;
}

bool find_allocation(void *srcptr,  void *&start, size_t &size) {
    auto it = tracked_allocations.upper_bound(srcptr);
    if (it == tracked_allocations.begin())
        return false;
    it--;
    start = it->first;
    size = it->second;
    return (srcptr >= start)&&((uint8_t*)srcptr < ((uint8_t*)start + size));
}

} // namespace scopes
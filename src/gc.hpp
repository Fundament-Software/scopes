/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_GC_HPP
#define SCOPES_GC_HPP

#include "result.hpp"

#include <stddef.h>

namespace scopes {

// for allocated pointers, register the size of the range
void track(void *ptr, size_t size);

void *tracked_malloc(size_t size);
bool find_allocation(void *srcptr,  void *&start, size_t &size);

} // namespace scopes

#endif // SCOPES_GC_HPP
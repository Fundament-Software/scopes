/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UTILS_HPP
#define SCOPES_UTILS_HPP

#include <stddef.h>
#include <stdio.h>

#include <functional>
#include <map>

namespace scopes {

int stb_fprintf(FILE *out, const char *fmt, ...);

inline size_t align(size_t offset, size_t align) {
    return (offset + align - 1) & ~(align - 1);
}

inline size_t ceilpow2 (size_t v) {
    v--;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    v |= (v >> 32);
    v++;
    return v;
}

template <typename R, typename... Args>
inline std::function<R (Args...)> memoize(R (*fn)(Args...)) {
    std::map<std::tuple<Args...>, R> table;
    return [fn, table](Args... args) mutable -> R {
        auto argt = std::make_tuple(args...);
        auto memoized = table.find(argt);
        if(memoized == table.end()) {
            auto result = fn(args...);
            table[argt] = result;
            return result;
        } else {
            return memoized->second;
        }
    };
}

} // namespace scopes

#endif // SCOPES_UTILS_HPP
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

#ifndef SCOPES_TYPEFACTORY_HPP
#define SCOPES_TYPEFACTORY_HPP

#include "any.hpp"

#include <vector>
#include <unordered_map>

namespace scopes {

//------------------------------------------------------------------------------
// TYPE FACTORIES
//------------------------------------------------------------------------------

struct TypeFactoryArgs {
    std::vector<Any> args;

    TypeFactoryArgs();
    TypeFactoryArgs(const std::vector<Any> &_args);

    bool operator==(const TypeFactoryArgs &other) const;

    struct Hash {
        std::size_t operator()(const TypeFactoryArgs& s) const;
    };
};

//------------------------------------------------------------------------------

template<typename T>
struct TypeFactory {
    typedef std::unordered_map<TypeFactoryArgs, T *, typename TypeFactoryArgs::Hash> ArgMap;

    ArgMap map;

    const Type *insert(const std::vector<Any> &args) {
        TypeFactoryArgs ta(args);
        typename ArgMap::iterator it = map.find(ta);
        if (it == map.end()) {
            T *t = new T(args);
            map.insert({ta, t});
            return t;
        } else {
            return it->second;
        }
    }

    template <typename... Args>
    const Type *insert(Args... args) {
        TypeFactoryArgs ta({ args... });
        typename ArgMap::iterator it = map.find(ta);
        if (it == map.end()) {
            T *t = new T(args...);
            map.insert({ta, t});
            return t;
        } else {
            return it->second;
        }
    }
};

} // namespace scopes

#endif // SCOPES_TYPEFACTORY_HPP
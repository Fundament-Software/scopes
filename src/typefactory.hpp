/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
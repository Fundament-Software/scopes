/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "typefactory.hpp"

#include "cityhash/city.h"

namespace scopes {

TypeFactoryArgs::TypeFactoryArgs() {}
TypeFactoryArgs::TypeFactoryArgs(const std::vector<Any> &_args) : args(_args) {}

bool TypeFactoryArgs::operator==(const TypeFactoryArgs &other) const {
    if (args.size() != other.args.size()) return false;
    for (size_t i = 0; i < args.size(); ++i) {
        auto &&a = args[i];
        auto &&b = other.args[i];
        if (a != b)
            return false;
    }
    return true;
}

std::size_t TypeFactoryArgs::Hash::operator()(const TypeFactoryArgs& s) const {
    std::size_t h = 0;
    for (auto &&arg : s.args) {
        h = HashLen16(h, arg.hash());
    }
    return h;
}

} // namespace scopes

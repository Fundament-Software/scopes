/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ARGUMENT_HPP
#define SCOPES_ARGUMENT_HPP

#include "symbol.hpp"
#include "any.hpp"

#include <vector>

namespace scopes {

struct Argument {
    Symbol key;
    Any value;

    Argument();
    Argument(Any _value);
    Argument(Symbol _key, Any _value);
    template<typename T>
    Argument(const T &x) : key(SYM_Unnamed), value(x) {}

    bool is_keyed() const;

    bool operator ==(const Argument &other) const;

    bool operator !=(const Argument &other) const;

    uint64_t hash() const;
};

typedef std::vector<Argument> Args;

Argument first(const Args &values);

void stream_args(StyledStream &ss, const Args &args, size_t start = 1);


} // namespace scopes

#endif // SCOPES_ARGUMENT_HPP
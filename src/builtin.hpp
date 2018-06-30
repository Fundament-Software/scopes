/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_BUILTIN_HPP
#define SCOPES_BUILTIN_HPP

#include "symbol.hpp"

namespace scopes {

struct StyledStream;

//------------------------------------------------------------------------------
// BUILTIN
//------------------------------------------------------------------------------

struct Builtin {
    typedef KnownSymbol EnumT;
protected:
    Symbol _name;

public:
    Builtin();
    Builtin(EnumT name);

    EnumT value() const;

    bool operator < (Builtin b) const;
    bool operator ==(Builtin b) const;
    bool operator !=(Builtin b) const;
    bool operator ==(EnumT b) const;
    bool operator !=(EnumT b) const;
    std::size_t hash() const;
    Symbol name() const;

    StyledStream& stream(StyledStream& ost) const;
};

} // namespace scopes

#endif // SCOPES_BUILTIN_HPP

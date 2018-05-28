/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "builtin.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// BUILTIN
//------------------------------------------------------------------------------

Builtin::Builtin(Builtin::EnumT name) :
    _name(name) {
}

Builtin::EnumT Builtin::value() const {
    return _name.known_value();
}

bool Builtin::operator < (Builtin b) const { return _name < b._name; }
bool Builtin::operator ==(Builtin b) const { return _name == b._name; }
bool Builtin::operator !=(Builtin b) const { return _name != b._name; }
bool Builtin::operator ==(EnumT b) const { return _name == b; }
bool Builtin::operator !=(EnumT b) const { return _name != b; }
std::size_t Builtin::hash() const { return _name.hash(); }
Symbol Builtin::name() const { return _name; }

StyledStream& Builtin::stream(StyledStream& ost) const {
    ost << Style_Function; name().name()->stream(ost, ""); ost << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Builtin &builtin) {
    return builtin.stream(ost);
}

StyledStream& operator<<(StyledStream& ost, const Builtin &builtin) {
    return builtin.stream(ost);
}

} // namespace scopes

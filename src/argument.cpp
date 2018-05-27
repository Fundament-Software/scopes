/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "argument.hpp"
#include "hash.hpp"

namespace scopes {

Argument::Argument() : key(SYM_Unnamed), value(none) {}
Argument::Argument(Any _value) : key(SYM_Unnamed), value(_value) {}
Argument::Argument(Symbol _key, Any _value) : key(_key), value(_value) {}

bool Argument::is_keyed() const {
    return key != SYM_Unnamed;
}

bool Argument::operator ==(const Argument &other) const {
    return (key == other.key) && (value == other.value);
}

bool Argument::operator !=(const Argument &other) const {
    return (key != other.key) || (value != other.value);
}

uint64_t Argument::hash() const {
    return hash2(std::hash<uint64_t>{}(key.value()), value.hash());
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Argument value) {
    if (value.key != SYM_Unnamed) {
        ost << value.key << Style_Operator << "=" << Style_None;
    }
    ost << value.value;
    return ost;
}

//------------------------------------------------------------------------------

Argument first(const Args &values) {
    return values.empty()?Argument():values.front();
}

void stream_args(StyledStream &ss, const Args &args, size_t start) {
    for (size_t i = start; i < args.size(); ++i) {
        ss << " ";
        if (is_unknown(args[i].value)) {
            ss << "<unknown>:" << args[i].value.typeref;
        } else {
            ss << args[i].value;
        }
    }
    ss << std::endl;
}

} // namespace scopes

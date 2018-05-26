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

#include "argument.hpp"
#include "cityhash/city.h"

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
    return HashLen16(std::hash<uint64_t>{}(key.value()), value.hash());
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

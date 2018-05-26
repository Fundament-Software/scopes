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

#include "parameter.hpp"
#include "type.hpp"
#include "label.hpp"

namespace scopes {

//------------------------------------------------------------------------------

Parameter::Parameter(const Anchor *_anchor, Symbol _name, const Type *_type, ParameterKind _kind) :
    anchor(_anchor), name(_name), type(_type), label(nullptr), index(-1),
    kind(_kind) {}

bool Parameter::is_vararg() const {
    return (kind == PK_Variadic);
}

bool Parameter::is_typed() const {
    return type != TYPE_Unknown;
}

bool Parameter::is_none() const {
    return type == TYPE_Nothing;
}

StyledStream &Parameter::stream_local(StyledStream &ss) const {
    if ((name != SYM_Unnamed) || !label) {
        ss << Style_Symbol;
        name.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
        ss << Style_None;
    } else {
        ss << Style_Operator << "@" << Style_None << index;
    }
    if (is_vararg()) {
        ss << Style_Keyword << "…" << Style_None;
    }
    if (is_typed()) {
        ss << Style_Operator << ":" << Style_None << type;
    }
    return ss;
}

Parameter *Parameter::from(const Parameter *_param) {
    return new Parameter(
        _param->anchor, _param->name, _param->type, _param->kind);
}

Parameter *Parameter::from(const Anchor *_anchor, Symbol _name, const Type *_type) {
    return new Parameter(_anchor, _name, _type, PK_Regular);
}

Parameter *Parameter::variadic_from(const Anchor *_anchor, Symbol _name, const Type *_type) {
    return new Parameter(_anchor, _name, _type, PK_Variadic);
}

StyledStream &Parameter::stream(StyledStream &ss) const {
    if (label) {
        label->stream_short(ss);
    } else {
        ss << Style_Comment << "<unbound>" << Style_None;
    }
    ss << Style_Comment << "." << Style_None;
    stream_local(ss);
    return ss;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ss, Parameter *param) {
    param->stream(ss);
    return ss;
}

} // namespace scopes

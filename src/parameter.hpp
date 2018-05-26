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

#ifndef SCOPES_PARAMETER_HPP
#define SCOPES_PARAMETER_HPP

#include "symbol.hpp"

#include <vector>

namespace scopes {

struct Anchor;
struct Type;
struct Label;

enum {
    ARG_Cont = 0,
    ARG_Arg0 = 1,
    PARAM_Cont = 0,
    PARAM_Arg0 = 1,
};

enum ParameterKind {
    PK_Regular = 0,
    PK_Variadic = 1,
};

struct Parameter {
protected:
    Parameter(const Anchor *_anchor, Symbol _name, const Type *_type, ParameterKind _kind);

public:
    const Anchor *anchor;
    Symbol name;
    const Type *type;
    Label *label;
    int index;
    ParameterKind kind;

    bool is_vararg() const;

    bool is_typed() const;

    bool is_none() const;

    StyledStream &stream_local(StyledStream &ss) const;
    StyledStream &stream(StyledStream &ss) const;

    static Parameter *from(const Parameter *_param);

    static Parameter *from(const Anchor *_anchor, Symbol _name, const Type *_type);

    static Parameter *variadic_from(const Anchor *_anchor, Symbol _name, const Type *_type);
};

StyledStream& operator<<(StyledStream& ss, Parameter *param);

typedef std::vector<Parameter *> Parameters;

} // namespace scopes

#endif // SCOPES_PARAMETER_HPP
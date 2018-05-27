/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
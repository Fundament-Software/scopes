/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_RETURN_HPP
#define SCOPES_RETURN_HPP

#include "type.hpp"
#include "argument.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// RETURN LABEL TYPE
//------------------------------------------------------------------------------

enum ReturnLabelMode {
    RLM_Return,
    RLM_NoReturn,
};

struct ReturnLabelType : Type {
    static bool classof(const Type *T);

    bool is_returning() const;

    const Type *to_unconst() const;

    ReturnLabelType(ReturnLabelMode _mode, const Args &_values);

    bool has_constants() const;

    bool has_variables() const;

    bool has_multiple_return_values() const;

    Args values;
    const Type *return_type;
    ReturnLabelMode mode;
protected:
    bool has_mrv;
    bool has_const;
    bool has_vars;
};

const Type *ReturnLabel(ReturnLabelMode mode, const Args &values);

const Type *ReturnLabel(const Args &values);

const Type *NoReturnLabel();

} // namespace scopes

#endif // SCOPES_RETURN_HPP
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

enum {
    RLF_NoReturn = (1 << 0),
    // returns a hidden boolean in first position that indicates
    // if the function raised an error; if true, the value of
    // remaining arguments is undefined, and the caller
    // must also raise an error -- thus, a raising function can only
    // be called by other raising functions
    RLF_Raising = (1 << 1),
};

struct ReturnLabelType : Type {
    static bool classof(const Type *T);

    bool is_returning() const;
    bool is_raising() const;

    const Type *to_unconst() const;
    const Type *to_raising() const;
    const Type *to_trycall() const;

    void stream_name(StyledStream &ss) const;
    ReturnLabelType(const Args &_values, uint64_t flags);

    bool has_constants() const;

    bool has_variables() const;

    bool has_multiple_return_values() const;

    Args values;
    const Type *return_type;
    const Type *ll_return_type;
    uint64_t flags;
protected:
    bool has_mrv;
    bool has_const;
    bool has_vars;
};

const Type *ReturnLabel(const Args &values, uint64_t flags = 0);
const Type *NoReturnLabel(uint64_t flags = 0);

} // namespace scopes

#endif // SCOPES_RETURN_HPP
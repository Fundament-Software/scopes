/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_RETURN_HPP
#define SCOPES_RETURN_HPP

#include "type.hpp"
#include "argument.hpp"
#include "symbol.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// RETURN LABEL TYPE
//------------------------------------------------------------------------------

enum {
    RTF_NoReturn = (1 << 0),
    // returns a hidden boolean in first position that indicates
    // if the function raised an error; if true, the value of
    // remaining arguments is undefined, and the caller
    // must also raise an error -- thus, a raising function can only
    // be called by other raising functions
    RTF_Raising = (1 << 1),
};

struct ReturnType : Type {
    static bool classof(const Type *T);

    bool is_returning() const;
    bool is_raising() const;

    const Type *to_raising() const;
    const Type *to_trycall() const;
    const Type *to_single(Symbol key = SYM_Unnamed) const;
    const Type *get_single() const;

    void stream_name(StyledStream &ss) const;
    ReturnType(const KeyedTypes &_values, uint64_t flags);

    KeyedTypes values;
    const Type *return_type;
    uint64_t flags;
};

const Type *Return(const ArgTypes &values, uint64_t flags = 0);
const Type *KeyedReturn(const KeyedTypes &values, uint64_t flags = 0);
const Type *NoReturn(uint64_t flags = 0);

bool is_raising(const Type *T);
bool is_returning(const Type *T);

} // namespace scopes

#endif // SCOPES_RETURN_HPP
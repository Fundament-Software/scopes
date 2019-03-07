/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_REFER_QUALIFIER_HPP
#define SCOPES_REFER_QUALIFIER_HPP

#include "../type/pointer_type.hpp"
#include "../type/qualify_type.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// REFER QUALIFIER
//------------------------------------------------------------------------------

struct ReferQualifier : Qualifier {
    enum { Kind = QK_Refer };

    static bool classof(const Qualifier *T);

    ReferQualifier(uint64_t _flags, Symbol _storage_class);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    const Type *get_pointer_type(const Type *ET) const;

    uint64_t flags;
    Symbol storage_class;
};

const Type *refer_type(const Type *element_type, uint64_t flags,
    Symbol storage_class);
uint64_t refer_flags(const Type *T);
Symbol refer_storage_class(const Type *T);

bool is_reference(const Type *T);

} // namespace scopes

#endif // SCOPES_REFER_QUALIFIER_HPP

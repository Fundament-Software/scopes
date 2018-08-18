/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "arguments_type.hpp"
#include "tuple_type.hpp"
#include "typename_type.hpp"
#include "hash.hpp"
#include "dyn_cast.inc"
#include "result.hpp"

#include <assert.h>

#include <unordered_set>

namespace scopes {

static std::unordered_map<const Type *, const Type *> arguments;

//------------------------------------------------------------------------------
// ARGUMENTS TYPE
//------------------------------------------------------------------------------

const Type *keyed_arguments_type(const KeyedTypes &values) {
    if ((values.size() == 1)
            && (values[0].key == SYM_Unnamed))
        return values[0].type;
    auto ST = keyed_tuple_type(values).assert_ok();
    auto it = arguments.find(ST);
    if (it != arguments.end())
        return it->second;
    StyledString ss = StyledString::plain();
    if (values.size() == 0) {
        ss.out << "void";
    } else {
        ss.out << "λ(";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                ss.out << " ";
            }
            if (values[i].key != SYM_Unnamed) {
                ss.out << values[i].key.name()->data << "=";
            }
            stream_type_name(ss.out, values[i].type);
        }
        ss.out << ")";
    }
    auto T = typename_type(ss.str());
    auto tn = const_cast<TypenameType *>(cast<TypenameType>(T));
    tn->super_type = TYPE_Arguments;
    tn->finalize(ST).assert_ok();
    arguments.insert({ST, T});
    return T;
}

const Type *arguments_type(const ArgTypes &values) {
    KeyedTypes types;
    for (auto &&val : values) {
        types.push_back(val);
    }
    return keyed_arguments_type(types);
}

static const Type *empty_type = nullptr;
const Type *empty_arguments_type() {
    if (!empty_type) {
        empty_type = arguments_type({});
     }
    return empty_type;
}

bool is_arguments_type(const Type *T) {
    return superof(T) == TYPE_Arguments;
}

} // namespace scopes

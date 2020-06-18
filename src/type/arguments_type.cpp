/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "arguments_type.hpp"
#include "tuple_type.hpp"
#include "typename_type.hpp"
#include "../hash.hpp"
#include "../dyn_cast.inc"
#include "../result.hpp"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace ArgumentsSet {
    struct Hash {
        std::size_t operator()(const ArgumentsType *s) const {
            std::size_t h = 0;
            for (auto &&arg : s->values) {
                h = hash2(h, std::hash<const Type *>{}(arg));
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ArgumentsType *lhs, const ArgumentsType *rhs ) const {
            if (lhs->values.size() != rhs->values.size()) return false;
            for (size_t i = 0; i < lhs->values.size(); ++i) {
                auto &&a = lhs->values[i];
                auto &&b = rhs->values[i];
                if (a != b)
                    return false;
            }
            return true;
        }
    };
} // namespace TupleSet

static std::unordered_set<const ArgumentsType *,
    ArgumentsSet::Hash, ArgumentsSet::KeyEqual> arguments;

//------------------------------------------------------------------------------
// ARGUMENTS TYPE
//------------------------------------------------------------------------------

void ArgumentsType::stream_name(StyledStream &ss) const {
    if (values.size() == 0) {
        ss << "void";
    } else {
        ss << "λ(";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                ss << " ";
            }
            stream_type_name(ss, values[i]);
        }
        ss << ")";
    }
}

const TupleType *ArgumentsType::to_tuple_type() const {
    assert(!values.empty());
    Types storage_types;
    for (auto &&value : values) {
        storage_types.push_back(qualified_storage_type(value).assert_ok());
    }
    return cast<TupleType>(tuple_type(storage_types).assert_ok());
}

ArgumentsType::ArgumentsType(const Types &_values) :
    Type(TK_Arguments), values(_values) {
}

SCOPES_RESULT(const Type *) ArgumentsType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return values[i];
}

const Type *arguments_type(const Types &values) {
    if (values.size() == 1)
        return values[0];
    Types newvalues;
    int idx = 1;
    for (auto &&value : values) {
        if (is_arguments_type(value)) {
            auto at = cast<ArgumentsType>(value);
            for (auto &&value : at->values) {
                newvalues.push_back(value);
                if (idx != values.size())
                    break;
            }
        } else {
            newvalues.push_back(value);
        }
        idx++;
    }
    ArgumentsType key(newvalues);
    auto it = arguments.find(&key);
    if (it != arguments.end())
        return *it;
    auto result = new ArgumentsType(newvalues);
    arguments.insert(result);
    return result;
}

static const Type *empty_type = nullptr;
const Type *empty_arguments_type() {
    if (!empty_type) {
        empty_type = arguments_type({});
     }
    return empty_type;
}

bool is_arguments_type(const Type *T) {
    return T->kind() == TK_Arguments;
}

//------------------------------------------------------------------------------

int get_argument_count(const Type *T) {
    if (isa<ArgumentsType>(T)) {
        return cast<ArgumentsType>(T)->values.size();
    } else if (T == TYPE_NoReturn) {
        return 0;
    } else {
        return 1;
    }
}

const Type *get_argument(const Type *T, int index) {
    if (!is_returning(T))
        return TYPE_NoReturn;
    if (isa<ArgumentsType>(T)) {
        auto at = cast<ArgumentsType>(T);
        if (index < at->values.size()) {
            return at->values[index];
        }
    } else if (index == 0) {
        return T;
    }
    return TYPE_Nothing;
}

//------------------------------------------------------------------------------


} // namespace scopes

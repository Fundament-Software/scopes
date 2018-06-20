/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "return.hpp"
#include "tuple.hpp"
#include "hash.hpp"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace ReturnLabelSet {
    struct Hash {
        std::size_t operator()(const ReturnLabelType *s) const {
            std::size_t h = std::hash<uint64_t>{}(s->flags);
            for (auto &&arg : s->values) {
                h = hash2(h, arg.hash());
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ReturnLabelType *lhs, const ReturnLabelType *rhs ) const {
            return (lhs->flags == rhs->flags)
                && (lhs->values == rhs->values);
        }
    };
} // namespace ReturnLabelSet

static std::unordered_set<const ReturnLabelType *, ReturnLabelSet::Hash, ReturnLabelSet::KeyEqual> return_labels;


//------------------------------------------------------------------------------
// RETURN LABEL TYPE
//------------------------------------------------------------------------------

bool ReturnLabelType::classof(const Type *T) {
    return T->kind() == TK_ReturnLabel;
}

bool ReturnLabelType::is_raising() const {
    return flags & RLF_Raising;
}

bool ReturnLabelType::is_returning() const {
    return !(flags & RLF_NoReturn);
}

const Type *ReturnLabelType::to_unconst() const {
    if (!has_const)
        return this;
    // unconst all constant types
    Args rvalues;
    for (size_t i = 0; i < values.size(); ++i) {
        if (is_unknown(values[i].value)) {
            rvalues.push_back(values[i].value);
        } else {
            rvalues.push_back(
                Argument(
                    values[i].key,
                    unknown_of(values[i].value.type)));
        }
    }
    return ReturnLabel(rvalues, flags);
}

const Type *ReturnLabelType::to_trycall() const {
    if (!is_raising())
        return this;
    if (return_type == TYPE_Void) {
        return ReturnLabel({unknown_of(TYPE_Bool)}, flags & ~RLF_Raising);
    } else {
        return ReturnLabel({unknown_of(TYPE_Bool), unknown_of(return_type)}, flags & ~RLF_Raising);
    }
}

const Type *ReturnLabelType::to_raising() const {
    if (is_raising())
        return this;
    return ReturnLabel(values, flags | RLF_Raising);
}

void ReturnLabelType::stream_name(StyledStream &ss) const {
    if (is_returning()) {
        ss << "λ(";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                ss << " ";
            }
            if (values[i].key != SYM_Unnamed) {
                ss << values[i].key.name()->data << "=";
            }
            if (is_unknown(values[i].value)) {
                stream_type_name(ss, values[i].value.typeref);
            } else {
                ss << "!";
                stream_type_name(ss, values[i].value.type);
            }
        }
        ss << ")";
    } else {
        ss << "λ<noreturn>";
    }
    if (is_raising()) {
        ss << "!";
    }
}

ReturnLabelType::ReturnLabelType(const Args &_values, uint64_t _flags)
    : Type(TK_ReturnLabel) {
    values = _values;
    flags = _flags;

    has_const = false;
    has_vars = false;
    if (is_returning()) {
        for (size_t i = 0; i < values.size(); ++i) {
            if (is_unknown(values[i].value)) {
                has_vars = true;
            } else {
                has_const = true;
            }
        }

        {
            ArgTypes rettypes;
            for (size_t i = 0; i < values.size(); ++i) {
                if (is_unknown(values[i].value)) {
                    rettypes.push_back(values[i].value.typeref);
                } else {
                    rettypes.push_back(values[i].value.type);
                }
            }

            if (rettypes.size() == 1) {
                return_type = rettypes[0];
                has_mrv = false;
            } else if (!rettypes.empty()) {
                return_type = Tuple(rettypes).assert_ok();
                has_mrv = true;
            } else {
                return_type = TYPE_Void;
                has_mrv = false;
            }
            if (is_raising()) {
                if (return_type == TYPE_Void) {
                    ll_return_type = TYPE_Bool;
                } else {
                    ll_return_type = Tuple({TYPE_Bool, return_type}).assert_ok();
                }
            } else {
                ll_return_type = return_type;
            }
        }
    } else {
        return_type = TYPE_Void;
        ll_return_type = return_type;
        has_mrv = false;
    }
}

bool ReturnLabelType::has_constants() const {
    return has_const;
}

bool ReturnLabelType::has_variables() const {
    return has_vars;
}

bool ReturnLabelType::has_multiple_return_values() const {
    return has_mrv;
}

//------------------------------------------------------------------------------

const Type *ReturnLabel(const Args &values, uint64_t flags) {
    ReturnLabelType key(values, flags);
    auto it = return_labels.find(&key);
    if (it != return_labels.end())
        return *it;

#ifdef SCOPES_DEBUG
    for (size_t i = 0; i < values.size(); ++i) {
        assert(values[i].value.is_const());
        assert(is_unknown(values[i].value));
    }
#endif

    auto result = new ReturnLabelType(values, flags);
    return_labels.insert(result);
    return result;
}

const Type *ReturnType(const ArgTypes &values, uint64_t flags) {
    Args args;
    for (auto &&val : values) {
        args.push_back(unknown_of(val));
    }
    return ReturnLabel(args, flags);
}

const Type *NoReturnLabel(uint64_t flags) {
    return ReturnLabel({}, flags | RLF_NoReturn);
}

} // namespace scopes

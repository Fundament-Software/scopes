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

//------------------------------------------------------------------------------
// RETURN LABEL TYPE
//------------------------------------------------------------------------------

bool ReturnLabelType::classof(const Type *T) {
    return T->kind() == TK_ReturnLabel;
}

bool ReturnLabelType::is_returning() const {
    return mode != RLM_NoReturn;
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
    return ReturnLabel(rvalues);
}

ReturnLabelType::ReturnLabelType(ReturnLabelMode _mode, const Args &_values)
    : Type(TK_ReturnLabel) {
    values = _values;
    mode = _mode;

    has_const = false;
    has_vars = false;
    switch(mode) {
    case RLM_Return: {
        StyledString ss = StyledString::plain();
        ss.out << "λ(";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                ss.out << " ";
            }
            if (values[i].key != SYM_Unnamed) {
                ss.out << values[i].key.name()->data << "=";
            }
            if (is_unknown(values[i].value)) {
                ss.out << values[i].value.typeref->name()->data;
                has_vars = true;
            } else {
                ss.out << "!" << values[i].value.type;
                has_const = true;
            }
        }
        ss.out << ")";
        _name = ss.str();

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
                return_type = Tuple(rettypes);
                has_mrv = true;
            } else {
                return_type = TYPE_Void;
                has_mrv = false;
            }
        }
    } break;
    case RLM_NoReturn: {
        _name = String::from("λ<noreturn>");
        return_type = TYPE_Void;
        has_mrv = false;
    } break;
    default:
        assert(false);
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

const Type *ReturnLabel(ReturnLabelMode mode, const Args &values) {
    struct Equal {
        bool operator()( const ReturnLabelType *lhs, const ReturnLabelType *rhs ) const {
            return (lhs->mode == rhs->mode)
                && (lhs->values == rhs->values);
        }
    };

    struct Hash {
        std::size_t operator()(const ReturnLabelType *s) const {
            std::size_t h = std::hash<int32_t>{}((int32_t)s->mode);
            for (auto &&arg : s->values) {
                h = hash2(h, arg.hash());
            }
            return h;
        }
    };

    typedef std::unordered_set<ReturnLabelType *, Hash, Equal> ArgMap;

    static ArgMap map;

#ifdef SCOPES_DEBUG
    for (size_t i = 0; i < values.size(); ++i) {
        assert(values[i].value.is_const());
        assert(is_unknown(values[i].value));
    }
#endif

    ReturnLabelType key(mode, values);

    typename ArgMap::iterator it = map.find(&key);
    if (it == map.end()) {
        ReturnLabelType *t = new ReturnLabelType(mode, values);
        map.insert(t);
        return t;
    } else {
        return *it;
    }
}

const Type *ReturnLabel(const Args &values) {
    return ReturnLabel(RLM_Return, values);
}

const Type *NoReturnLabel() {
    return ReturnLabel(RLM_NoReturn, {});
}

} // namespace scopes

/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "function_type.hpp"
#include "arguments_type.hpp"
#include "tuple_type.hpp"
#include "pointer_type.hpp"
#include "qualify_type.hpp"
#include "../error.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"

#include "../qualifier/unique_qualifiers.hpp"
#include "../qualifier.inc"
#include "absl/container/flat_hash_map.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// FUNCTION TYPE
//------------------------------------------------------------------------------

void FunctionType::stream_name(StyledStream &ss) const {
    ss << "(";
    stream_type_name(ss, return_type);
    ss << " <-: (";
    for (size_t i = 0; i < argument_types.size(); ++i) {
        if (i > 0)
            ss << " ";
        stream_type_name(ss, argument_types[i]);
    }
    if (vararg()) {
        ss << " ...";
    }
    ss << ")";
    if (has_exception()) {
        ss << " raises ";
        stream_type_name(ss, except_type);
    }
    ss << ")";
}

FunctionType::FunctionType(const Type *_except_type,
    const Type *_return_type,
    const Types &_argument_types, uint32_t _flags) :
    Type(TK_Function),
    except_type(_except_type),
    return_type(_return_type),
    argument_types(_argument_types),
    flags(_flags),
    stripped(nullptr) {
    assert(except_type);
    assert(return_type);
}

const FunctionType *FunctionType::strip_annotations() const {
    if (!stripped) {
        Types args;
        for (auto arg : argument_types) {
            args.push_back(strip_qualifiers(arg, QM_Annotations));
        }
        stripped =
        cast<FunctionType>(raising_function_type(
            strip_qualifiers(except_type, QM_Annotations),
            strip_qualifiers(return_type, QM_Annotations),
            args,
            flags));
    }
    return stripped;
}

bool FunctionType::has_exception() const {
    return except_type != TYPE_NoReturn;
}

bool FunctionType::vararg() const {
    return flags & FF_Variadic;
}

SCOPES_RESULT(const Type *) FunctionType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, argument_types.size()));
    return argument_types[i];
}

//------------------------------------------------------------------------------

static void canonicalize_unique_types(ID2SetMap &idmap, Types &types,
    int idoffset = 0, int idscale = 1) {
    int argcount = types.size();
    // find and remap uniques
    for (int i = 0; i < argcount; ++i) {
        auto &&T = types[i];
        auto vq = try_qualifier<ViewQualifier>(T);
        if (vq) {
            continue;
        }
        auto uq = try_qualifier<UniqueQualifier>(T);
        if (uq) {
            int id = i * idscale + idoffset;
            auto result = idmap.insert({uq->id, { id } });
            if (!result.second) {
                result.first->second.insert(id);
            }
            T = unique_type(T, id);
            continue;
        } else if (!is_plain(T)) {
            int id = i * idscale + idoffset;
            T = unique_type(T, id);
        }
    }
    // remap views with possibly local references
    for (int i = 0; i < argcount; ++i) {
        auto &&T = types[i];
        auto vq = try_qualifier<ViewQualifier>(T);
        if (!vq) continue;
        if (vq->ids.empty()) {
            int id = i * idscale + idoffset;
            T = view_type(T, { id });
            continue;
        }
        int idcount = vq->sorted_ids.size();
        IDSet ids;
        ids.reserve(idcount);
        for (int k = 0; k < idcount; ++k) {
            int id = vq->sorted_ids[k];
            auto it = idmap.find(id);
            if (it == idmap.end()) {
                int newid = i * idscale + idoffset;
                ids.insert(newid);
                auto result = idmap.insert({id, { newid } });
                if (!result.second) {
                    result.first->second.insert(newid);
                }
            } else {
                for (auto &&newid : it->second) {
                    ids.insert(newid);
                }
            }
        }
        T = view_type(strip_qualifier<ViewQualifier>(T), ids);
    }
}

//------------------------------------------------------------------------------

void canonicalize_argument_types(Types &types) {
    ID2SetMap idmap;
    canonicalize_unique_types(idmap, types, 1);
}

const Type *raising_function_type(const Type *except_type, const Type *return_type,
    Types argument_types, uint32_t flags) {

    struct TypeArgs {
        const Type *except_type;
        const Type *return_type;
        Types argtypes;
        uint32_t flags;

        TypeArgs() {}
        TypeArgs(const Type *_except_type,
            const Type *_return_type,
            const Types &_argument_types,
            uint32_t _flags = 0) :
            except_type(_except_type),
            return_type(_return_type),
            argtypes(_argument_types),
            flags(_flags)
        {}

        bool operator==(const TypeArgs &other) const {
            if (except_type != other.except_type) return false;
            if (return_type != other.return_type) return false;
            if (flags != other.flags) return false;
            if (argtypes.size() != other.argtypes.size()) return false;
            for (size_t i = 0; i < argtypes.size(); ++i) {
                if (argtypes[i] != other.argtypes[i])
                    return false;
            }
            return true;
        }

        struct Hash {
            std::size_t operator()(const TypeArgs& s) const {
                std::size_t h = std::hash<const Type *>{}(s.except_type);
                h = hash2(h, std::hash<const Type *>{}(s.return_type));
                h = hash2(h, std::hash<uint32_t>{}(s.flags));
                for (auto arg : s.argtypes) {
                    h = hash2(h, std::hash<const Type *>{}(arg));
                }
                return h;
            }
        };
    };

    typedef absl::flat_hash_map<TypeArgs, FunctionType *, typename TypeArgs::Hash> ArgMap;

    static ArgMap map;

#if 1
    TypeArgs ta(except_type, return_type, argument_types, flags);
    typename ArgMap::iterator it = map.find(ta);
    if (it == map.end()) {
        FunctionType *t = new FunctionType(except_type, return_type, argument_types, flags);
        map.insert({ta, t});
        return t;
    } else {
        return it->second;
    }
#else
    TypeArgs ta(except_type, return_type, argument_types, flags);
    typename ArgMap::iterator it = map.find(ta);
    if (it == map.end()) {
        // bring unique arguments into canonical order
        ID2SetMap idmap;
        canonicalize_unique_types(idmap, argument_types, 1);

        if (is_returning_value(except_type)) {
            ID2SetMap idmap2 = idmap;
            auto ecount = get_argument_count(except_type);
            Types retargs;
            retargs.reserve(ecount);
            for (int i = 0; i < ecount; ++i) {
                retargs.push_back(get_argument(except_type, i));
            }
            canonicalize_unique_types(idmap2, retargs, -257, -1);
            except_type = arguments_type(retargs);
        }
        if (is_returning_value(return_type)) {
            auto rcount = get_argument_count(return_type);
            Types retargs;
            retargs.reserve(rcount);
            for (int i = 0; i < rcount; ++i) {
                retargs.push_back(get_argument(return_type, i));
            }
            canonicalize_unique_types(idmap, retargs, -1, -1);
            return_type = arguments_type(retargs);
        }

        TypeArgs tb(except_type, return_type, argument_types, flags);
        it = map.find(tb);
        if (it == map.end()) {
            FunctionType *t = new FunctionType(except_type, return_type, argument_types, flags);
            map.insert({ta, t});
            if (!(ta == tb)) {
                map.insert({tb, t});
            }
            return t;
        } else {
            return it->second;
        }
    } else {
        return it->second;
    }
#endif
}

const Type *raising_function_type(const Type *return_type,
    const Types &argument_types, uint32_t flags) {
    return raising_function_type(TYPE_Error, return_type, argument_types, flags);
}

const Type *function_type(const Type *return_type,
    const Types &argument_types, uint32_t flags) {
    return raising_function_type(TYPE_NoReturn, return_type, argument_types, flags);
}

//------------------------------------------------------------------------------

bool is_function_pointer(const Type *type) {
    switch (type->kind()) {
    case TK_Pointer: {
        const PointerType *ptype = cast<PointerType>(type);
        return isa<FunctionType>(ptype->element_type);
    } break;
    default: return false;
    }
}

const FunctionType *extract_function_type(const Type *T) {
    switch(T->kind()) {
    case TK_Pointer: {
        auto pi = cast<PointerType>(T);
        return cast<FunctionType>(pi->element_type);
    } break;
    default: assert(false && "unexpected function type");
        return nullptr;
    }
}

SCOPES_RESULT(void) verify_function_pointer(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (!is_function_pointer(type)) {
        SCOPES_ERROR(FunctionPointerExpected, type);
    }
    return {};
}

} // namespace scopes

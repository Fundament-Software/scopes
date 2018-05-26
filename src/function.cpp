/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "function.hpp"
#include "return.hpp"
#include "tuple.hpp"
#include "pointer.hpp"
#include "extern.hpp"
#include "error.hpp"

#include <assert.h>

#include "cityhash/city.h"

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------
// FUNCTION TYPE
//------------------------------------------------------------------------------

bool FunctionType::classof(const Type *T) {
    return T->kind() == TK_Function;
}

FunctionType::FunctionType(
    const Type *_return_type, const ArgTypes &_argument_types, uint32_t _flags) :
    Type(TK_Function),
    return_type(_return_type),
    argument_types(_argument_types),
    flags(_flags) {

    assert(!(flags & FF_Divergent) || argument_types.empty());

    std::stringstream ss;
    if (divergent()) {
        ss << "?<-";
    } else {
        ss <<  return_type->name()->data;
        if (pure()) {
            ss << "<~";
        } else {
            ss << "<-";
        }
    }
    ss << "(";
    for (size_t i = 0; i < argument_types.size(); ++i) {
        if (i > 0) {
            ss << " ";
        }
        ss << argument_types[i]->name()->data;
    }
    if (vararg()) {
        ss << " ...";
    }
    ss << ")";
    _name = String::from_stdstring(ss.str());
}

bool FunctionType::vararg() const {
    return flags & FF_Variadic;
}
bool FunctionType::pure() const {
    return flags & FF_Pure;
}
bool FunctionType::divergent() const {
    return flags & FF_Divergent;
}

const Type *FunctionType::type_at_index(size_t i) const {
    verify_range(i, argument_types.size() + 1);
    if (i == 0)
        return return_type;
    else
        return argument_types[i - 1];
}

//------------------------------------------------------------------------------

const Type *Function(const Type *return_type,
    const ArgTypes &argument_types, uint32_t flags) {

    struct TypeArgs {
        const Type *return_type;
        ArgTypes argtypes;
        uint32_t flags;

        TypeArgs() {}
        TypeArgs(const Type *_return_type,
            const ArgTypes &_argument_types,
            uint32_t _flags = 0) :
            return_type(_return_type),
            argtypes(_argument_types),
            flags(_flags)
        {}

        bool operator==(const TypeArgs &other) const {
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
                std::size_t h = std::hash<const Type *>{}(s.return_type);
                h = HashLen16(h, std::hash<uint32_t>{}(s.flags));
                for (auto arg : s.argtypes) {
                    h = HashLen16(h, std::hash<const Type *>{}(arg));
                }
                return h;
            }
        };
    };

    typedef std::unordered_map<TypeArgs, FunctionType *, typename TypeArgs::Hash> ArgMap;

    static ArgMap map;

    if (return_type->kind() != TK_ReturnLabel) {
        if (return_type == TYPE_Void) {
            return_type = ReturnLabel({});
        } else if (return_type->kind() == TK_Tuple) {
            auto &&types = cast<TupleType>(return_type)->types;
            Args values;
            for (auto it = types.begin(); it != types.end(); ++it) {
                values.push_back(unknown_of(*it));
            }
            return_type = ReturnLabel(values);
        } else {
            return_type = ReturnLabel({unknown_of(return_type)});
        }
    }

    TypeArgs ta(return_type, argument_types, flags);
    typename ArgMap::iterator it = map.find(ta);
    if (it == map.end()) {
        FunctionType *t = new FunctionType(return_type, argument_types, flags);
        map.insert({ta, t});
        return t;
    } else {
        return it->second;
    }
}

//------------------------------------------------------------------------------

bool is_function_pointer(const Type *type) {
    switch (type->kind()) {
    case TK_Pointer: {
        const PointerType *ptype = cast<PointerType>(type);
        return isa<FunctionType>(ptype->element_type);
    } break;
    case TK_Extern: {
        const ExternType *etype = cast<ExternType>(type);
        return isa<FunctionType>(etype->type);
    } break;
    default: return false;
    }
}

bool is_pure_function_pointer(const Type *type) {
    const PointerType *ptype = dyn_cast<PointerType>(type);
    if (!ptype) return false;
    const FunctionType *ftype = dyn_cast<FunctionType>(ptype->element_type);
    if (!ftype) return false;
    return ftype->flags & FF_Pure;
}

const FunctionType *extract_function_type(const Type *T) {
    switch(T->kind()) {
    case TK_Extern: {
        auto et = cast<ExternType>(T);
        return cast<FunctionType>(et->type);
    } break;
    case TK_Pointer: {
        auto pi = cast<PointerType>(T);
        return cast<FunctionType>(pi->element_type);
    } break;
    default: assert(false && "unexpected function type");
        return nullptr;
    }
}

void verify_function_pointer(const Type *type) {
    if (!is_function_pointer(type)) {
        StyledString ss;
        ss.out << "function pointer expected, got " << type;
        location_error(ss.str());
    }
}

} // namespace scopes

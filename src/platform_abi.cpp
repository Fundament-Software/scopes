/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "platform_abi.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "dyn_cast.inc"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// PLATFORM ABI
//------------------------------------------------------------------------------

// life is unfair, which is why we need to implement the remaining platform ABI
// support in the front-end, particularly whether an argument is passed by
// value or not.

const char *abi_class_to_string(ABIClass class_) {
    switch(class_) {
    #define T(X) case ABI_CLASS_ ## X: return #X;
    DEF_ABI_CLASS_NAMES
    #undef T
    default: return "?";
    }
}

#undef DEF_ABI_CLASS_NAMES

#ifdef SCOPES_WIN32
#else
// x86-64 PS ABI based on https://www.uclibc.org/docs/psABI-x86_64.pdf

static ABIClass merge_abi_classes(ABIClass class1, ABIClass class2) {
    if (class1 == class2)
        return class1;

    if (class1 == ABI_CLASS_NO_CLASS)
        return class2;
    if (class2 == ABI_CLASS_NO_CLASS)
        return class1;

    if (class1 == ABI_CLASS_MEMORY || class2 == ABI_CLASS_MEMORY)
        return ABI_CLASS_MEMORY;

    if ((class1 == ABI_CLASS_INTEGERSI && class2 == ABI_CLASS_SSESF)
        || (class2 == ABI_CLASS_INTEGERSI && class1 == ABI_CLASS_SSESF))
        return ABI_CLASS_INTEGERSI;
    if (class1 == ABI_CLASS_INTEGER || class1 == ABI_CLASS_INTEGERSI
        || class2 == ABI_CLASS_INTEGER || class2 == ABI_CLASS_INTEGERSI)
        return ABI_CLASS_INTEGER;

    if (class1 == ABI_CLASS_X87
        || class1 == ABI_CLASS_X87UP
        || class1 == ABI_CLASS_COMPLEX_X87
        || class2 == ABI_CLASS_X87
        || class2 == ABI_CLASS_X87UP
        || class2 == ABI_CLASS_COMPLEX_X87)
        return ABI_CLASS_MEMORY;

    return ABI_CLASS_SSE;
}

static size_t classify(const Type *T, ABIClass *classes, size_t offset);

static size_t classify_array_like(size_t size,
    const Type *element_type, size_t count,
    ABIClass *classes, size_t offset) {
    const size_t UNITS_PER_WORD = 8;
    size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
    if (size > 32)
        return 0;
    for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
        classes[i] = ABI_CLASS_NO_CLASS;
    if (!words) {
        classes[0] = ABI_CLASS_NO_CLASS;
        return 1;
    }
    auto ET = element_type;
    ABIClass subclasses[MAX_ABI_CLASSES];
    size_t alignment = align_of(ET).assert_ok();
    size_t esize = size_of(ET).assert_ok();
    for (size_t i = 0; i < count; ++i) {
        offset = align(offset, alignment);
        size_t num = classify(ET, subclasses, offset % 8);
        if (!num) return 0;
        for (size_t k = 0; k < num; ++k) {
            size_t pos = offset / 8;
            classes[k + pos] =
                merge_abi_classes (subclasses[k], classes[k + pos]);
        }
        offset += esize;
    }
    if (words > 2) {
        if (classes[0] != ABI_CLASS_SSE)
            return 0;
        for (size_t i = 1; i < words; ++i) {
            if (classes[i] != ABI_CLASS_SSEUP)
                return 0;
        }
    }
    for (size_t i = 0; i < words; i++) {
        if (classes[i] == ABI_CLASS_MEMORY)
            return 0;

        if (classes[i] == ABI_CLASS_SSEUP) {
            assert(i > 0);
            if (classes[i - 1] != ABI_CLASS_SSE
                && classes[i - 1] != ABI_CLASS_SSEUP) {
                classes[i] = ABI_CLASS_SSE;
            }
        }

        if (classes[i] == ABI_CLASS_X87UP) {
            assert(i > 0);
            if(classes[i - 1] != ABI_CLASS_X87) {
                return 0;
            }
        }
    }
    return words;
}

static size_t classify(const Type *T, ABIClass *classes, size_t offset) {
    switch(T->kind()) {
    case TK_Integer:
    case TK_Extern:
    case TK_Pointer: {
        size_t size = size_of(T).assert_ok() + offset;
        if (size <= 4) {
            classes[0] = ABI_CLASS_INTEGERSI;
            return 1;
        } else if (size <= 8) {
            classes[0] = ABI_CLASS_INTEGER;
            return 1;
        } else if (size <= 12) {
            classes[0] = ABI_CLASS_INTEGER;
            classes[1] = ABI_CLASS_INTEGERSI;
            return 2;
        } else if (size <= 16) {
            classes[0] = ABI_CLASS_INTEGER;
            classes[1] = ABI_CLASS_INTEGER;
            return 2;
        } else {
            assert(false && "illegal type");
        }
    } break;
    case TK_Real: {
        size_t size = size_of(T).assert_ok();
        if (size == 4) {
            if (!(offset % 8))
                classes[0] = ABI_CLASS_SSESF;
            else
                classes[0] = ABI_CLASS_SSE;
            return 1;
        } else if (size == 8) {
            classes[0] = ABI_CLASS_SSEDF;
            return 1;
        } else {
            assert(false && "illegal type");
        }
    } break;
    case TK_ReturnLabel:
    case TK_Typename: {
        if (is_opaque(T)) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        } else {
            return classify(storage_type(T).assert_ok(), classes, offset);
        }
    } break;
    case TK_Vector: {
        auto tt = cast<VectorType>(T);
        return classify_array_like(size_of(T).assert_ok(),
            tt->element_type, tt->count, classes, offset);
    } break;
    case TK_Array: {
        auto tt = cast<ArrayType>(T);
        return classify_array_like(size_of(T).assert_ok(),
            tt->element_type, tt->count, classes, offset);
    } break;
    case TK_Union: {
        auto ut = cast<UnionType>(T);
        return classify(ut->types[ut->largest_field], classes, offset);
    } break;
    case TK_Tuple: {
        const size_t UNITS_PER_WORD = 8;
        size_t size = size_of(T).assert_ok();
	    size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
        if (size > 32)
            return 0;
        for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
	        classes[i] = ABI_CLASS_NO_CLASS;
        if (!words) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        }
        auto tt = cast<TupleType>(T);
        ABIClass subclasses[MAX_ABI_CLASSES];
        for (size_t i = 0; i < tt->types.size(); ++i) {
            auto ET = tt->types[i];
            if (!tt->packed)
                offset = align(offset, align_of(ET).assert_ok());
            size_t num = classify (ET, subclasses, offset % 8);
            if (!num) return 0;
            for (size_t k = 0; k < num; ++k) {
                size_t pos = offset / 8;
		        classes[k + pos] =
		            merge_abi_classes (subclasses[k], classes[k + pos]);
            }
            offset += size_of(ET).assert_ok();
        }
        if (words > 2) {
            if (classes[0] != ABI_CLASS_SSE)
                return 0;
            for (size_t i = 1; i < words; ++i) {
                if (classes[i] != ABI_CLASS_SSEUP)
                    return 0;
            }
        }
        for (size_t i = 0; i < words; i++) {
            if (classes[i] == ABI_CLASS_MEMORY)
                return 0;

            if (classes[i] == ABI_CLASS_SSEUP) {
                assert(i > 0);
                if (classes[i - 1] != ABI_CLASS_SSE
                    && classes[i - 1] != ABI_CLASS_SSEUP) {
                    classes[i] = ABI_CLASS_SSE;
                }
            }

            if (classes[i] == ABI_CLASS_X87UP) {
                assert(i > 0);
                if(classes[i - 1] != ABI_CLASS_X87) {
                    return 0;
                }
            }
        }
        return words;
    } break;
    default: {
        assert(false && "not supported in ABI");
        return 0;
    } break;
    }
    return 0;
}
#endif // SCOPES_WIN32

size_t abi_classify(const Type *T, ABIClass *classes) {
#ifdef SCOPES_WIN32
    if (T->kind() == TK_ReturnLabel) {
        T = cast<ReturnLabelType>(T)->return_type;
    }
    classes[0] = ABI_CLASS_NO_CLASS;
    if (is_opaque(T))
        return 1;
    T = storage_type(T);
    size_t sz = size_of(T);
    if (sz > 8)
        return 0;
    switch(T->kind()) {
    case TK_Array:
    case TK_Union:
    case TK_Tuple:
        if (sz <= 1)
            classes[0] = ABI_CLASS_INTEGERSI8;
        else if (sz <= 2)
            classes[0] = ABI_CLASS_INTEGERSI16;
        else if (sz <= 4)
            classes[0] = ABI_CLASS_INTEGERSI;
        else
            classes[0] = ABI_CLASS_INTEGER;
        return 1;
    case TK_Integer:
    case TK_Extern:
    case TK_Pointer:
    case TK_Real:
    case TK_ReturnLabel:
    case TK_Typename:
    case TK_Vector:
    default:
        return 1;
    }
#else
    size_t sz = classify(T, classes, 0);
#if 0
    if (sz) {
        StyledStream ss(std::cout);
        ss << T << " -> " << sz;
        for (int i = 0; i < sz; ++i) {
            ss << " " << abi_class_to_string(classes[i]);
        }
        ss << std::endl;
    }
#endif
    return sz;
#endif
}

bool is_memory_class(const Type *T) {
    ABIClass classes[MAX_ABI_CLASSES];
    return !abi_classify(T, classes);
}


} // namespace scopes

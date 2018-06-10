/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "specializer.hpp"
#include "types.hpp"
#include "any.hpp"
#include "error.hpp"
#include "gc.hpp"
#include "utils.hpp"
#include "argument.hpp"
#include "parameter.hpp"
#include "body.hpp"
#include "label.hpp"
#include "frame.hpp"
#include "stream_label.hpp"
#include "closure.hpp"
#include "anchor.hpp"
#include "gen_llvm.hpp"
#include "syntax.hpp"
#include "scope.hpp"
#include "stream_expr.hpp"
#include "source_file.hpp"
#include "lexerparser.hpp"
#include "list.hpp"
#include "timer.hpp"
#include "expander.hpp"
#include "profiler.hpp"
#include "execution.hpp"

#include "scopes.h"

#include "dyn_cast.inc"
#include "verify_tools.inc"

#include "linenoise-ng/include/linenoise.h"

#include <cmath>
#include <memory.h>

#include <ffi.h>

#pragma GCC diagnostic ignored "-Wzero-length-array"
#pragma GCC diagnostic ignored "-Wvla-extension"

#define SCOPE_INLINE_PARAMETERS 1

/*
one mechanism to optimize labels by skipping:

outer loop iterates labels that will definitely be used; the function entry
point for example is a definite label. (definite loop)

inner loop iterates possible labels. if a label folds, the next possible label
will be evaluated, until a definite label is encountered. the definite label
is then returned to the outer loop.

the outer loop then moves on to the next definite label and continues the task.
*/

namespace scopes {

static void apply_type_error(const Any &enter) {
    StyledString ss;
    ss.out << "don't know how to apply value of type " << enter.type;
    location_error(ss.str());
}

//------------------------------------------------------------------------------
// COMMON ERRORS
//------------------------------------------------------------------------------

void invalid_op2_types_error(const Type *A, const Type *B) {
    StyledString ss;
    ss.out << "invalid operand types " << A << " and " << B;
    location_error(ss.str());
}

//------------------------------------------------------------------------------
// OPERATOR TEMPLATES
//------------------------------------------------------------------------------

#define OP1_TEMPLATE(NAME, RTYPE, OP) \
    template<typename T> struct op_ ## NAME { \
        typedef RTYPE rtype; \
        static bool reductive() { return false; } \
        void operator()(void **srcptrs, void *destptr, size_t count) { \
            for (size_t i = 0; i < count; ++i) { \
                ((rtype *)destptr)[i] = op(((T *)(srcptrs[0]))[i]); \
            } \
        } \
        rtype op(T x) { \
            return OP; \
        } \
    };

#define OP2_TEMPLATE(NAME, RTYPE, OP) \
    template<typename T> struct op_ ## NAME { \
        typedef RTYPE rtype; \
        static bool reductive() { return false; } \
        void operator()(void **srcptrs, void *destptr, size_t count) { \
            for (size_t i = 0; i < count; ++i) { \
                ((rtype *)destptr)[i] = op(((T *)(srcptrs[0]))[i], ((T *)(srcptrs[1]))[i]); \
            } \
        } \
        rtype op(T a, T b) { \
            return OP; \
        } \
    };

template<typename T>
inline bool isnan(T f) {
    return f != f;
}

#define BOOL_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, a OP b)
#define BOOL_OF_TEMPLATE(NAME) OP2_TEMPLATE(NAME, bool, !isnan(a) && !isnan(b))
#define BOOL_UF_TEMPLATE(NAME) OP2_TEMPLATE(NAME, bool, isnan(a) || isnan(b))
#define BOOL_OF_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, !isnan(a) && !isnan(b) && (a OP b))
#define BOOL_UF_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, isnan(a) || isnan(b) || (a OP b))
#define IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, T, a OP b)
#define PFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, T, OP(a, b))
#define PUNOP_TEMPLATE(NAME, OP) OP1_TEMPLATE(NAME, T, OP(x))

template<typename RType>
struct select_op_return_type {
    const Type *operator ()(const Type *T) { return T; }
};

static const Type *bool_op_return_type(const Type *T) {
    T = storage_type(T);
    if (T->kind() == TK_Vector) {
        auto vi = cast<VectorType>(T);
        return Vector(TYPE_Bool, vi->count);
    } else {
        return TYPE_Bool;
    }
}

template<>
struct select_op_return_type<bool> {
    const Type *operator ()(const Type *T) {
        return bool_op_return_type(T);
    }
};

BOOL_IFXOP_TEMPLATE(Equal, ==)
BOOL_IFXOP_TEMPLATE(NotEqual, !=)
BOOL_IFXOP_TEMPLATE(Greater, >)
BOOL_IFXOP_TEMPLATE(GreaterEqual, >=)
BOOL_IFXOP_TEMPLATE(Less, <)
BOOL_IFXOP_TEMPLATE(LessEqual, <=)

BOOL_OF_IFXOP_TEMPLATE(OEqual, ==)
BOOL_OF_IFXOP_TEMPLATE(ONotEqual, !=)
BOOL_OF_IFXOP_TEMPLATE(OGreater, >)
BOOL_OF_IFXOP_TEMPLATE(OGreaterEqual, >=)
BOOL_OF_IFXOP_TEMPLATE(OLess, <)
BOOL_OF_IFXOP_TEMPLATE(OLessEqual, <=)
BOOL_OF_TEMPLATE(Ordered)

BOOL_UF_IFXOP_TEMPLATE(UEqual, ==)
BOOL_UF_IFXOP_TEMPLATE(UNotEqual, !=)
BOOL_UF_IFXOP_TEMPLATE(UGreater, >)
BOOL_UF_IFXOP_TEMPLATE(UGreaterEqual, >=)
BOOL_UF_IFXOP_TEMPLATE(ULess, <)
BOOL_UF_IFXOP_TEMPLATE(ULessEqual, <=)
BOOL_UF_TEMPLATE(Unordered)

IFXOP_TEMPLATE(Add, +)
IFXOP_TEMPLATE(Sub, -)
IFXOP_TEMPLATE(Mul, *)

IFXOP_TEMPLATE(SDiv, /)
IFXOP_TEMPLATE(UDiv, /)
IFXOP_TEMPLATE(SRem, %)
IFXOP_TEMPLATE(URem, %)

IFXOP_TEMPLATE(BAnd, &)
IFXOP_TEMPLATE(BOr, |)
IFXOP_TEMPLATE(BXor, ^)

IFXOP_TEMPLATE(Shl, <<)
IFXOP_TEMPLATE(LShr, >>)
IFXOP_TEMPLATE(AShr, >>)

IFXOP_TEMPLATE(FAdd, +)
IFXOP_TEMPLATE(FSub, -)
IFXOP_TEMPLATE(FMul, *)
IFXOP_TEMPLATE(FDiv, /)
PFXOP_TEMPLATE(FRem, std::fmod)

} namespace std {

#if 0
static bool abs(bool x) {
    return x;
}
#endif

} namespace scopes {

PUNOP_TEMPLATE(FAbs, std::abs)

template <typename T>
static T sgn(T val) {
    return T((T(0) < val) - (val < T(0)));
}

template<typename T>
static T radians(T x) {
    return x * T(M_PI / 180.0);
}

template<typename T>
static T degrees(T x) {
    return x * T(180.0 / M_PI);
}

template<typename T>
static T inversesqrt(T x) {
    return T(1.0) / std::sqrt(x);
}

template<typename T>
static T step(T edge, T x) {
    return T(x >= edge);
}

PUNOP_TEMPLATE(FSign, sgn)
PUNOP_TEMPLATE(SSign, sgn)
PUNOP_TEMPLATE(Radians, radians)
PUNOP_TEMPLATE(Degrees, degrees)
PUNOP_TEMPLATE(Sin, std::sin)
PUNOP_TEMPLATE(Cos, std::cos)
PUNOP_TEMPLATE(Tan, std::tan)
PUNOP_TEMPLATE(Asin, std::asin)
PUNOP_TEMPLATE(Acos, std::acos)
PUNOP_TEMPLATE(Atan, std::atan)
PFXOP_TEMPLATE(Atan2, std::atan2)
PUNOP_TEMPLATE(Exp, std::exp)
PUNOP_TEMPLATE(Log, std::log)
PUNOP_TEMPLATE(Exp2, std::exp2)
PUNOP_TEMPLATE(Log2, std::log2)
PUNOP_TEMPLATE(Sqrt, std::sqrt)
PUNOP_TEMPLATE(Trunc, std::trunc)
PUNOP_TEMPLATE(Floor, std::floor)
PUNOP_TEMPLATE(InverseSqrt, inversesqrt)
PFXOP_TEMPLATE(Pow, std::pow)
PFXOP_TEMPLATE(Step, step)

template<typename T> struct op_Cross {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        assert(count == 3);
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        auto ret = (rtype *)destptr;
        ret[0] = x[1] * y[2] - y[1] * x[2];
        ret[1] = x[2] * y[0] - y[2] * x[0];
        ret[2] = x[0] * y[1] - y[0] * x[1];
    }
};

template<typename T> struct op_FMix {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        auto a = (T *)(srcptrs[2]);
        auto ret = (rtype *)destptr;
        for (size_t i = 0; i < count; ++i) {
            ret[i] = x[i] * (T(1.0) - a[i]) + y[i] * a[i];
        }
    }
};

template<typename T> struct op_Normalize {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            r += x[i] * x[i];
        }
        r = (r == T(0))?T(1):(T(1)/std::sqrt(r));
        auto ret = (rtype *)destptr;
        for (size_t i = 0; i < count; ++i) {
            ret[i] = x[i] * r;
        }
    }
};

template<typename T> struct op_Length {
    typedef T rtype;
    static bool reductive() { return true; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            r += x[i] * x[i];
        }
        auto ret = (rtype *)destptr;
        ret[0] = std::sqrt(r);
    }
};

template<typename T> struct op_Distance {
    typedef T rtype;
    static bool reductive() { return true; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            T d = x[i] - y[i];
            r += d * d;
        }
        auto ret = (rtype *)destptr;
        ret[0] = std::sqrt(r);
    }
};

#undef BOOL_IFXOP_TEMPLATE
#undef BOOL_OF_TEMPLATE
#undef BOOL_UF_TEMPLATE
#undef BOOL_OF_IFXOP_TEMPLATE
#undef BOOL_UF_IFXOP_TEMPLATE
#undef IFXOP_TEMPLATE
#undef PFXOP_TEMPLATE

static void *aligned_alloc(size_t sz, size_t al) {
    assert(sz);
    assert(al);
    return reinterpret_cast<void *>(
        scopes::align(reinterpret_cast<uintptr_t>(tracked_malloc(sz + al - 1)), al));
}

static void *alloc_storage(const Type *T) {
    size_t sz = size_of(T);
    size_t al = align_of(T);
    return aligned_alloc(sz, al);
}

static void *copy_storage(const Type *T, void *ptr) {
    size_t sz = size_of(T);
    size_t al = align_of(T);
    void *destptr = aligned_alloc(sz, al);
    memcpy(destptr, ptr, sz);
    return destptr;
}

struct IntTypes_i {
    typedef bool i1;
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;
};
struct IntTypes_u {
    typedef bool i1;
    typedef uint8_t i8;
    typedef uint16_t i16;
    typedef uint32_t i32;
    typedef uint64_t i64;
};

template<typename IT, template<typename T> class OpT>
struct DispatchInteger {
    typedef typename OpT<int8_t>::rtype rtype;
    static bool reductive() { return OpT<int8_t>::reductive(); }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return select_op_return_type<rtype>{}(args[0].type);
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        size_t width = cast<IntegerType>(ET)->width;
        switch(width) {
        case 1: OpT<typename IT::i1>{}(srcptrs, destptr, count); break;
        case 8: OpT<typename IT::i8>{}(srcptrs, destptr, count); break;
        case 16: OpT<typename IT::i16>{}(srcptrs, destptr, count); break;
        case 32: OpT<typename IT::i32>{}(srcptrs, destptr, count); break;
        case 64: OpT<typename IT::i64>{}(srcptrs, destptr, count); break;
        default:
            StyledString ss;
            ss.out << "unsupported bitwidth (" << width << ") for integer operation";
            location_error(ss.str());
            break;
        };
    }
};

template<template<typename T> class OpT>
struct DispatchReal {
    typedef typename OpT<float>::rtype rtype;
    static bool reductive() { return OpT<float>::reductive(); }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return select_op_return_type<rtype>{}(args[0].type);
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        size_t width = cast<RealType>(ET)->width;
        switch(width) {
        case 32: OpT<float>{}(srcptrs, destptr, count); break;
        case 64: OpT<double>{}(srcptrs, destptr, count); break;
        default:
            StyledString ss;
            ss.out << "unsupported bitwidth (" << width << ") for float operation";
            location_error(ss.str());
            break;
        };
    }
};

struct DispatchSelect {
    static bool reductive() { return false; }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return args[1].type;
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        assert(numargs == 3);
        bool *cond = (bool *)srcptrs[0];
        void *x = srcptrs[1];
        void *y = srcptrs[2];
        const Type *Tx = storage_type(args[1].type);
        if (Tx->kind() == TK_Vector) {
            auto VT = cast<VectorType>(Tx);
            auto stride = VT->stride;
            for (size_t i = 0; i < count; ++i) {
                memcpy(VT->getelementptr(destptr, i),
                    VT->getelementptr((cond[i] ? x : y), i),
                    stride);
            }
        } else {
            assert(count == 1);
            auto sz = size_of(Tx);
            memcpy(destptr, (cond[0] ? x : y), sz);
        }
    }
};

template<typename DispatchT>
static Any apply_op(Any *args, size_t numargs) {
    auto ST = storage_type(args[0].type);
    size_t count;
    void *srcptrs[numargs];
    void *destptr;
    Any result = none;
    auto RT = DispatchT::return_type(args, numargs);
    const Type *ET = nullptr;
    if (ST->kind() == TK_Vector) {
        auto vi = cast<VectorType>(ST);
        count = vi->count;
        for (size_t i = 0; i < numargs; ++i) {
            srcptrs[i] = args[i].pointer;
        }
        if (DispatchT::reductive()) {
            result.type = vi->element_type;
            destptr = get_pointer(result.type, result);
        } else {
            destptr = alloc_storage(RT);
            result = Any::from_pointer(RT, destptr);
        }
        ET = storage_type(vi->element_type);
    } else {
        count = 1;
        for (size_t i = 0; i < numargs; ++i) {
            srcptrs[i] = get_pointer(args[i].type, args[i]);
        }
        result.type = RT;
        destptr = get_pointer(result.type, result);
        ET = ST;
    }
    DispatchT{}(ET, srcptrs, destptr, count, args, numargs);
    return result;
}

template<typename IT, template<typename T> class OpT >
static Any apply_integer_op(Any x) {
    Any args[] = { x };
    return apply_op< DispatchInteger<IT, OpT> >(args, 1);
}

template<typename IT, template<typename T> class OpT >
static Any apply_integer_op(Any a, Any b) {
    Any args[] = { a, b };
    return apply_op< DispatchInteger<IT, OpT> >(args, 2);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any x) {
    Any args[] = { x };
    return apply_op< DispatchReal<OpT> >(args, 1);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any a, Any b) {
    Any args[] = { a, b };
    return apply_op< DispatchReal<OpT> >(args, 2);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any a, Any b, Any c) {
    Any args[] = { a, b, c };
    return apply_op< DispatchReal<OpT> >(args, 3);
}

//------------------------------------------------------------------------------
// NORMALIZE
//------------------------------------------------------------------------------

// assuming that value is an elementary type that can be put in a vector
static Any smear(Any value, size_t count) {
    size_t sz = size_of(value.type);
    void *psrc = get_pointer(value.type, value);
    auto VT = cast<VectorType>(Vector(value.type, count));
    void *pdest = alloc_storage(VT);
    for (size_t i = 0; i < count; ++i) {
        void *p = VT->getelementptr(pdest, i);
        memcpy(p, psrc, sz);
    }
    return Any::from_pointer(VT, pdest);
}

#define B_ARITH_OPS() \
        IARITH_NUW_NSW_OPS(Add) \
        IARITH_NUW_NSW_OPS(Sub) \
        IARITH_NUW_NSW_OPS(Mul) \
        \
        IARITH_OP(SDiv, i) \
        IARITH_OP(UDiv, u) \
        IARITH_OP(SRem, i) \
        IARITH_OP(URem, u) \
        \
        IARITH_OP(BAnd, u) \
        IARITH_OP(BOr, u) \
        IARITH_OP(BXor, u) \
        \
        IARITH_OP(Shl, u) \
        IARITH_OP(LShr, u) \
        IARITH_OP(AShr, i) \
        \
        FARITH_OP(FAdd) \
        FARITH_OP(FSub) \
        FARITH_OP(FMul) \
        FARITH_OP(FDiv) \
        FARITH_OP(FRem) \
        \
        FUN_OP(FAbs) \
        \
        IUN_OP(SSign, i) \
        FUN_OP(FSign) \
        \
        FUN_OP(Radians) FUN_OP(Degrees) \
        FUN_OP(Sin) FUN_OP(Cos) FUN_OP(Tan) \
        FUN_OP(Asin) FUN_OP(Acos) FUN_OP(Atan) FARITH_OP(Atan2) \
        FUN_OP(Exp) FUN_OP(Log) FUN_OP(Exp2) FUN_OP(Log2) \
        FUN_OP(Trunc) FUN_OP(Floor) FARITH_OP(Step) \
        FARITH_OP(Pow) FUN_OP(Sqrt) FUN_OP(InverseSqrt) \
        \
        FTRI_OP(FMix)

struct Specializer {
    enum CLICmd {
        CmdNone,
        CmdSkip,
    };

    struct Trace {
        const Anchor *anchor;
        Symbol name;

        Trace(const Anchor *_anchor, Symbol _name) :
            anchor(_anchor), name(_name) {
            assert(anchor);
            }
    };

    typedef std::vector<Trace> Traces;

    StyledStream ss_cout;
    static Traces traceback;
    static int solve_refs;
    static bool enable_step_debugger;
    static CLICmd clicmd;
    Args tmp_args;
    Parameters tmp_params;
    Label::UserMap user_map;

    Specializer()
        : ss_cout(std::cout)
    {}

    typedef std::unordered_map<Parameter *, Args > MangleParamMap;
    typedef std::unordered_map<Label *, Label *> MangleLabelMap;

    static void mangle_remap_body(Label::UserMap &um, Label *ll, Label *entry, MangleLabelMap &lmap, MangleParamMap &pmap) {
        Any enter = entry->body.enter;
        Args &args = entry->body.args;
        Args &body = ll->body.args;
        if (enter.type == TYPE_Label) {
            auto it = lmap.find(enter.label);
            if (it != lmap.end()) {
                enter = it->second;
            }
        } else if (enter.type == TYPE_Parameter) {
            auto it = pmap.find(enter.parameter);
            if (it != pmap.end()) {
                enter = first(it->second).value;
            }
        }
        ll->flags = entry->flags & LF_Reentrant;
        ll->body.copy_traits_from(entry->body);
        ll->body.unset_optimized();
        ll->body.enter = enter;

        size_t lasti = (args.size() - 1);
        for (size_t i = 0; i < args.size(); ++i) {
            Argument arg = args[i];
            if (arg.value.type == TYPE_Label) {
                auto it = lmap.find(arg.value.label);
                if (it != lmap.end()) {
                    arg.value = it->second;
                }
            } else if (arg.value.type == TYPE_Parameter) {
                auto it = pmap.find(arg.value.parameter);
                if (it != pmap.end()) {
                    if ((i == lasti) && arg.value.parameter->is_vararg()) {
                        for (auto subit = it->second.begin(); subit != it->second.end(); ++subit) {
                            body.push_back(*subit);
                        }
                        continue;
                    } else {
                        arg.value = first(it->second).value;
                    }
                }
            }
            body.push_back(arg);
        }

        ll->insert_into_usermap(um);
    }

    enum MangleFlag {
        Mangle_Verbose = (1<<0),
    };

    static Label *mangle(Label::UserMap &um, Label *entry,
        std::vector<Parameter *> params, MangleParamMap &pmap, int verbose = 0) {
        MangleLabelMap lmap;

        std::vector<Label *> entry_scope;
        entry->build_scope(um, entry_scope);

        // remap entry point
        Label *le = Label::from(entry);
        le->set_parameters(params);
        // create new labels and map new parameters
        for (auto &&l : entry_scope) {
            Label *ll = Label::from(l);
            l->paired = ll;
            lmap.insert({l, ll});
            ll->params.reserve(l->params.size());
            for (auto &&param : l->params) {
                Parameter *pparam = Parameter::from(param);
                pmap.insert({ param, {Argument(Any(pparam))}});
                ll->append(pparam);
            }
        }

        // remap label bodies
        for (auto &&l : entry_scope) {
            Label *ll = l->paired;
            l->paired = nullptr;
            mangle_remap_body(um, ll, l, lmap, pmap);
        }
        mangle_remap_body(um, le, entry, lmap, pmap);

        if (verbose & Mangle_Verbose) {
        StyledStream ss(std::cout);
        ss << "IN[\n";
        stream_label(ss, entry, StreamLabelFormat::debug_single());
        for (auto && l : entry_scope) {
            stream_label(ss, l, StreamLabelFormat::debug_single());
        }
        ss << "]IN\n";
        ss << "OUT[\n";
        stream_label(ss, le, StreamLabelFormat::debug_single());
        for (auto && l : entry_scope) {
            auto it = lmap.find(l);
            stream_label(ss, it->second, StreamLabelFormat::debug_single());
        }
        ss << "]OUT\n";
        }

        return le;
    }

    // inlining the arguments of an untyped scope (including continuation)
    // folds arguments and types parameters
    // arguments are treated as follows:
    // TYPE_Unknown = type the parameter
    //      type as TYPE_Unknown = leave the parameter as-is
    // any other = inline the argument and remove the parameter
    static Label *fold_type_label(Label::UserMap &um, Label *label, const Args &args) {
        assert(!label->params.empty());

        MangleParamMap map;
        std::vector<Parameter *> newparams;
        size_t lasti = label->params.size() - 1;
        size_t srci = 0;
        for (size_t i = 0; i < label->params.size(); ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                assert(i == lasti);
                size_t ncount = args.size();
                if (srci < ncount) {
                    ncount -= srci;
                    Args vargs;
                    for (size_t k = 0; k < ncount; ++k) {
                        Argument value = args[srci + k];
                        if (value.value.type == TYPE_Unknown) {
                            Parameter *newparam = Parameter::from(param);
                            newparam->kind = PK_Regular;
                            newparam->type = value.value.typeref;
                            newparam->name = Symbol(SYM_Unnamed);
                            newparams.push_back(newparam);
                            vargs.push_back(Argument(value.key, newparam));
                        } else {
                            vargs.push_back(value);
                        }
                    }
                    map[param] = vargs;
                    srci = ncount;
                } else {
                    map[param] = {};
                }
            } else if (srci < args.size()) {
                Argument value = args[srci];
                if (is_unknown(value.value)) {
                    Parameter *newparam = Parameter::from(param);
                    if (is_typed(value.value)) {
                        if (newparam->is_typed()
                            && (newparam->type != value.value.typeref)) {
                            StyledString ss;
                            ss.out << "attempting to retype parameter of type "
                                << newparam->type << " as " << value.value.typeref;
                            location_error(ss.str());
                        } else {
                            newparam->type = value.value.typeref;
                        }
                    }
                    newparams.push_back(newparam);
                    map[param] = {Argument(value.key, newparam)};
                } else {
                    if (!srci) {
                        Parameter *newparam = Parameter::from(param);
                        newparam->type = TYPE_Nothing;
                        newparams.push_back(newparam);
                    }
                    map[param] = {value};
                }
                srci++;
            } else {
                map[param] = {Argument()};
                srci++;
            }
        }
        return mangle(um, label, newparams, map);//, Mangle_Verbose);
    }

    static void evaluate(Frame *frame, Argument arg, Args &dest, bool last_param = false) {
        if (arg.value.type == TYPE_Label) {
            // do not wrap labels in closures that have been solved
            if (!arg.value.label->is_template()) {
                dest.push_back(Argument(arg.key, arg.value.label));
            } else {
                Label *label = arg.value.label;
                assert(frame);
                Frame *top = frame->find_parent_frame(label);
                if (top) {
                    frame = top;
                } else if (label->body.scope_label) {
                    top = frame->find_parent_frame(label->body.scope_label);
                    if (top) {
                        frame = top;
                    } else {
                        if (label->is_debug()) {
                            StyledStream ss(std::cerr);
                            ss << "frame " <<  frame <<  ": can't find scope label for closure" << std::endl;
                            stream_label(ss, label, StreamLabelFormat::debug_single());
                        }
                    }
                } else {
                    if (label->is_debug()) {
                        StyledStream ss(std::cerr);
                        ss << "frame " <<  frame <<  ": label has no scope label for closure" << std::endl;
                        stream_label(ss, label, StreamLabelFormat::debug_single());
                    }
                }
                dest.push_back(Argument(arg.key, Closure::from(label, frame)));
            }
        } else if (arg.value.type == TYPE_Parameter
            && arg.value.parameter->label
            && arg.value.parameter->label->is_template()) {
            auto param = arg.value.parameter;
            frame = frame->find_parent_frame(param->label);
            if (!frame) {
                StyledString ss;
                ss.out << "parameter " << param << " is unbound";
                location_error(ss.str());
            }
            // special situation: we're forwarding varargs, but assigning
            // it to a new argument key; since keys can only be mapped to
            // individual varargs, and must not be duplicated, we have these
            // options to resolve the conflict:
            // 1. the vararg keys override the new explicit key; this was the
            //    old behavior and made it possible for implicit vararg return
            //    values to override explicit reassignments, which was
            //    always surprising and unwanted, i.e. a bug.
            // 2. re-assign only the first vararg key, keeping remaining keys
            //    as they are.
            // 3. produce a compiler error when an explicit key is set, but
            //    the vararg set is larger than 1.
            // 4. treat a keyed argument in last place like any previous argument,
            //    causing only a single vararg result to be forwarded.
            // we use option 4, as it is most consistent with existing behavior,
            // and seems to be the least surprising choice.
            if (last_param && param->is_vararg() && !arg.is_keyed()) {
                // forward as-is, with keys
                for (size_t i = (size_t)param->index; i < frame->args.size(); ++i) {
                    dest.push_back(frame->args[i]);
                }
            } else if ((size_t)param->index < frame->args.size()) {
                auto &&srcarg = frame->args[param->index];
                // when forwarding a vararg and the arg is not re-keyed,
                // forward the vararg key as well.
                if (param->is_vararg() && !arg.is_keyed()) {
                    dest.push_back(srcarg);
                } else {
                    dest.push_back(Argument(arg.key, srcarg.value));
                }
            } else {
                if (!param->is_vararg()) {
    #if SCOPES_DEBUG_CODEGEN
                    {
                        StyledStream ss;
                        ss << frame << " " << frame->label;
                        for (size_t i = 0; i < frame->args.size(); ++i) {
                            ss << " " << frame->args[i];
                        }
                        ss << std::endl;
                    }
    #endif
                    StyledString ss;
                    ss.out << "parameter " << param << " is out of bounds ("
                        << param->index << " >= " << (int)frame->args.size() << ")";
                    location_error(ss.str());
                }
                dest.push_back(Argument(arg.key, none));
            }
        } else {
            dest.push_back(arg);
        }
    }

    void evaluate_body(Frame *frame, Body &dest, const Body &source) {
        auto &&args = source.args;
        Args &body = dest.args;
        tmp_args.clear();
        dest.copy_traits_from(source);
        evaluate(frame, source.enter, tmp_args);
        dest.enter = first(tmp_args).value;
        body.clear();

        size_t lasti = (args.size() - 1);
        for (size_t i = 0; i < args.size(); ++i) {
            evaluate(frame, args[i], body, (i == lasti));
        }
    }

    static void map_constant_arguments(Frame *frame, Label *label, const Args &args) {
        size_t lasti = label->params.size() - 1;
        size_t srci = 0;
        for (size_t i = 0; i < label->params.size(); ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                assert(i == lasti);
                size_t ncount = args.size();
                while (srci < ncount) {
                    Argument value = args[srci];
                    assert(!is_unknown(value.value));
                    frame->args.push_back(value);
                    srci++;
                }
            } else if (srci < args.size()) {
                Argument value = args[srci];
                assert(!is_unknown(value.value));
                frame->args.push_back(value);
                srci++;
            } else {
                frame->args.push_back(none);
                srci++;
            }
        }
    }

    Label *fold_type_label_single(Frame *parent, Label *label, const Args &args) {
        return fold_type_label_single_frame(parent, label, args)->get_instance();
    }

    void stream_arg_types(StyledStream &ss, const Args &args) {
        if (args.size() <= 1) {
            ss << TYPE_Void << " ";
            return;
        }
        for (int i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            assert(is_unknown(arg.value));
            ss << arg.value.typeref << " ";
        }
    }

    // inlining the arguments of an untyped scope (including continuation)
    // folds arguments and types parameters
    // arguments are treated as follows:
    // TYPE_Unknown = type the parameter
    //      type as TYPE_Unknown = leave the parameter as-is
    // any other = inline the argument and remove the parameter
    Frame *fold_type_label_single_frame(Frame *parent, Label *label, const Args &args) {
        assert(parent);
        assert(label);
        assert(!label->body.is_complete());
        size_t loop_count = 0;
        if ((parent != Frame::root) && (parent->label == label)) {
            Frame *top = parent;
            parent = top->parent;
            loop_count = top->loop_count + 1;
            if (loop_count > SCOPES_MAX_RECURSIONS) {
                StyledString ss;
                ss.out << "maximum number of recursions exceeded during"
                " compile time evaluation (" << SCOPES_MAX_RECURSIONS << ")."
                " Use 'unconst' to prevent constant propagation.";
                location_error(ss.str());
            }
        }

        const Anchor *caller_anchor = parent->label?parent->label->body.anchor:label->anchor;

        tmp_args.clear();

        size_t numparams = label->params.size();
        size_t numargs = args.size();

        size_t lasti = numparams - 1;
        size_t srci = 0;
        for (size_t i = 0; i < numparams; ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                while (srci < numargs) {
                    tmp_args.push_back(args[srci]);
                    srci++;
                }
            } else {
                tmp_args.push_back((srci < numargs)?(args[srci]):none);
                srci++;
            }
        }

        if (label->is_debug()) {
            StyledStream ss(std::cerr);
            ss << "frame " << parent <<  ": instantiating label" << std::endl;
            stream_label(ss, label, StreamLabelFormat::debug_single());
            ss << "with key ";
            for (size_t i = 0; i < tmp_args.size(); ++i) {
                if (is_unknown(tmp_args[i].value)) {
                    ss << "?" << tmp_args[i].value.typeref << " ";
                } else {
                    ss << tmp_args[i] << " ";
                }
            }
            if (parent->inline_merge) {
                ss << "and merge inlined";
            }
            ss << std::endl;
        }

        Frame::ArgsKey la;
        la.label = label;
        la.args = tmp_args;
        {
            Frame *result = parent->find_frame(la);
            if (label->is_debug()) {
                StyledStream ss(std::cerr);
                if (result) {
                    ss << " and the label already exists (frame " << result << ")";
                    if (!result->get_instance()->body.is_complete()) {
                        ss << " but is incomplete:" << std::endl;
                        stream_label(ss, result->get_instance(), StreamLabelFormat::debug_single());
                    } else {
                        ss << std::endl;
                    }
                } else {
                    ss << " and the label is new" << std::endl;
                }
            }
            if (result)
                return result;
        }

        if (label->is_merge() && !parent->inline_merge) {
            for (int i = 1; i < tmp_args.size(); ++i) {
                // verify that all keys are unknown
                auto &&val = tmp_args[i].value;
                if (is_unknown(val)) {
                } else if (val.is_const()) {
                    StyledString ss;
                    ss.out << "attempting to return from branch, but returned argument #"
                        << i << " of type " << val.type << " is constant";
                    set_active_anchor(caller_anchor);
                    location_error(ss.str());
                } else {
                    // should not happen
                    assert(false);
                }
            }
            Frame::ArgsKey key;
            Frame *result_frame = parent->find_any_frame(label, key);
            if (result_frame && !(la == key)) {
                if ((key.args.size() > 1) && !is_unknown(key.args[0].value)) {
                    // ugh, label has been inlined before :C
                    location_message(key.label->anchor, String::from("internal error: first inlined here"));
                } else {
                    StyledString ss;
                    ss.out << "previously returned ";
                    stream_arg_types(ss.out, key.args);
                    location_message(key.label->anchor, ss.str());
                }
                {
                    StyledString ss;
                    ss.out << "cannot merge conditional branches returning ";
                    stream_arg_types(ss.out, key.args);
                    ss.out << "and ";
                    stream_arg_types(ss.out, la.args);
                    set_active_anchor(caller_anchor);
                    location_error(ss.str());
                }
            }
        }

        tmp_params.clear();

        srci = 0;
        for (size_t i = 0; i < label->params.size(); ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                assert(i == lasti);
                size_t ncount = args.size();
                while (srci < ncount) {
                    auto &&value = tmp_args[srci];
                    if (is_unknown(value.value)) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        newparam->kind = PK_Regular;
                        newparam->type = value.value.typeref;
                        newparam->name = Symbol(SYM_Unnamed);
                        tmp_args[srci] = Argument(value.key, newparam);
                    }
                    srci++;
                }
            } else {
                if (srci < args.size()) {
                    auto &&value = tmp_args[srci];
                    if (is_unknown(value.value)) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        if (is_typed(value.value)) {
                            if (newparam->is_typed()
                                && (newparam->type != value.value.typeref)) {
                                StyledString ss;
                                ss.out << "attempting to retype parameter of type "
                                    << newparam->type << " as " << value.value.typeref;
                                location_error(ss.str());
                            } else {
                                newparam->type = value.value.typeref;
                            }
                        }
                        tmp_args[srci] = Argument(value.key, newparam);
                    } else if (!srci) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        newparam->type = TYPE_Nothing;
                    }
                }
                srci++;
            }
        }

        Label *newlabel = Label::from(label);
        newlabel->set_parameters(tmp_params);
        if (parent->inline_merge) {
            newlabel->unset_merge();
        }

        Frame *frame = Frame::from(parent, label, newlabel, loop_count);
        frame->args = tmp_args;
        newlabel->frame = frame;

        if (label->is_debug()) {
            StyledStream ss(std::cerr);
            ss << "the label is contained in frame " << frame << std::endl;
        }

        parent->insert_frame(la, frame);

        evaluate_body(frame, newlabel->body, label->body);

        return frame;
    }

    // inlining the continuation of a branch label without arguments
    void verify_branch_continuation(const Closure *closure) {
        if (closure->label->is_inline())
            return;
        StyledString ss;
        ss.out << "branch destination must be inline" << std::endl;
        location_error(ss.str());
    }

    Any fold_type_return(Any dest, const Type *return_label) {
    repeat:
        //ss_cout << "type_return: " << dest << std::endl;
        if (dest.type == TYPE_Parameter) {
            Parameter *param = dest.parameter;
            if (param->is_none()) {
                location_error(String::from("attempting to type return continuation of non-returning label"));
            } else if (!param->is_typed()) {
                param->type = return_label;
                param->anchor = get_active_anchor();
            } else {
                const Type *ptype = param->type;
                if (return_label != ptype) {
                    // try to get a fit by unconsting types
                    return_label = cast<ReturnLabelType>(return_label)->to_unconst();
                    ptype = cast<ReturnLabelType>(ptype)->to_unconst();
                    if (return_label == ptype) {
                        param->type = return_label;
                        param->anchor = get_active_anchor();
                    } else {
                        {
                            StyledStream cerr(std::cerr);
                            cerr << param->anchor << " first typed here as " << ptype << std::endl;
                            param->anchor->stream_source_line(cerr);
                        }
                        {
                            StyledString ss;
                            ss.out << "attempting to retype return continuation as " << return_label;
                            location_error(ss.str());
                        }
                    }
                }
            }
        } else if (dest.type == TYPE_Closure) {
            assert(return_label->kind() == TK_ReturnLabel);
            const ReturnLabelType *rlt = cast<ReturnLabelType>(return_label);
            if (!rlt->is_returning()) {
                return none;
            }
            auto &&values = rlt->values;
            auto enter_frame = dest.closure->frame;
            auto enter_label = dest.closure->label;
            Args args;
            args.reserve(values.size() + 1);
            args = { Argument(untyped()) };
            for (size_t i = 0; i < values.size(); ++i) {
                args.push_back(values[i]);
            }
            Label *newl = fold_type_label_single(enter_frame, enter_label, args);
            if (is_jumping(newl)
                && !newl->is_important()
                && (is_calling_continuation(newl) || is_calling_closure(newl))
                && matches_arg_count(newl, values.size())
                && forwards_all_args(newl)) {
                dest = newl->body.enter;
                goto repeat;
            } else {
                dest = newl;
            }
        } else if (dest.type == TYPE_Label) {
            auto TR = dest.label->get_params_as_return_label_type();
            if (return_label != TR) {
                {
                    StyledStream cerr(std::cerr);
                    cerr << dest.label->anchor << " typed as " << TR << std::endl;
                    dest.label->anchor->stream_source_line(cerr);
                }
                {
                    StyledString ss;
                    ss.out << "attempting to retype label as " << return_label;
                    location_error(ss.str());
                }
            }
        } else {
            apply_type_error(dest);
        }
        return dest;
    }

    static void verify_integer_ops(Any x) {
        verify_integer_vector(storage_type(x.indirect_type()));
    }

    static void verify_real_ops(Any x) {
        verify_real_vector(storage_type(x.indirect_type()));
    }

    static void verify_integer_ops(Any a, Any b) {
        verify_integer_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
    }

    static void verify_real_ops(Any a, Any b) {
        verify_real_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
    }

    static void verify_real_ops(Any a, Any b, Any c) {
        verify_real_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
        verify(a.indirect_type(), c.indirect_type());
    }

    static bool has_keyed_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].key != SYM_Unnamed)
                return true;
        }
        return false;
    }

    static void verify_no_keyed_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].key != SYM_Unnamed) {
                location_error(String::from("unexpected keyed argument"));
            }
        }

    }

    static bool is_jumping(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Nothing;
    }

    static bool is_branching(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Builtin && enter.builtin == FN_Branch;
    }

    static bool is_continuing_to_label(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Label;
    }

    static bool is_continuing_to_parameter(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Parameter;
    }

    static bool is_continuing_to_closure(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Closure;
    }

    static bool is_calling_closure(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Closure;
    }

    static bool is_calling_label(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Label;
    }

    static bool is_return_parameter(Any val) {
        return (val.type == TYPE_Parameter) && (val.parameter->index == 0);
    }

    static bool is_calling_continuation(Label *l) {
        auto &&enter = l->body.enter;
        return (enter.type == TYPE_Parameter) && (enter.parameter->index == 0);
    }

    static bool is_calling_builtin(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Builtin;
    }

    static bool is_calling_callable(Label *l) {
        if (l->body.is_rawcall())
            return false;
        auto &&enter = l->body.enter;
        const Type *T = enter.indirect_type();
        Any value = none;
        return T->lookup_call_handler(value);
    }

    static bool is_calling_label_macro(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_LabelMacro;
    }

    static bool is_calling_function(Label *l) {
        auto &&enter = l->body.enter;
        return is_function_pointer(enter.indirect_type());
    }

    static bool all_params_typed(Label *l) {
        auto &&params = l->params;
        for (size_t i = 1; i < params.size(); ++i) {
            if (!params[i]->is_typed())
                return false;
        }
        return true;
    }

    static size_t find_untyped_arg(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if ((args[i].value.type == TYPE_Parameter)
                && (args[i].value.parameter->index != 0)
                && (!args[i].value.parameter->is_typed()))
                return i;
        }
        return 0;
    }

    static bool all_args_typed(Label *l) {
        return !find_untyped_arg(l);
    }

    static bool all_args_constant(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (!args[i].value.is_const())
                return false;
        }
        return true;
    }

    static bool has_foldable_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].value.is_const())
                return true;
            else if (is_return_parameter(args[i].value))
                return true;
        }
        return false;
    }

    static bool is_called_by(Label *callee, Label *caller) {
        auto &&enter = caller->body.enter;
        return (enter.type == TYPE_Label) && (enter.label == callee);
    }

    static bool is_called_by(Parameter *callee, Label *caller) {
        auto &&enter = caller->body.enter;
        return (enter.type == TYPE_Parameter) && (enter.parameter == callee);
    }

    static bool is_continuing_from(Label *callee, Label *caller) {
        auto &&args = caller->body.args;
        assert(!args.empty());
        return (args[0].value.type == TYPE_Label) && (args[0].value.label == callee);
    }

    static bool is_continuing_from(Parameter *callee, Label *caller) {
        auto &&args = caller->body.args;
        assert(!args.empty());
        return (args[0].value.type == TYPE_Parameter) && (args[0].value.parameter == callee);
    }

    void verify_function_argument_signature(const FunctionType *fi, Label *l) {
        auto &&args = l->body.args;
        verify_function_argument_count(fi, args.size() - 1);

        size_t fargcount = fi->argument_types.size();
        for (size_t i = 1; i < args.size(); ++i) {
            Argument &arg = args[i];
            size_t k = i - 1;
            const Type *argT = arg.value.indirect_type();
            if (k < fargcount) {
                const Type *ft = fi->argument_types[k];
                const Type *A = storage_type(argT);
                const Type *B = storage_type(ft);
                if (A == B)
                    continue;
                if ((A->kind() == TK_Pointer) && (B->kind() == TK_Pointer)) {
                    auto pa = cast<PointerType>(A);
                    auto pb = cast<PointerType>(B);
                    if ((pa->element_type == pb->element_type)
                        && (pa->flags == pb->flags)
                        && (pb->storage_class == SYM_Unnamed))
                        continue;
                }
                StyledString ss;
                ss.out << "argument of type " << ft << " expected, got " << argT;
                location_error(ss.str());
            }
        }
    }

    void fold_pure_function_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding pure function call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;

        const FunctionType *fi = extract_function_type(enter.type);

        verify_function_argument_signature(fi, l);

        assert(!args.empty());
        Any result = none;

        if (fi->flags & FF_Variadic) {
            // convert C types
            size_t argcount = args.size() - 1;
            Args cargs;
            cargs.reserve(argcount);
            for (size_t i = 0; i < argcount; ++i) {
                Argument &srcarg = args[i + 1];
                if (i >= fi->argument_types.size()) {
                    if (srcarg.value.type == TYPE_F32) {
                        cargs.push_back(Argument(srcarg.key, (double)srcarg.value.f32));
                        continue;
                    }
                }
                cargs.push_back(srcarg);
            }
            result = run_ffi_function(enter, &cargs[0], cargs.size());
        } else {
            result = run_ffi_function(enter, &args[1], args.size() - 1);
        }

        enter = args[0].value;
        args = { Argument() };
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        if (rlt->return_type != TYPE_Void) {
            if (isa<TupleType>(rlt->return_type)) {
                // unpack
                auto ti = cast<TupleType>(rlt->return_type);
                size_t count = ti->types.size();
                for (size_t i = 0; i < count; ++i) {
                    args.push_back(Argument(ti->unpack(result.pointer, i)));
                }
            } else {
                args.push_back(Argument(result));
            }
        }
    }

    void solve_keyed_args(Label *l) {
        Label *enter = l->get_closure_enter()->label;

        auto &&args = l->body.args;
        assert(!args.empty());
        Args newargs;
        newargs.reserve(args.size());
        newargs.push_back(args[0]);
        Parameter *vaparam = nullptr;
        if (!enter->params.empty() && enter->params.back()->is_vararg()) {
            vaparam = enter->params.back();
        }
        std::vector<bool> mapped;
        mapped.reserve(args.size());
        mapped.push_back(true);
        size_t next_index = 1;
        for (size_t i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            if (arg.key == SYM_Unnamed) {
                while ((next_index < mapped.size()) && mapped[next_index])
                    next_index++;
                while (mapped.size() <= next_index) {
                    mapped.push_back(false);
                    newargs.push_back(none);
                }
                mapped[next_index] = true;
                newargs[next_index] = arg;
                next_index++;
            } else {
                auto param = enter->get_param_by_name(arg.key);
                size_t index = -1;
                if (param && (param != vaparam)) {
                    while (mapped.size() <= (size_t)param->index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    if (mapped[param->index]) {
                        StyledString ss;
                        ss.out << "duplicate binding to parameter " << arg.key;
                        location_error(ss.str());
                    }
                    index = param->index;
                } else if (vaparam) {
                    while (mapped.size() < (size_t)vaparam->index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    index = newargs.size();
                    mapped.push_back(false);
                    newargs.push_back(none);
                    newargs[index].key = arg.key;
                } else {
                    // no such parameter, map like regular parameter
                    while ((next_index < mapped.size()) && mapped[next_index])
                        next_index++;
                    while (mapped.size() <= next_index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    index = next_index;
                    newargs[index].key = SYM_Unnamed;
                    next_index++;
                }
                mapped[index] = true;
                newargs[index].value = arg.value;
            }
        }
        args = newargs;
    }

    bool is_indirect_closure_type(const Type *T) {
        if (is_opaque(T)) return false;
        if (T == TYPE_Closure) return true;
        T = storage_type(T);
        const Type *ST = storage_type(TYPE_Closure);
        if (T == ST) return true;
        // TODO: detect closures in aggregate types
        return false;
    }

    void validate_label_return_types(Label *l) {
        assert(!l->is_basic_block_like());
        assert(l->is_return_param_typed());
        const ReturnLabelType *rlt = cast<ReturnLabelType>(l->params[0]->type);
        for (size_t i = 0; i < rlt->values.size(); ++i) {
            auto &&val = rlt->values[i].value;
            bool needs_inline = false;
            const char *name = nullptr;
            const Type *displayT = nullptr;
            #if 0
            if (is_indirect_closure_type(val.type)) {
                needs_inline = true;
                name = "closure";
                displayT = val.type;
            } else
            #endif
            if (is_unknown(val)) {
                auto T = val.typeref;
                if (!is_opaque(T)) {
                    T = storage_type(T);
                    if (T->kind() == TK_Pointer) {
                        auto pt = cast<PointerType>(T);
                        if (pt->storage_class != SYM_Unnamed) {
                            needs_inline = true;
                            name = "pointer";
                            displayT = val.typeref;
                        }
                    }
                }
            }
            if (needs_inline) {
                StyledString ss;
                set_active_anchor(l->anchor);
                ss.out << "return argument #" << i << " is of non-returnable " << name << " type "
                    << displayT << " but function is not being inlined" << std::endl;
                location_error(ss.str());
            }
        }
    }

    bool frame_args_match_keys(const Args &args, const Args &keys) const {
        if (args.size() != keys.size())
            return false;
        for (size_t i = 1; i < keys.size(); ++i) {
            auto &&arg = args[i].value;
            auto &&key = keys[i].value;
            if (is_unknown(key)
                && !arg.is_const()
                && (arg.parameter->type == key.typeref))
                continue;
            if (args[i].value != keys[i].value)
                return false;
        }
        return true;
    }

#if 0
    bool has_single_user(Label *l) {
        assert(!user_map.empty());
        auto it = user_map.label_map.find(l);
        if (it == user_map.label_map.end()) {
            // possibly indirectly referenced through a closure, scope, type
            return false;
        }
        assert(it != user_map.label_map.end());
        auto &&users = it->second;
        if (users.size() >= 1)
            return false;
        return true;
    }
#endif

    const Type *fold_closure_call(Label *l, bool &recursive, bool &inlined) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding & typing arguments in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        assert(enter.type == TYPE_Closure);
        Frame *enter_frame = enter.closure->frame;
        Label *enter_label = enter.closure->label;

        bool inline_const = (!enter_label->is_merge()) || enter_frame->inline_merge;

        bool want_inline = enter_label->is_inline();
            //&& enter_label->is_basic_block_like();
            //|| (enter_label->is_basic_block_like() && has_single_user(enter_label))

        // inline constant arguments
        Args callargs;
        Args keys;
        auto &&args = l->body.args;
#if SCOPE_INLINE_PARAMETERS
        if (want_inline && inline_const) {
            callargs.push_back(none);
            keys.push_back(args[0]);
            for (size_t i = 1; i < args.size(); ++i) {
                keys.push_back(args[i]);
            }
            if (!enter_label->is_basic_block_like()) {
                on_label_specialized(enter_label);
            }
        } else
#endif
        {
            callargs.push_back(args[0]);
            keys.push_back(Argument(untyped()));
            for (size_t i = 1; i < args.size(); ++i) {
                auto &&arg = args[i];
                if (arg.value.is_const() && inline_const) {
                    keys.push_back(Argument(arg.key,
                        unknown_of(arg.value.indirect_type())));
                    callargs.push_back(arg);
                    //keys.push_back(arg);
                } else if (is_return_parameter(arg.value)) {
                    keys.push_back(arg);
                } else {
                    keys.push_back(Argument(arg.key,
                        unknown_of(arg.value.indirect_type())));
                    callargs.push_back(arg);
                }
            }
            #if !SCOPE_INLINE_PARAMETERS
            if (enter_label->is_inline()) {
                callargs[0] = none;
                keys[0] = args[0];
            }
            #endif
        }

#if 0
        bool is_function_entry = !enter_label->is_basic_block_like();
#endif

        Frame *newf = fold_type_label_single_frame(
            enter_frame, enter_label, keys);
        Label *newl = newf->get_instance();

#if 0
        if (newf == enter_frame) {
            location_error(String::from("label or function forms an infinite but empty loop"));
        }
#endif

        // labels points do not need to be entered
        if (newl->is_basic_block_like()) {
            enter = newl;
            args = callargs;
            //clear_continuation_arg(l);
            return nullptr;
        } else {
            // newl is a function

            // we need to solve body, return type and reentrant flags for the
            // section that follows
            normalize_function(newf);

            // function is done, but not typed
            if (!newl->is_return_param_typed()) {
                // two possible reasons for this:
                // 2. all return paths are unreachable, because the function
                //    never returns.
                //    but this is not the case, because the function has not been
                //    typed yet as not returning.

                // 1. recursion - entry label has already been
                //    processed, but not exited yet, so we don't have the
                //    continuation type yet.
                // try again when all outstanding branches have been processed
                recursive = true;
                return nullptr;
            } else {
                validate_label_return_types(newl);
            }

            const Type *rtype = newl->get_return_type();
            if (is_empty_function(newl)) {
    #if 1
                if (enable_step_debugger) {
                    StyledStream ss(std::cerr);
                    ss << "folding call to empty function:" << std::endl;
                    stream_label(ss, newl, StreamLabelFormat::debug_scope());
                }
    #endif
                // function performs no work, fold
                enter = args[0].value;
                args = { none };
                const ReturnLabelType *rlt = cast<ReturnLabelType>(rtype);
                auto &&values = rlt->values;
                for (size_t i = 0; i < values.size(); ++i) {
                    auto &&arg = values[i];
                    assert(!is_unknown(arg.value));
                    args.push_back(arg);
                }
            } else {
                enter = newl;
                args = callargs;
                auto nrl = NoReturnLabel();
                if (rtype == nrl) {
                    rtype = nullptr;
                }

#if 0
                if (newl->is_inline()) {
                    /*
                    mangle solved function to include explicit return continuation.

                    problem with this method:
                    if closures escape the function, the closure's frames
                    still map template parameters to labels used before the mangling.
                    */
                    Parameter *cont_param = newl->params[0];
                    const Type *cont_type = cont_param->type;
                    assert(isa<ReturnLabelType>(cont_type));
                    Any cont = none;
                    if (cont_type != nrl) {
                        cont = fold_type_return(args[0].value, cont_type);
                        assert(cont.type != TYPE_Closure);
                    }
                    keys.clear();
                    keys.push_back(cont);
                    for (size_t i = 1; i < callargs.size(); ++i) {
                        keys.push_back(args[i]);
                    }
                    args = { none };
                    std::unordered_set<Label *> visited;
                    std::vector<Label *> labels;
                    newl->build_reachable(visited, &labels);
                    Label::UserMap um;
                    for (auto it = labels.begin(); it != labels.end(); ++it) {
                        (*it)->insert_into_usermap(um);
                    }
                    Label *newll = fold_type_label(um, newl, keys);
                    enter = newll;
                    l->body.set_complete();
                    fold_useless_labels(l);
                    inlined = true;
                    return nullptr;
                }
#endif
            }
            return rtype;
        }
    }

    // returns true if the builtin folds regardless of whether the arguments are
    // constant
    bool builtin_always_folds(Builtin builtin) {
        switch(builtin.value()) {
        case FN_AnyWrap:
        case KW_SyntaxExtend:
        case FN_TypeOf:
        case FN_NullOf:
        case FN_IsConstant:
        case FN_VaCountOf:
        case FN_VaKeys:
        case FN_VaKey:
        case FN_VaValues:
        case FN_VaAt:
        case FN_Location:
        case FN_Dump:
        case FN_ExternNew:
        case FN_ExternSymbol:
        case FN_TupleType:
        case FN_UnionType:
        case FN_StaticAlloc:
        case KW_Forward:
            return true;
        default: return false;
        }
    }

    bool builtin_has_keyed_args(Builtin builtin) {
        switch(builtin.value()) {
        case FN_VaCountOf:
        case FN_VaKeys:
        case FN_VaValues:
        case FN_VaAt:
        case FN_Dump:
        case FN_ExternNew:
        case FN_ReturnLabelType:
        case FN_ScopeOf:
        case FN_TupleType:
        case FN_UnionType:
        case FN_Sample:
        case FN_ImageQuerySize:
            return true;
        default: return false;
        }
    }

    bool builtin_never_folds(Builtin builtin) {
        switch(builtin.value()) {
        case FN_Bitcast:
        case FN_Unconst:
        case FN_Undef:
        case FN_Alloca:
        case FN_AllocaExceptionPad:
        case FN_AllocaArray:
        case FN_Malloc:
        case FN_MallocArray:
        case SFXFN_Unreachable:
        case SFXFN_Discard:
        case FN_VolatileStore:
        case FN_Store:
        case FN_VolatileLoad:
        case FN_Load:
        case FN_Sample:
        case FN_ImageRead:
        case FN_ImageQuerySize:
        case FN_ImageQueryLod:
        case FN_ImageQueryLevels:
        case FN_ImageQuerySamples:
        case FN_GetElementPtr:
        case SFXFN_ExecutionMode:
        case OP_Tertiary:
            return true;
        default: return false;
        }
    }

    void verify_readable(const Type *T) {
        auto pi = cast<PointerType>(T);
        if (!pi->is_readable()) {
            StyledString ss;
            ss.out << "can not load value from address of type " << T
                << " because the target is non-readable";
            location_error(ss.str());
        }
    }

    void verify_writable(const Type *T) {
        auto pi = cast<PointerType>(T);
        if (!pi->is_writable()) {
            StyledString ss;
            ss.out << "can not store value at address of type " << T
                << " because the target is non-writable";
            location_error(ss.str());
        }
    }

    // reduce typekind to compatible
    static TypeKind canonical_typekind(TypeKind k) {
        if (k == TK_Real)
            return TK_Integer;
        return k;
    }

#define CHECKARGS(MINARGS, MAXARGS) \
    checkargs<MINARGS, MAXARGS>(args.size())

#define RETARGTYPES(...) \
    { \
        const Type *retargtypes[] = { __VA_ARGS__ }; \
        size_t _count = (sizeof(retargtypes) / sizeof(const Type *)); \
        retvalues.reserve(_count); \
        for (size_t _i = 0; _i < _count; ++_i) { \
            retvalues.push_back(unknown_of(retargtypes[_i])); \
        } \
    }
#define RETARGS(...) \
    retvalues = { __VA_ARGS__ }; \
    return true;

    // returns true if the call can be eliminated
    bool values_from_builtin_call(Label *l, Args &retvalues) {
        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        assert(enter.type == TYPE_Builtin);
        switch(enter.builtin.value()) {
        case FN_Sample: {
            CHECKARGS(2, -1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            RETARGTYPES(it->type);
        } break;
        case FN_ImageQuerySize: {
            CHECKARGS(1, -1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            int comps = 0;
            switch(it->dim.value()) {
            case SYM_SPIRV_Dim1D:
            case SYM_SPIRV_DimBuffer:
                comps = 1;
                break;
            case SYM_SPIRV_Dim2D:
            case SYM_SPIRV_DimCube:
            case SYM_SPIRV_DimRect:
            case SYM_SPIRV_DimSubpassData:
                comps = 2;
                break;
            case SYM_SPIRV_Dim3D:
                comps = 3;
                break;
            default:
                location_error(String::from("unsupported dimensionality"));
                break;
            }
            if (it->arrayed) {
                comps++;
            }
            if (comps == 1) {
                RETARGTYPES(TYPE_I32);
            } else {
                RETARGTYPES(Vector(TYPE_I32, comps));
            }
        } break;
        case FN_ImageQueryLod: {
            CHECKARGS(2, 2);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            RETARGTYPES(Vector(TYPE_F32, 2));
        } break;
        case FN_ImageQueryLevels:
        case FN_ImageQuerySamples: {
            CHECKARGS(1, 1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            RETARGTYPES(TYPE_I32);
        } break;
        case FN_ImageRead: {
            CHECKARGS(2, 2);
            auto ST = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            RETARGTYPES(it->type);
        } break;
        case SFXFN_ExecutionMode: {
            CHECKARGS(1, 4);
            args[1].value.verify(TYPE_Symbol);
            switch(args[1].value.symbol.value()) {
            #define T(NAME) \
                case SYM_SPIRV_ExecutionMode ## NAME: break;
                B_SPIRV_EXECUTION_MODE()
            #undef T
                default:
                    location_error(String::from("unsupported execution mode"));
                    break;
            }
            for (size_t i = 2; i < args.size(); ++i) {
                cast_number<int>(args[i].value);
            }
            RETARGTYPES();
        } break;
        case OP_Tertiary: {
            CHECKARGS(3, 3);
            auto &&cond = args[1].value;
            if (cond.is_const() &&
                ((cond.type == TYPE_Bool)
                    || (args[2].value.is_const() && args[3].value.is_const()))) {
                if (cond.type == TYPE_Bool) {
                    if (cond.i1) {
                        RETARGS(args[2].value);
                    } else {
                        RETARGS(args[3].value);
                    }
                } else {
                    auto T1 = storage_type(cond.type);
                    auto T2 = storage_type(args[2].value.type);
                    verify_bool_vector(T1);
                    verify(args[2].value.type, args[3].value.type);
                    if (T1->kind() == TK_Vector) {
                        verify_vector_sizes(T1, T2);
                    } else if (T2->kind() == TK_Vector) {
                        cond = smear(cond, cast<VectorType>(T2)->count);
                    }
                    Any fargs[] = { cond, args[2].value, args[3].value };
                    RETARGS(apply_op< DispatchSelect >(fargs, 3));
                }
            } else {
                auto T1 = storage_type(args[1].value.indirect_type());
                auto T2 = storage_type(args[2].value.indirect_type());
                auto T3 = storage_type(args[3].value.indirect_type());
                verify_bool_vector(T1);
                if (T1->kind() == TK_Vector) {
                    verify_vector_sizes(T1, T2);
                }
                verify(T2, T3);
                RETARGTYPES(args[2].value.indirect_type());
            }
        } break;
        case FN_Unconst: {
            CHECKARGS(1, 1);
            if (!args[1].value.is_const()) {
                RETARGS(args[1]);
            } else {
                auto T = args[1].value.indirect_type();
                auto et = dyn_cast<ExternType>(T);
                if (et) {
                    RETARGTYPES(et->pointer_type);
                } else if (args[1].value.type == TYPE_Label) {
                    Label *fn = args[1].value;
                    fn->verify_compilable();
                    const Type *functype = Pointer(
                        fn->get_function_type(), PTF_NonWritable, SYM_Unnamed);
                    RETARGTYPES(functype);
                } else {
                    RETARGTYPES(T);
                }
            }
        } break;
        case FN_Bitcast: {
            CHECKARGS(2, 2);
            // todo: verify source and dest type are non-aggregate
            // also, both must be of same category
            args[2].value.verify(TYPE_Type);
            const Type *SrcT = args[1].value.indirect_type();
            const Type *DestT = args[2].value.typeref;
            if (SrcT == DestT) {
                RETARGS(args[1].value);
            } else {
                const Type *SSrcT = storage_type(SrcT);
                const Type *SDestT = storage_type(DestT);

                if (canonical_typekind(SSrcT->kind())
                        != canonical_typekind(SDestT->kind())) {
                    StyledString ss;
                    ss.out << "can not bitcast value of type " << SrcT
                        << " to type " << DestT
                        << " because storage types are not of compatible category";
                    location_error(ss.str());
                }
                if (SSrcT != SDestT) {
                    switch (SDestT->kind()) {
                    case TK_Array:
                    //case TK_Vector:
                    case TK_Tuple:
                    case TK_Union: {
                        StyledString ss;
                        ss.out << "can not bitcast value of type " << SrcT
                            << " to type " << DestT
                            << " with aggregate storage type " << SDestT;
                        location_error(ss.str());
                    } break;
                    default: break;
                    }
                }
                if (args[1].value.is_const()) {
                    Any result = args[1].value;
                    result.type = DestT;
                    RETARGS(result);
                } else {
                    RETARGTYPES(DestT);
                }
            }
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            verify_integer(storage_type(args[1].value.indirect_type()));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_kind<TK_Pointer>(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            verify_kind<TK_Pointer>(
                storage_type(args[1].value.indirect_type()));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if (cast<RealType>(T)->width >= cast<RealType>(DestT)->width) {
            } else { invalid_op2_types_error(T, DestT); }
            RETARGTYPES(DestT);
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if (cast<RealType>(T)->width <= cast<RealType>(DestT)->width) {
            } else { invalid_op2_types_error(T, DestT); }
            RETARGTYPES(DestT);
        } break;
        case FN_FPToUI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            if ((T == TYPE_F32) || (T == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            if ((T == TYPE_F32) || (T == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_UIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((DestT == TYPE_F32) || (DestT == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((DestT == TYPE_F32) || (DestT == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_ZExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_SExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_ExtractElement: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(vi->element_type);
        } break;
        case FN_InsertElement: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.indirect_type());
            const Type *ET = storage_type(args[2].value.indirect_type());
            verify_integer(storage_type(args[3].value.indirect_type()));
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            verify(storage_type(vi->element_type), ET);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_ShuffleVector: {
            CHECKARGS(3, 3);
            const Type *TV1 = storage_type(args[1].value.indirect_type());
            const Type *TV2 = storage_type(args[2].value.indirect_type());
            const Type *TMask = storage_type(args[3].value.type);
            verify_kind<TK_Vector>(TV1);
            verify_kind<TK_Vector>(TV2);
            verify_kind<TK_Vector>(TMask);
            verify(TV1, TV2);
            auto vi = cast<VectorType>(TV1);
            auto mask_vi = cast<VectorType>(TMask);
            verify(TYPE_I32, mask_vi->element_type);
            size_t incount = vi->count * 2;
            size_t outcount = mask_vi->count;
            for (size_t i = 0; i < outcount; ++i) {
                verify_range(
                    (size_t)mask_vi->unpack(args[3].value.pointer, i).i32,
                    incount);
            }
            RETARGTYPES(Vector(vi->element_type, outcount));
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            size_t idx = cast_number<size_t>(args[2].value);
            const Type *T = storage_type(args[1].value.indirect_type());
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                RETARGTYPES(ai->type_at_index(idx));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                RETARGTYPES(ti->type_at_index(idx));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                RETARGTYPES(ui->type_at_index(idx));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
        } break;
        case FN_InsertValue: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.indirect_type());
            const Type *ET = storage_type(args[2].value.indirect_type());
            size_t idx = cast_number<size_t>(args[3].value);
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                verify(storage_type(ai->type_at_index(idx)), ET);
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                verify(storage_type(ti->type_at_index(idx)), ET);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                verify(storage_type(ui->type_at_index(idx)), ET);
            } break;
            default: {
                StyledString ss;
                ss.out << "can not insert value into type " << T;
                location_error(ss.str());
            } break;
            }
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Undef: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(args[1].value.typeref);
        } break;
        case FN_Malloc: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(NativePointer(args[1].value.typeref));
        } break;
        case FN_Alloca: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(LocalPointer(args[1].value.typeref));
        } break;
        case FN_AllocaExceptionPad: {
            CHECKARGS(0, 0);
            RETARGTYPES(LocalPointer(Array(TYPE_U8, sizeof(ExceptionPad))));
        } break;
        case FN_AllocaOf: {
            CHECKARGS(1, 1);
            RETARGTYPES(LocalROPointer(args[1].value.indirect_type()));
        } break;
        case FN_MallocArray: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Type);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(NativePointer(args[1].value.typeref));
        } break;
        case FN_AllocaArray: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Type);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(LocalPointer(args[1].value.typeref));
        } break;
        case FN_Free: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Pointer>(T);
            verify_writable(T);
            if (cast<PointerType>(T)->storage_class != SYM_Unnamed) {
                location_error(String::from(
                    "pointer is not a heap pointer"));
            }
            RETARGTYPES();
        } break;
        case FN_GetElementPtr: {
            CHECKARGS(2, -1);
            const Type *T = storage_type(args[1].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            auto pi = cast<PointerType>(T);
            T = pi->element_type;
            bool all_const = args[1].value.is_const();
            if (all_const) {
                for (size_t i = 2; i < args.size(); ++i) {
                    if (!args[i].value.is_const()) {
                        all_const = false;
                        break;
                    }
                }
            }
            if (!is_extern && all_const) {
                void *ptr = args[1].value.pointer;
                size_t idx = cast_number<size_t>(args[2].value);
                ptr = pi->getelementptr(ptr, idx);

                for (size_t i = 3; i < args.size(); ++i) {
                    const Type *ST = storage_type(T);
                    auto &&arg = args[i].value;
                    switch(ST->kind()) {
                    case TK_Array: {
                        auto ai = cast<ArrayType>(ST);
                        T = ai->element_type;
                        size_t idx = cast_number<size_t>(arg);
                        ptr = ai->getelementptr(ptr, idx);
                    } break;
                    case TK_Tuple: {
                        auto ti = cast<TupleType>(ST);
                        size_t idx = 0;
                        if (arg.type == TYPE_Symbol) {
                            idx = ti->field_index(arg.symbol);
                            if (idx == (size_t)-1) {
                                StyledString ss;
                                ss.out << "no such field " << arg.symbol << " in storage type " << ST;
                                location_error(ss.str());
                            }
                            // rewrite field
                            arg = (int)idx;
                        } else {
                            idx = cast_number<size_t>(arg);
                        }
                        T = ti->type_at_index(idx);
                        ptr = ti->getelementptr(ptr, idx);
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "can not get element pointer from type " << T;
                        location_error(ss.str());
                    } break;
                    }
                }
                T = Pointer(T, pi->flags, pi->storage_class);
                RETARGS(Any::from_pointer(T, ptr));
            } else {
                verify_integer(storage_type(args[2].value.indirect_type()));
                for (size_t i = 3; i < args.size(); ++i) {

                    const Type *ST = storage_type(T);
                    auto &&arg = args[i];
                    switch(ST->kind()) {
                    case TK_Array: {
                        auto ai = cast<ArrayType>(ST);
                        T = ai->element_type;
                        verify_integer(storage_type(arg.value.indirect_type()));
                    } break;
                    case TK_Tuple: {
                        auto ti = cast<TupleType>(ST);
                        size_t idx = 0;
                        if (arg.value.type == TYPE_Symbol) {
                            idx = ti->field_index(arg.value.symbol);
                            if (idx == (size_t)-1) {
                                StyledString ss;
                                ss.out << "no such field " << arg.value.symbol << " in storage type " << ST;
                                location_error(ss.str());
                            }
                            // rewrite field
                            arg = Argument(arg.key, Any((int)idx));
                        } else {
                            idx = cast_number<size_t>(arg.value);
                        }
                        T = ti->type_at_index(idx);
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "can not get element pointer from type " << T;
                        location_error(ss.str());
                    } break;
                    }
                }
                T = Pointer(T, pi->flags, pi->storage_class);
                RETARGTYPES(T);
            }
        } break;
        case FN_VolatileLoad:
        case FN_Load: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            verify_readable(T);
            auto pi = cast<PointerType>(T);
            if (!is_extern && args[1].value.is_const()
                && !pi->is_writable()) {
                RETARGS(pi->unpack(args[1].value.pointer));
            } else {
                RETARGTYPES(pi->element_type);
            }
        } break;
        case FN_VolatileStore:
        case FN_Store: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[2].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            verify_writable(T);
            auto pi = cast<PointerType>(T);
            verify(storage_type(pi->element_type),
                storage_type(args[1].value.indirect_type()));
            RETARGTYPES();
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            const Type *Ta = storage_type(args[1].value.indirect_type());
            const Type *Tb = storage_type(args[2].value.indirect_type());
            verify_real_vector(Ta, 3);
            verify(Ta, Tb);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_real_vector(T);
            if (T->kind() == TK_Vector) {
                RETARGTYPES(cast<VectorType>(T)->element_type);
            } else {
                RETARGTYPES(args[1].value.indirect_type());
            }
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            const Type *T = storage_type(args[1].value.indirect_type());
            if (T->kind() == TK_Vector) {
                RETARGTYPES(cast<VectorType>(T)->element_type);
            } else {
                RETARGTYPES(args[1].value.indirect_type());
            }
        } break;
        case OP_ICmpEQ:
        case OP_ICmpNE:
        case OP_ICmpUGT:
        case OP_ICmpUGE:
        case OP_ICmpULT:
        case OP_ICmpULE:
        case OP_ICmpSGT:
        case OP_ICmpSGE:
        case OP_ICmpSLT:
        case OP_ICmpSLE: {
            CHECKARGS(2, 2);
            verify_integer_ops(args[1].value, args[2].value);
            RETARGTYPES(
                bool_op_return_type(args[1].value.indirect_type()));
        } break;
        case OP_FCmpOEQ:
        case OP_FCmpONE:
        case OP_FCmpORD:
        case OP_FCmpOGT:
        case OP_FCmpOGE:
        case OP_FCmpOLT:
        case OP_FCmpOLE:
        case OP_FCmpUEQ:
        case OP_FCmpUNE:
        case OP_FCmpUNO:
        case OP_FCmpUGT:
        case OP_FCmpUGE:
        case OP_FCmpULT:
        case OP_FCmpULE: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            RETARGTYPES(
                bool_op_return_type(args[1].value.indirect_type()));
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
    case OP_ ## NAME: \
    case OP_ ## NAME ## NUW: \
    case OP_ ## NAME ## NSW: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define IARITH_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FARITH_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_real_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FTRI_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(3, 3); \
        verify_real_ops(args[1].value, args[2].value, args[3].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define IUN_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_integer_ops(args[1].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FUN_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_real_ops(args[1].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
            B_ARITH_OPS()

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP
        default: {
            StyledString ss;
            ss.out << "can not type builtin " << enter.builtin;
            location_error(ss.str());
        } break;
        }

        return false;
    }
#undef RETARGS
#undef RETARGTYPES

#define RETARGS(...) \
    enter = args[0].value; \
    args = { none, __VA_ARGS__ };


    static void print_traceback_entry(StyledStream &ss, const Trace &trace) {
        ss << trace.anchor << " in ";
        if (trace.name == SYM_Unnamed) {
            ss << "unnamed";
        } else {
            ss << Style_Function << trace.name.name()->data << Style_None;
        }
        ss << std::endl;
        trace.anchor->stream_source_line(ss);
    }

    static void print_traceback() {
        if (traceback.empty()) return;
        StyledStream ss(std::cerr);
        ss << "Traceback (most recent call last):" << std::endl;

        size_t sz = traceback.size();
    #if 0
        size_t lasti = sz - 1;
        size_t i = 0;
        Label *l = traceback[lasti - i];
        Label *last_head = l;
        Label *last_loc = l;
        i++;
        while (i < sz) {
            l = traceback[lasti - i++];
            Label *orig = l->get_original();
            if (!orig->is_basic_block_like()) {
                print_traceback_entry(ss, last_head, last_loc);
                last_head = l;
            }
            last_loc = l;
            if (is_calling_label(orig)
                && !orig->get_label_enter()->is_basic_block_like()) {
                if (!last_head)
                    last_head = last_loc;
                print_traceback_entry(ss, last_head, last_loc);
                last_head = nullptr;
            }
        }
        print_traceback_entry(ss, last_head, last_loc);
    #else
        std::unordered_set<const Anchor *> visited;
        size_t i = sz;
        size_t capped = 0;
        while (i != 0) {
            i--;
            auto &&trace = traceback[i];
            if (!visited.count(trace.anchor)) {
                if (capped) {
                    ss << "<...>" << std::endl;
                    capped = 0;
                }
                visited.insert(trace.anchor);
                print_traceback_entry(ss, trace);
            } else {
                capped++;
            }
        }
    #endif
    }

    Label *fold_label_macro_call(Label *l) {
        auto &&enter = l->body.enter;

        typedef Label *(*label_macro_handler)(Label *);

        label_macro_handler handler = (label_macro_handler)enter.pointer;
        return handler(l);
    }

    void fold_callable_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding callable call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        const Type *T = enter.indirect_type();

        Any value = none;
        bool result = T->lookup_call_handler(value);
        assert(result);
        args.insert(args.begin() + 1, Argument(enter));
        enter = value;
    }

    void verify_all_args_constant(Label *l) {
        if (!all_args_constant(l)) {
            location_error(String::from("all arguments must be constants"));
        }
    }

    bool fold_builtin_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding builtin call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        assert(enter.type == TYPE_Builtin);
        switch(enter.builtin.value()) {
        case KW_SyntaxExtend: {
            CHECKARGS(3, 3);
            verify_all_args_constant(l);
            const Closure *cl = args[1].value;
            const Syntax *sx = args[2].value;
            Scope *env = args[3].value;
            Specializer solver;
            Label *metafunc = solver.solve_inline(cl->frame, cl->label, { untyped(), env });
            auto rlt = metafunc->verify_return_label();
            //const Type *functype = metafunc->get_function_type();
            if (rlt->values.size() != 1)
                goto failed;
            {
                Scope *scope = nullptr;
                Any compiled = compile(metafunc, 0);
                if (rlt->values[0].value.type == TYPE_Scope) {
                    // returns a constant scope
                    typedef void (*FuncType)();
                    FuncType fptr = (FuncType)compiled.pointer;
                    fptr();
                    scope = rlt->values[0].value.scope;
                } else if ((rlt->values[0].value.type == TYPE_Unknown)
                    && (rlt->values[0].value.typeref == TYPE_Scope)) {
                    // returns a variable scope
                    typedef Scope *(*FuncType)();
                    FuncType fptr = (FuncType)compiled.pointer;
                    scope = fptr();
                } else {
                    goto failed;
                }
                enter = fold_type_label_single(cl->frame,
                    expand_module(sx, scope), { args[0] });
                args = { none };
                return false;
            }
        failed:
            set_active_anchor(sx->anchor);
            StyledString ss;
            const Type *T = rlt;
            ss.out << "syntax-extend has wrong return type (expected "
                << ReturnLabel({unknown_of(TYPE_Scope)}) << ", got "
                << T << ")";
            location_error(ss.str());
        } break;
        case FN_ScopeOf: {
            CHECKARGS(0, -1);
            Scope *scope = nullptr;
            size_t start = 1;
            if ((args.size() > 1) && (args[1].key == SYM_Unnamed)) {
                start = 2;
                scope = Scope::from(args[1].value);
            } else {
                scope = Scope::from();
            }
            for (size_t i = start; i < args.size(); ++i) {
                auto &&arg = args[i];
                if (arg.key == SYM_Unnamed) {
                    scope = Scope::from(scope, arg.value);
                } else {
                    scope->bind(arg.key, arg.value);
                }
            }
            RETARGS(scope);
        } break;
        case FN_AllocaOf: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value.type;
            void *src = get_pointer(T, args[1].value);
            void *dst = tracked_malloc(size_of(T));
            memcpy(dst, src, size_of(T));
            RETARGS(Any::from_pointer(NativeROPointer(T), dst));
        } break;
        case FN_StaticAlloc: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value;
            void *dst = tracked_malloc(size_of(T));
            RETARGS(Any::from_pointer(StaticPointer(T), dst));
        } break;
        case FN_NullOf: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value;
            Any value = none;
            value.type = T;
            if (!is_opaque(T)) {
                void *ptr = get_pointer(T, value, true);
                memset(ptr, 0, size_of(T));
            }
            RETARGS(value);
        } break;
        case FN_ExternSymbol: {
            CHECKARGS(1, 1);
            verify_kind<TK_Extern>(args[1].value);
            RETARGS(args[1].value.symbol);
        } break;
        case FN_ExternNew: {
            CHECKARGS(2, -1);
            args[1].value.verify(TYPE_Symbol);
            const Type *T = args[2].value;
            Any value(args[1].value.symbol);
            Symbol extern_storage_class = SYM_Unnamed;
            size_t flags = 0;
            int location = -1;
            int binding = -1;
            if (args.size() > 3) {
                size_t i = 3;
                while (i < args.size()) {
                    auto &&arg = args[i];
                    switch(arg.key.value()) {
                    case SYM_Location: {
                        if (location == -1) {
                            location = cast_number<int>(arg.value);
                        } else {
                            location_error(String::from("duplicate location"));
                        }
                    } break;
                    case SYM_Binding: {
                        if (binding == -1) {
                            binding = cast_number<int>(arg.value);
                        } else {
                            location_error(String::from("duplicate binding"));
                        }
                    } break;
                    case SYM_Storage: {
                        arg.value.verify(TYPE_Symbol);

                        if (extern_storage_class == SYM_Unnamed) {
                            switch(arg.value.symbol.value()) {
                            #define T(NAME) \
                                case SYM_SPIRV_StorageClass ## NAME:
                                B_SPIRV_STORAGE_CLASS()
                            #undef T
                                extern_storage_class = arg.value.symbol; break;
                            default: {
                                location_error(String::from("illegal storage class"));
                            } break;
                            }
                        } else {
                            location_error(String::from("duplicate storage class"));
                        }
                    } break;
                    case SYM_Unnamed: {
                        arg.value.verify(TYPE_Symbol);

                        switch(arg.value.symbol.value()) {
                        case SYM_Buffer: flags |= EF_BufferBlock; break;
                        case SYM_ReadOnly: flags |= EF_NonWritable; break;
                        case SYM_WriteOnly: flags |= EF_NonReadable; break;
                        case SYM_Coherent: flags |= EF_Coherent; break;
                        case SYM_Restrict: flags |= EF_Restrict; break;
                        case SYM_Volatile: flags |= EF_Volatile; break;
                        default: {
                            location_error(String::from("unknown flag"));
                        } break;
                        }
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "unexpected key: " << arg.key;
                        location_error(ss.str());
                    } break;
                    }

                    i++;
                }
            }
            value.type = Extern(T, flags, extern_storage_class, location, binding);
            RETARGS(value);
        } break;
        case FN_FunctionType: {
            CHECKARGS(1, -1);
            ArgTypes types;
            size_t k = 2;
            while (k < args.size()) {
                if (args[k].value.type != TYPE_Type)
                    break;
                types.push_back(args[k].value);
                k++;
            }
            uint32_t flags = 0;

            while (k < args.size()) {
                args[k].value.verify(TYPE_Symbol);
                Symbol sym = args[k].value.symbol;
                uint64_t flag = 0;
                switch(sym.value()) {
                case SYM_Variadic: flag = FF_Variadic; break;
                default: {
                    StyledString ss;
                    ss.out << "illegal option: " << sym;
                    location_error(ss.str());
                } break;
                }
                flags |= flag;
                k++;
            }
            RETARGS(Function(args[1].value, types, flags));
        } break;
        case FN_TupleType: {
            CHECKARGS(0, -1);
            Args values;
            for (size_t i = 1; i < args.size(); ++i) {
#if 0
                if (args[i].value.is_const()) {
                    values.push_back(args[i]);
                } else {
                    values.push_back(
                        Argument(args[i].key,
                            unknown_of(args[i].value.indirect_type())));
                }
#else
                values.push_back(
                    Argument(args[i].key,
                        unknown_of(args[i].value)));
#endif
            }
            RETARGS(MixedTuple(values));
        } break;
        case FN_UnionType: {
            CHECKARGS(0, -1);
            Args values;
            for (size_t i = 1; i < args.size(); ++i) {
#if 0
                if (args[i].value.is_const()) {
                    location_error(String::from("all union type arguments must be non-constant"));
                    //values.push_back(args[i]);
                } else {
                    values.push_back(
                        Argument(args[i].key,
                            unknown_of(args[i].value.indirect_type())));
                }
#else
                values.push_back(
                    Argument(args[i].key,
                        unknown_of(args[i].value)));
#endif
            }
            RETARGS(MixedUnion(values));
        } break;
        case FN_ReturnLabelType: {
            CHECKARGS(0, -1);
            Args values;
            // can theoretically also be initialized with constants; for that
            // we need a way to quote constants.
            for (size_t i = 1; i < args.size(); ++i) {
                const Type *T = args[i].value;
                values.push_back(Argument(args[i].key, unknown_of(T)));
            }
            RETARGS(ReturnLabel(values));
        } break;
        case FN_Location: {
            CHECKARGS(0, 0);
            RETARGS(l->body.anchor);
        } break;
        case SFXFN_DelTypeSymbol: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            const_cast<Type *>(T)->del(args[2].value.symbol);
            RETARGS();
        } break;
        case FN_TypeAt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            Any result = none;
            if (!T->lookup(args[2].value.symbol, result)) {
                RETARGS(none, false);
            } else {
                RETARGS(result, true);
            }
        } break;
        case FN_TypeLocalAt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            Any result = none;
            if (!T->lookup_local(args[2].value.symbol, result)) {
                RETARGS(none, false);
            } else {
                RETARGS(result, true);
            }
        } break;
        case FN_IsConstant: {
            CHECKARGS(1, 1);
            RETARGS(args[1].value.is_const());
        } break;
        case KW_Forward: {
            CHECKARGS(0, -1);
            Args result = { none };
            for (size_t i = 1; i < args.size(); ++i) {
                result.push_back(args[i]);
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_VaCountOf: {
            RETARGS((int)(args.size()-1));
        } break;
        case FN_VaKeys: {
            CHECKARGS(0, -1);
            Args result = { none };
            for (size_t i = 1; i < args.size(); ++i) {
                result.push_back(args[i].key);
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_VaValues: {
            CHECKARGS(0, -1);
            Args result = { none };
            for (size_t i = 1; i < args.size(); ++i) {
                result.push_back(args[i].value);
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_VaKey: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Symbol);
            enter = args[0].value;
            args = { none, Argument(args[1].value.symbol, args[2].value) };
        } break;
        case FN_VaAt: {
            CHECKARGS(1, -1);
            Args result = { none };
            if (args[1].value.type == TYPE_Symbol) {
                auto key = args[1].value.symbol;
                for (size_t i = 2; i < args.size(); ++i) {
                    if (args[i].key == key) {
                        result.push_back(args[i]);
                    }
                }
            } else {
                size_t idx = cast_number<size_t>(args[1].value);
                for (size_t i = (idx + 2); i < args.size(); ++i) {
                    result.push_back(args[i]);
                }
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_OffsetOf: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            auto &&arg = args[2].value;
            size_t idx = 0;
            T = storage_type(T);
            verify_kind<TK_Tuple>(T);
            auto ti = cast<TupleType>(T);
            if (arg.type == TYPE_Symbol) {
                idx = ti->field_index(arg.symbol);
                if (idx == (size_t)-1) {
                    StyledString ss;
                    ss.out << "no such field " << arg.symbol << " in storage type " << T;
                    location_error(ss.str());
                }
                // rewrite field
                arg = (int)idx;
            } else {
                idx = cast_number<size_t>(arg);
            }
            verify_range(idx, ti->offsets.size());
            RETARGS(ti->offsets[idx]);
        } break;
        case FN_Branch: {
            CHECKARGS(3, 3);
            args[1].value.verify(TYPE_Bool);
            // either branch label is typed and binds no parameters,
            // so we can directly inline it
            const Closure *newl = nullptr;
            if (args[1].value.i1) {
                newl = args[2].value;
            } else {
                newl = args[3].value;
            }
            verify_branch_continuation(newl);
            Any cont = args[0].value;
            if (cont.type == TYPE_Closure) {
                cont.closure->frame->inline_merge = true;
            }
            enter = newl;
            args = { cont };
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            verify_integer(storage_type(args[1].value.type));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_kind<TK_Pointer>(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            verify_kind<TK_Pointer>(storage_type(args[1].value.type));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((T == TYPE_F64) && (DestT == TYPE_F32)) {
                RETARGS((float)args[1].value.f64);
            } else { invalid_op2_types_error(T, DestT); }
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((T == TYPE_F32) && (DestT == TYPE_F64)) {
                RETARGS((double)args[1].value.f32);
            } else { invalid_op2_types_error(T, DestT); }
        } break;
        case FN_FPToUI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            uint64_t val = 0;
            if (T == TYPE_F32) {
                val = (uint64_t)args[1].value.f32;
            } else if (T == TYPE_F64) {
                val = (uint64_t)args[1].value.f64;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            Any result = val;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            int64_t val = 0;
            if (T == TYPE_F32) {
                val = (int64_t)args[1].value.f32;
            } else if (T == TYPE_F64) {
                val = (int64_t)args[1].value.f64;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            Any result = val;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_UIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            uint64_t src = cast_number<uint64_t>(args[1].value);
            Any result = none;
            if (DestT == TYPE_F32) {
                result = (float)src;
            } else if (DestT == TYPE_F64) {
                result = (double)src;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGS(result);
        } break;
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            int64_t src = cast_number<int64_t>(args[1].value);
            Any result = none;
            if (DestT == TYPE_F32) {
                result = (float)src;
            } else if (DestT == TYPE_F64) {
                result = (double)src;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGS(result);
        } break;
        case FN_ZExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            auto ST = storage_type(T);
            verify_integer(ST);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            auto DestST = storage_type(DestT);
            verify_integer(DestST);
            Any result = args[1].value;
            result.type = DestT;
            int oldbitnum = integer_type_bit_size(ST);
            int newbitnum = integer_type_bit_size(DestST);
            for (int i = oldbitnum; i < newbitnum; ++i) {
                result.u64 &= ~(1ull << i);
            }
            RETARGS(result);
        } break;
        case FN_SExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            auto ST = storage_type(T);
            verify_integer(ST);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            auto DestST = storage_type(DestT);
            verify_integer(DestST);
            Any result = args[1].value;
            result.type = DestT;
            int oldbitnum = integer_type_bit_size(ST);
            int newbitnum = integer_type_bit_size(DestST);
            uint64_t bit = (result.u64 >> (oldbitnum - 1)) & 1ull;
            for (int i = oldbitnum; i < newbitnum; ++i) {
                result.u64 &= ~(1ull << i);
                result.u64 |= bit << i;
            }
            RETARGS(result);
        } break;
        case FN_TypeOf: {
            CHECKARGS(1, 1);
            RETARGS(args[1].value.indirect_type());
        } break;
        case FN_ExtractElement: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[1].value.type);
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            size_t idx = cast_number<size_t>(args[2].value);
            RETARGS(vi->unpack(args[1].value.pointer, idx));
        } break;
        case FN_InsertElement: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.type);
            const Type *ET = storage_type(args[2].value.type);
            size_t idx = cast_number<size_t>(args[3].value);
            void *destptr = args[1].value.pointer;
            void *offsetptr = nullptr;
            destptr = copy_storage(T, destptr);
            auto vi = cast<VectorType>(T);
            verify(storage_type(vi->type_at_index(idx)), ET);
            offsetptr = vi->getelementptr(destptr, idx);
            void *srcptr = get_pointer(ET, args[2].value);
            memcpy(offsetptr, srcptr, size_of(ET));
            RETARGS(Any::from_pointer(args[1].value.type, destptr));
        } break;
        case FN_ShuffleVector: {
            CHECKARGS(3, 3);
            const Type *TV1 = storage_type(args[1].value.type);
            const Type *TV2 = storage_type(args[2].value.type);
            const Type *TMask = storage_type(args[3].value.type);
            verify_kind<TK_Vector>(TV1);
            verify_kind<TK_Vector>(TV2);
            verify_kind<TK_Vector>(TMask);
            verify(TV1, TV2);
            auto vi = cast<VectorType>(TV1);
            auto mask_vi = cast<VectorType>(TMask);
            verify(TYPE_I32, storage_type(mask_vi->element_type));
            size_t halfcount = vi->count;
            size_t incount = halfcount * 2;
            size_t outcount = mask_vi->count;
            const Type *T = Vector(vi->element_type, outcount);
            void *srcptr_a = get_pointer(TV1, args[1].value);
            void *srcptr_b = get_pointer(TV1, args[2].value);
            void *destptr = alloc_storage(T);
            auto out_vi = cast<VectorType>(T);
            size_t esize = size_of(vi->element_type);
            for (size_t i = 0; i < outcount; ++i) {
                size_t idx = (size_t)mask_vi->unpack(args[3].value.pointer, i).i32;
                verify_range(idx, incount);
                void *srcptr;
                if (idx < halfcount) {
                    srcptr = srcptr_a;
                } else {
                    srcptr = srcptr_b;
                    idx -= halfcount;
                }
                void *inp = vi->getelementptr(srcptr, idx);
                void *outp = out_vi->getelementptr(destptr, i);
                memcpy(outp, inp, esize);
            }
            RETARGS(Any::from_pointer(T, destptr));
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            size_t idx = cast_number<size_t>(args[2].value);
            const Type *T = storage_type(args[1].value.type);
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                RETARGS(ai->unpack(args[1].value.pointer, idx));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                RETARGS(ti->unpack(args[1].value.pointer, idx));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                RETARGS(ui->unpack(args[1].value.pointer, idx));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
        } break;
        case FN_InsertValue: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.type);
            const Type *ET = storage_type(args[2].value.type);
            size_t idx = cast_number<size_t>(args[3].value);

            void *destptr = args[1].value.pointer;
            void *offsetptr = nullptr;
            switch(T->kind()) {
            case TK_Array: {
                destptr = copy_storage(T, destptr);
                auto ai = cast<ArrayType>(T);
                verify(storage_type(ai->type_at_index(idx)), ET);
                offsetptr = ai->getelementptr(destptr, idx);
            } break;
            case TK_Tuple: {
                destptr = copy_storage(T, destptr);
                auto ti = cast<TupleType>(T);
                verify(storage_type(ti->type_at_index(idx)), ET);
                offsetptr = ti->getelementptr(destptr, idx);
            } break;
            case TK_Union: {
                destptr = copy_storage(T, destptr);
                auto ui = cast<UnionType>(T);
                verify(storage_type(ui->type_at_index(idx)), ET);
                offsetptr = destptr;
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
            void *srcptr = get_pointer(ET, args[2].value);
            memcpy(offsetptr, srcptr, size_of(ET));
            RETARGS(Any::from_pointer(args[1].value.type, destptr));
        } break;
        case FN_AnyExtract: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Any);
            Any arg = *args[1].value.ref;
            RETARGS(arg);
        } break;
        case FN_AnyWrap: {
            CHECKARGS(1, 1);
            verify_all_args_constant(l);
            RETARGS(args[1].value.toref());
        } break;
        case SFXFN_CompilerError: {
            CHECKARGS(1, 1);
            location_error(args[1].value);
            RETARGS();
        } break;
        case FN_CompilerMessage: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_String);
            StyledString ss;
            ss.out << l->body.anchor << " message: " << args[1].value.string->data << std::endl;
            std::cout << ss.str()->data;
            RETARGS();
        } break;
        case FN_Dump: {
            CHECKARGS(0, -1);
            StyledStream ss(std::cerr);
            ss << l->body.anchor << " dump:";
            for (size_t i = 1; i < args.size(); ++i) {
                ss << " ";
                if (args[i].key != SYM_Unnamed) {
                    ss << args[i].key << " " << Style_Operator << "=" << Style_None << " ";
                }

                if (args[i].value.is_const()) {
                    stream_expr(ss, args[i].value, StreamExprFormat::singleline());
                } else {
                    /*
                    ss << "<unknown>"
                        << Style_Operator << ":" << Style_None
                        << args[i].value.indirect_type() << std::endl;*/
                    args[i].value.stream(ss, false);
                }
            }
            ss << std::endl;
            enter = args[0].value;
            args[0].value = none;
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            const Type *Ta = storage_type(args[1].value.type);
            const Type *Tb = storage_type(args[2].value.type);
            verify_real_vector(Ta, 3);
            verify(Ta, Tb);
            RETARGS(apply_real_op<op_Cross>(args[1].value, args[2].value));
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGS(apply_real_op<op_Normalize>(args[1].value));
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGS(apply_real_op<op_Length>(args[1].value));
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            RETARGS(apply_real_op<op_Distance>(args[1].value, args[2].value));
        } break;
        case OP_ICmpEQ:
        case OP_ICmpNE:
        case OP_ICmpUGT:
        case OP_ICmpUGE:
        case OP_ICmpULT:
        case OP_ICmpULE:
        case OP_ICmpSGT:
        case OP_ICmpSGE:
        case OP_ICmpSLT:
        case OP_ICmpSLE: {
            CHECKARGS(2, 2);
            verify_integer_ops(args[1].value, args[2].value);
#define B_INT_OP1(OP, N) \
    result = apply_integer_op<IntTypes_ ## N, op_ ## OP>(args[1].value);
#define B_INT_OP2(OP, N) \
    result = apply_integer_op<IntTypes_ ## N, op_ ## OP>(args[1].value, args[2].value);
            Any result = false;
            switch(enter.builtin.value()) {
            case OP_ICmpEQ: B_INT_OP2(Equal, u); break;
            case OP_ICmpNE: B_INT_OP2(NotEqual, u); break;
            case OP_ICmpUGT: B_INT_OP2(Greater, u); break;
            case OP_ICmpUGE: B_INT_OP2(GreaterEqual, u); break;
            case OP_ICmpULT: B_INT_OP2(Less, u); break;
            case OP_ICmpULE: B_INT_OP2(LessEqual, u); break;
            case OP_ICmpSGT: B_INT_OP2(Greater, i); break;
            case OP_ICmpSGE: B_INT_OP2(GreaterEqual, i); break;
            case OP_ICmpSLT: B_INT_OP2(Less, i); break;
            case OP_ICmpSLE: B_INT_OP2(LessEqual, i); break;
            default: assert(false); break;
            }
            RETARGS(result);
        } break;
        case OP_FCmpOEQ:
        case OP_FCmpONE:
        case OP_FCmpORD:
        case OP_FCmpOGT:
        case OP_FCmpOGE:
        case OP_FCmpOLT:
        case OP_FCmpOLE:
        case OP_FCmpUEQ:
        case OP_FCmpUNE:
        case OP_FCmpUNO:
        case OP_FCmpUGT:
        case OP_FCmpUGE:
        case OP_FCmpULT:
        case OP_FCmpULE: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
#define B_FLOAT_OP1(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value);
#define B_FLOAT_OP2(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value, args[2].value);
#define B_FLOAT_OP3(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value, args[2].value, args[3].value);
            Any result = false;
            switch(enter.builtin.value()) {
            case OP_FCmpOEQ: B_FLOAT_OP2(OEqual); break;
            case OP_FCmpONE: B_FLOAT_OP2(ONotEqual); break;
            case OP_FCmpORD: B_FLOAT_OP2(Ordered); break;
            case OP_FCmpOGT: B_FLOAT_OP2(OGreater); break;
            case OP_FCmpOGE: B_FLOAT_OP2(OGreaterEqual); break;
            case OP_FCmpOLT: B_FLOAT_OP2(OLess); break;
            case OP_FCmpOLE: B_FLOAT_OP2(OLessEqual); break;
            case OP_FCmpUEQ: B_FLOAT_OP2(UEqual); break;
            case OP_FCmpUNE: B_FLOAT_OP2(UNotEqual); break;
            case OP_FCmpUNO: B_FLOAT_OP2(Unordered); break;
            case OP_FCmpUGT: B_FLOAT_OP2(UGreater); break;
            case OP_FCmpUGE: B_FLOAT_OP2(UGreaterEqual); break;
            case OP_FCmpULT: B_FLOAT_OP2(ULess); break;
            case OP_FCmpULE: B_FLOAT_OP2(ULessEqual); break;
            default: assert(false); break;
            }
            RETARGS(result);
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
    case OP_ ## NAME: \
    case OP_ ## NAME ## NUW: \
    case OP_ ## NAME ## NSW: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        Any result = none; \
        switch(enter.builtin.value()) { \
        case OP_ ## NAME: B_INT_OP2(NAME, u); break; \
        case OP_ ## NAME ## NUW: B_INT_OP2(NAME, u); break; \
        case OP_ ## NAME ## NSW: B_INT_OP2(NAME, i); break; \
        default: assert(false); break; \
        } \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define IARITH_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        Any result = none; \
        B_INT_OP2(NAME, PFX); \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define FARITH_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_real_ops(args[1].value, args[2].value); \
        Any result = none; \
        B_FLOAT_OP2(NAME); \
        RETARGS(result); \
    } break;
#define FTRI_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(3, 3); \
        verify_real_ops(args[1].value, args[2].value, args[3].value); \
        Any result = none; \
        B_FLOAT_OP3(NAME); \
        RETARGS(result); \
    } break;
#define IUN_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_integer_ops(args[1].value); \
        Any result = none; \
        B_INT_OP1(NAME, PFX); \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define FUN_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_real_ops(args[1].value); \
        Any result = none; \
        B_FLOAT_OP1(NAME); \
        RETARGS(result); \
    } break;

        B_ARITH_OPS()

        default: {
            StyledString ss;
            ss.out << "can not fold constant expression using builtin " << enter.builtin;
            location_error(ss.str());
        } break;
        }
        return true;
    }

    bool requires_mergelabel(const Any &cont) {
        if (cont.type == TYPE_Closure) {
            const Closure *cl = cont.closure;
            if (cl->label->is_merge()) {
                return false;
            }
        }
        return true;
    }

    bool mergelabel_has_params(const Any &cont) {
        if (cont.type == TYPE_Closure) {
            const Closure *cl = cont.closure;
            if (!cl->label->has_params()) {
                return false;
            }
        }
        return true;
    }

    void type_branch_continuations(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "inlining branch continuations in " << l << std::endl;
#endif
        assert(!l->body.is_complete());
        auto &&args = l->body.args;
        CHECKARGS(3, 3);

        args[1].value.verify_indirect(TYPE_Bool);
        const Closure *then_br = args[2].value;
        const Closure *else_br = args[3].value;
        verify_branch_continuation(then_br);
        verify_branch_continuation(else_br);

        Any cont = args[0].value;
        args[2].value = fold_type_label_single(then_br->frame, then_br->label, { cont });
        args[3].value = fold_type_label_single(else_br->frame, else_br->label, { cont });
        args[0].value = none;
    }

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef FARITH_OPF
#undef B_INT_OP2
#undef B_INT_OP1
#undef B_FLOAT_OP3
#undef B_FLOAT_OP2
#undef B_FLOAT_OP1
#undef CHECKARGS
#undef RETARGS
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP

    static Label *skip_jumps(Label *l) {
        size_t counter = 0;
        while (jumps_immediately(l)) {
            l = l->body.enter.label;
            counter++;
            if (counter == SCOPES_MAX_SKIP_JUMPS) {
                std::cerr
                    << "internal warning: max iterations exceeded"
                        " during jump skip check" << std::endl;
                break;
            }
        }
        return l;
    }

    static bool is_exiting(Label *l) {
        return is_calling_continuation(l) || is_continuing_to_parameter(l);
    }

    // label targets count as calls, all other as ops
    static bool is_empty_function(Label *l) {
        assert(!l->params.empty());
        auto rtype = l->params[0]->type;
        if (rtype->kind() != TK_ReturnLabel)
            return false;
        const ReturnLabelType *rlt = cast<ReturnLabelType>(rtype);
        if (rlt->has_variables() || !rlt->is_returning())
            return false;
        assert(!l->params.empty());
        std::unordered_set<Label *> visited;
        while (!visited.count(l)) {
            visited.insert(l);
            if (jumps_immediately(l)) {
                l = l->body.enter.label;
                continue;
            }
            if (!is_calling_continuation(l)) {
                return false;
            }
            assert(!is_continuing_to_label(l));
            if (l->body.args[0].value.type == TYPE_Nothing) {
                // branch, unreachable, etc.
                break;
            } else {
                StyledStream ss(std::cerr);
                ss << "internal warning: unexpected continuation type "
                    << l->body.args[0].value.type
                    << " encountered while counting instructions" << std::endl;
                break;
            }
        }
        return true;
    }

    static bool matches_arg_count(Label *l, size_t inargs) {
        // works only on instantiated labels, as we're
        // assuming no parameter is variadic at this point
        auto &&params = l->params;
        return ((inargs + 1) == std::max(size_t(1), params.size()));
    }

    static bool forwards_all_args(Label *l) {
        assert(!l->params.empty());
        auto &&args = l->body.args;
        auto &&params = l->params;
        if (args.size() != params.size())
            return false;
        for (size_t i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            if (arg.value.type != TYPE_Parameter)
                return false;
            if (arg.value.parameter != params[i])
                return false;
        }
        return true;
    }

    // a label just jumps to the next label
    static bool jumps_immediately(Label *l) {
        return is_calling_label(l)
            && l->get_label_enter()->is_basic_block_like();
    }

    void clear_continuation_arg(Label *l) {
        auto &&args = l->body.args;
        args[0] = none;
    }

    // clear continuation argument and clear it for labels that use it
    void delete_continuation(Label *owner) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "deleting continuation of " << owner << std::endl;
#endif

        assert(!owner->params.empty());
        Parameter *param = owner->params[0];
        param->type = TYPE_Nothing;

        assert(!is_called_by(param, owner));
        if (is_continuing_from(param, owner)) {
            clear_continuation_arg(owner);
        }
    }

    const Type *get_return_type_from_function_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "typing continuation from function call in " << l << std::endl;
#endif
        auto &&enter = l->body.enter;

        const FunctionType *fi = extract_function_type(enter.indirect_type());
        verify_function_argument_signature(fi, l);
        return fi->return_type;
    }

    const Type *get_return_type_from_call_arguments(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "typing continuation call in " << l << std::endl;
#endif
        auto &&args = l->body.args;
        Args values;
        values.reserve(args.size());
        for (size_t i = 1; i < args.size(); ++i) {
            values.push_back(Argument(
                args[i].key,
                unknown_of(args[i].value.indirect_type())));
        }
        clear_continuation_arg(l);
        return ReturnLabel(values);
    }

    static void inc_solve_ref() {
        solve_refs++;
    }

    static bool dec_solve_ref() {
        solve_refs--;
        assert(solve_refs >= 0);
        return solve_refs == 0;
    }

    Label *solve_inline(Frame *frame, Label *label, const Args &values) {
        #if 0
        {
            assert(user_map.empty());
            std::unordered_set<Label *> visited;
            label->build_reachable(visited, nullptr, true);
            for (auto it = visited.begin(); it != visited.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }
        #endif

        Frame *entryf = fold_type_label_single_frame(frame, label, values);

        inc_solve_ref();
        SCOPES_TRY()

        normalize_function(entryf);

        SCOPES_CATCH(exc)
            if (dec_solve_ref()) {
                print_traceback();
                traceback.clear();
            }
            error(exc);
        SCOPES_TRY_END()

        if (dec_solve_ref()) {
            traceback.clear();
        }

        Label *entry = entryf->get_instance();
        validate_scope(entry);
        return entry;
    }

    Label *typify(Frame *frame, Label *label, const ArgTypes &argtypes) {

        Args args;
        args.reserve(argtypes.size() + 1);
        args = { Argument(untyped()) };
        for (size_t i = 0; i < argtypes.size(); ++i) {
            args.push_back(Argument(unknown_of(argtypes[i])));
        }
        return solve_inline(frame, label, args);
    }

#if 0
    Label *solve(Frame *frame, Label *label, const Args &values) {
        assert(frame);
        assert(!label->params.empty());
        Args args;
        args.reserve(values.size() + 1);
        args = { Argument(untyped()) };
        for (size_t i = 0; i < values.size(); ++i) {
            args.push_back(values[i]);
        }
        return solve_inline(frame, label, values);
    }
#endif

    const Type *complete_existing_label_continuation (Label *l) {
        Label *enter_label = l->get_label_enter();
        if (!enter_label->is_basic_block_like()) {
            const FunctionType *fi = cast<FunctionType>(enter_label->get_function_type());
            verify_function_argument_signature(fi, l);

            assert(enter_label->body.is_complete());
            assert(enter_label->is_return_param_typed());
            return enter_label->get_return_type();
        }
        return nullptr;
    }

    void on_label_processing(Label *l, const char *task = nullptr) {
        if (!enable_step_debugger) {
            if (clicmd == CmdSkip) {
                enable_step_debugger = true;
                clicmd = CmdNone;
            }
            return;
        }
        clicmd = CmdNone;
        auto slfmt = StreamLabelFormat::debug_single();
        slfmt.anchors = StreamLabelFormat::Line;
        if (task) {
            ss_cout << task << std::endl;
        }
        stream_label(ss_cout, l, slfmt);
        bool skip = false;
        while (!skip) {
            set_active_anchor(l->body.anchor);
            char *r = linenoise("solver> ");
            if (!r) {
                location_error(String::from("aborted"));
            }

            linenoiseHistoryAdd(r);
            SCOPES_TRY()
                auto file = SourceFile::from_string(Symbol("<string>"),
                    String::from_cstr(r));
                LexerParser parser(file);
                auto expr = parser.parse();
                //stream_expr(ss_cout, expr, StreamExprFormat());
                const List *stmts = unsyntax(expr);
                if (stmts != EOL) {
                    while (stmts != EOL) {
                        set_active_anchor(stmts->at.syntax->anchor);
                        auto cmd = unsyntax(stmts->at);
                        Symbol head = SYM_Unnamed;
                        const List *arglist = nullptr;
                        if (cmd.type == TYPE_Symbol) {
                            head = cmd.symbol;
                        } else if (cmd.type == TYPE_List) {
                            arglist = cmd.list;
                            if (arglist != EOL) {
                                cmd = unsyntax(arglist->at);
                                if (cmd.type == TYPE_Symbol) {
                                    head = cmd.symbol;
                                }
                                arglist = arglist->next;
                            }
                        }
                        if (head == SYM_Unnamed) {
                            location_error(String::from("syntax error"));
                        }
                        switch(head.value()) {
                        case SYM_C:
                        case KW_Continue: {
                            skip = true;
                            enable_step_debugger = false;
                        } break;
                        case SYM_Skip: {
                            skip = true;
                            clicmd = CmdSkip;
                            enable_step_debugger = false;
                        } break;
                        case SYM_Original: {
                            Label *o = l->original;
                            while (o) {
                                stream_label(ss_cout, o, slfmt);
                                o = o->original;
                            }
                        } break;
                        case SYM_Help: {
                            ss_cout << "Available commands:" << std::endl;
                            ss_cout << "c(ontinue) help original skip" << std::endl;
                            ss_cout << "An empty line continues to the next label." << std::endl;
                        } break;
                        default: {
                            location_error(String::from("unknown command. try 'help'."));
                        }break;
                        }
                        stmts = stmts->next;
                    }
                } else {
                    skip = true;
                }
            SCOPES_CATCH(exc)
                print_exception(exc);
            SCOPES_TRY_END()
        }
    }

    bool fold_useless_labels(Label *l) {
        Label *startl = l;
    repeat:
        if (l->body.is_complete()) {
            auto &&enter = l->body.enter;
            if (enter.type == TYPE_Label) {
                Label *nextl = enter.label;
                if (nextl->is_basic_block_like()
                    && !nextl->is_important()
                    && !nextl->has_params()) {
                    l = nextl;
                    goto repeat;
                }
            }
        }
        if (l != startl) {
            const Anchor *saveanchor = startl->body.anchor;
            startl->body = l->body;
            startl->body.anchor = saveanchor;
            return true;
        }
        return false;
    }

    void verify_stack_size() {
        size_t ssz = memory_stack_size();
        if (ssz >= SCOPES_MAX_STACK_SIZE) {
            location_error(String::from("stack overflow during partial evaluation"));
        }
    }

    // fold body of single label if possible
    // returns either the completed label if definite, or the next incomplete
    // or complete possible label, or null if completed label is recursive
    Label *fold_label_body(Label *l) {
    repeat:
        assert(!l->body.is_complete());
        on_label_processing(l);
        assert(!l->is_template());
        l->verify_valid();
        assert(all_params_typed(l));

        set_active_anchor(l->body.anchor);

        if (!all_args_typed(l)) {
            size_t idx = find_untyped_arg(l);
            StyledString ss;
            ss.out << "parameter " << l->body.args[idx].value.parameter
                << " passed as argument " << idx << " has not been typed yet";
            location_error(ss.str());
        }

        const Type *rtype = nullptr;
        if (is_calling_label(l)) {
            if (!l->get_label_enter()->body.is_complete()) {
                location_error(String::from("failed to propagate return type from untyped label"));
            }
            assert(all_params_typed(l));
            rtype = complete_existing_label_continuation(l);
        } else if (is_calling_label_macro(l)) {
            Any enter = l->body.enter;
            fold_label_macro_call(l);
            if (l->body.enter == enter) {
                location_error(String::from("label macro call failed to fold"));
            }
            if (!l->body.is_complete())
                goto repeat;
        } else if (is_calling_callable(l)) {
            fold_callable_call(l);
            goto repeat;
        } else if (is_calling_function(l)) {
            verify_no_keyed_args(l);
            rtype = get_return_type_from_function_call(l);
        } else if (is_calling_builtin(l)) {
            auto builtin = l->get_builtin_enter();
            if (!builtin_has_keyed_args(builtin))
                verify_no_keyed_args(l);
            if (builtin_always_folds(builtin)) {
                if (fold_builtin_call(l)) {
                    goto repeat;
                }
            } else if (builtin == FN_Branch) {
                type_branch_continuations(l);
                l->body.set_complete();
                // todo: process branches
                return l;
            } else {
                auto &&enter = l->body.enter;
                auto &&args = l->body.args;
                assert(enter.type == TYPE_Builtin);
                if ((enter.builtin == SFXFN_Unreachable)
                    || (enter.builtin == SFXFN_Discard)) {
                    args[0] = none;
                } else {
                    Args values;
                    bool fold = values_from_builtin_call(l, values);
                    if (fold) {
                        enter = args[0].value;
                        args = { none };
                        for (size_t i = 0; i < values.size(); ++i) {
                            args.push_back(values[i]);
                        }
                        goto repeat;
                    } else {
                        rtype = ReturnLabel(values);
                    }
                }
            }
        } else if (is_calling_closure(l)) {
            if (has_keyed_args(l)) {
                solve_keyed_args(l);
            }
            bool recursive = false;
            bool inlined = false;
            rtype = fold_closure_call(l, recursive, inlined);
            if (recursive) {
                return nullptr;
            }
            if (inlined) {
                return l;
            }
        } else if (is_calling_continuation(l)) {
            rtype = get_return_type_from_call_arguments(l);
        } else {
            StyledString ss;
            auto &&enter = l->body.enter;
            if (!enter.is_const()) {
                ss.out << "unable to call variable of type " << enter.indirect_type();
            } else {
                ss.out << "unable to call constant of type " << enter.type;
            }
            location_error(ss.str());
        }

        if (rtype) {
            if (is_jumping(l)) {
                l->body.enter = fold_type_return(l->body.enter, rtype);
            } else {
                assert(!l->body.args.empty());
                l->body.args[0] = fold_type_return(l->body.args[0].value, rtype);
            }
        }

        l->body.set_complete();

        if (fold_useless_labels(l)) {
            goto repeat;
        }

        bool is_jumping = jumps_immediately(l);
        if (is_jumping) {
            clear_continuation_arg(l);
        }
        if (!l->is_important() && l->is_basic_block_like()) {
            if (is_jumping) {
                Label *nextl = l->body.enter;
                if (!l->has_params() && !nextl->has_params())
                    return nextl;
            }
        }
        return l;
    }

    Label *skip_useless_labels(Label *l, LabelQueue &todo, LabelQueue &recursions) {
    repeat:
        Label *nextl = l;
        if(!l->body.is_complete()) {
            nextl = fold_label_body(l);
            if (!nextl) { // recursive, try again later
                recursions.push_front(l);
                return l;
            }
        }
        if (nextl == l) {
            if (!l->body.is_optimized()) {
                todo.push_back(l);
            }
            return l;
        } else {
            l = nextl;
            goto repeat;
        }
    }

    // normalize all labels in the scope of a function with entry point l
    void normalize_function(Frame *f) {
        Label *l = f->get_instance();
        if (l->body.is_complete())
            return;
        verify_stack_size();
        assert(!l->is_basic_block_like());
        Label *entry_label = l;
        // second stack of recursive calls that can't be typed yet
        LabelQueue recursions;
        LabelQueue todo;
        todo.push_back(l);

        SCOPES_TRY()

        while (true) {
            if (todo.empty()) {
                if (recursions.empty())
                    break;
                entry_label->set_reentrant();
                // all directly reachable branches have been processed
                // check that entry_label is typed at this point
                // if it is not, we are recursing infinitely
                if (!entry_label->is_return_param_typed()) {
                    set_active_anchor(entry_label->anchor);
                    location_error(String::from("recursive function never returns"));
                }
                todo = recursions;
                recursions.clear();
            }
            l = todo.back();
            todo.pop_back();
            if(!l->body.is_complete()) {
                if (!fold_label_body(l)) {
                    recursions.push_front(l);
                    continue;
                }
            }
            if (!l->body.is_optimized()) {
                l->body.set_optimized();
                if (jumps_immediately(l)) {
                    l->body.enter = skip_useless_labels(l->body.enter, todo, recursions);
                } else if (is_continuing_to_label(l)) {
                    l->body.args[0].value = skip_useless_labels(l->body.args[0].value, todo, recursions);
                } else if (is_branching(l)) {
                    l->body.args[2].value = skip_useless_labels(l->body.args[2].value, todo, recursions);
                    l->body.args[3].value = skip_useless_labels(l->body.args[3].value, todo, recursions);
                }
            }
        }

        SCOPES_CATCH(exc)
            traceback.push_back(Trace(l->body.anchor, l->name));
            if (entry_label != l) {
                traceback.push_back(Trace(entry_label->anchor, entry_label->name));
            }
            error(exc);
        SCOPES_TRY_END()

        assert(!entry_label->params.empty());
        assert(entry_label->body.is_complete());

        // function is done, but not typed
        if (!entry_label->is_return_param_typed()) {
            // two possible reasons for this:
            // 1. recursion - entry label has already been
            //    processed, but not exited yet, so we don't have the
            //    continuation type yet.
            //    -- but in that case, we're never arriving here!
            //if (f->is_reentrant())
            // 2. all return paths are unreachable, because the function
            //    never returns.
            fold_type_return(entry_label->params[0], NoReturnLabel());
        } else {
            validate_label_return_types(entry_label);
        }
    }

    Label *validate_scope(Label *entry) {
        Timer validate_scope_timer(TIMER_ValidateScope);

        std::unordered_set<Label *> visited;
        Labels labels;
        entry->build_reachable(visited, &labels);

        Label::UserMap um;
        for (auto it = labels.begin(); it != labels.end(); ++it) {
            (*it)->insert_into_usermap(um);
        }

        for (auto it = labels.begin(); it != labels.end(); ++it) {
            Label *l = *it;
            if (l->is_basic_block_like()) {
                continue;
            }
            Labels scope;
            l->build_scope(um, scope);
            for (size_t i = 0; i < scope.size(); ++i) {
                Label *subl = scope[i];
                if (!subl->is_basic_block_like()) {
                    assert(!subl->is_inline());
                    location_message(l->anchor, String::from("depends on this scope"));
                    set_active_anchor(subl->anchor);
                    location_error(String::from("expression using variable in exterior scope as well as provider of variable must be inline"));
                }
            }
        }

        return entry;
    }

    std::unordered_map<const Type *, ffi_type *> ffi_types;

    ffi_type *new_type() {
        ffi_type *result = (ffi_type *)malloc(sizeof(ffi_type));
        memset(result, 0, sizeof(ffi_type));
        return result;
    }

    ffi_type *create_ffi_type(const Type *type) {
        if (type == TYPE_Void) return &ffi_type_void;
        if (type == TYPE_Nothing) return &ffi_type_void;

        switch(type->kind()) {
        case TK_Integer: {
            auto it = cast<IntegerType>(type);
            if (it->issigned) {
                switch (it->width) {
                case 8: return &ffi_type_sint8;
                case 16: return &ffi_type_sint16;
                case 32: return &ffi_type_sint32;
                case 64: return &ffi_type_sint64;
                default: break;
                }
            } else {
                switch (it->width) {
                case 1: return &ffi_type_uint8;
                case 8: return &ffi_type_uint8;
                case 16: return &ffi_type_uint16;
                case 32: return &ffi_type_uint32;
                case 64: return &ffi_type_uint64;
                default: break;
                }
            }
        } break;
        case TK_Real: {
            switch(cast<RealType>(type)->width) {
            case 32: return &ffi_type_float;
            case 64: return &ffi_type_double;
            default: break;
            }
        } break;
        case TK_Pointer: return &ffi_type_pointer;
        case TK_Typename: {
            return get_ffi_type(storage_type(type));
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            size_t count = ai->count;
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * (count + 1));
            ffi_type *element_type = get_ffi_type(ai->element_type);
            for (size_t i = 0; i < count; ++i) {
                ty->elements[i] = element_type;
            }
            ty->elements[count] = nullptr;
            return ty;
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(type);
            size_t count = ti->types.size();
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * (count + 1));
            for (size_t i = 0; i < count; ++i) {
                ty->elements[i] = get_ffi_type(ti->types[i]);
            }
            ty->elements[count] = nullptr;
            return ty;
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            size_t count = ui->types.size();
            size_t sz = ui->size;
            size_t al = ui->align;
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            // find member with the same alignment
            for (size_t i = 0; i < count; ++i) {
                const Type *ET = ui->types[i];
                size_t etal = align_of(ET);
                if (etal == al) {
                    size_t remsz = sz - size_of(ET);
                    ffi_type *tvalue = get_ffi_type(ET);
                    if (remsz) {
                        ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * 3);
                        ty->elements[0] = tvalue;
                        ty->elements[1] = get_ffi_type(Array(TYPE_I8, remsz));
                        ty->elements[2] = nullptr;
                    } else {
                        ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * 2);
                        ty->elements[0] = tvalue;
                        ty->elements[1] = nullptr;
                    }
                    return ty;
                }
            }
            // should never get here
            assert(false);
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "FFI: cannot convert argument of type " << type;
        location_error(ss.str());
        return nullptr;
    }

    ffi_type *get_ffi_type(const Type *type) {
        auto it = ffi_types.find(type);
        if (it == ffi_types.end()) {
            auto result = create_ffi_type(type);
            ffi_types[type] = result;
            return result;
        } else {
            return it->second;
        }
    }

    void verify_function_argument_count(const FunctionType *fi, size_t argcount) {

        const Type *T = fi;

        size_t fargcount = fi->argument_types.size();
        if (fi->flags & FF_Variadic) {
            if (argcount < fargcount) {
                StyledString ss;
                ss.out << "argument count mismatch for call to function of type "
                    << T << " (need at least " << fargcount << ", got " << argcount << ")";
                location_error(ss.str());
            }
        } else {
            if (argcount != fargcount) {
                StyledString ss;
                ss.out << "argument count mismatch for call to function of type "
                    << T << " (need " << fargcount << ", got " << argcount << ")";
                location_error(ss.str());
            }
        }
    }

    void *extract_compile_time_pointer(Any value) {
        switch(value.type->kind()) {
        case TK_Extern: {
            void *ptr = local_aware_dlsym(value.symbol);
            if (!ptr) {
                StyledString ss;
                ss.out << "could not resolve external at compile time: " << value;
                location_error(ss.str());
            }
            return ptr;
        } break;
        case TK_Pointer: {
            return value.pointer;
        } break;
        default: assert(false && "unexpected pointer type");
            return nullptr;
        }
    }

    Any run_ffi_function(Any enter, Argument *args, size_t argcount) {
        auto fi = extract_function_type(enter.type);

        size_t fargcount = fi->argument_types.size();

        const Type *rettype = cast<ReturnLabelType>(fi->return_type)->return_type;

        ffi_cif cif;
        ffi_type *argtypes[argcount];
        void *avalues[argcount];
        for (size_t i = 0; i < argcount; ++i) {
            Argument &arg = args[i];
            argtypes[i] = get_ffi_type(arg.value.type);
            avalues[i] = get_pointer(arg.value.type, arg.value);
        }
        ffi_status prep_result;
        if (fi->flags & FF_Variadic) {
            prep_result = ffi_prep_cif_var(
                &cif, FFI_DEFAULT_ABI, fargcount, argcount, get_ffi_type(rettype), argtypes);
        } else {
            prep_result = ffi_prep_cif(
                &cif, FFI_DEFAULT_ABI, argcount, get_ffi_type(rettype), argtypes);
        }
        assert(prep_result == FFI_OK);

        Any result = Any::from_pointer(rettype, nullptr);
        void *ptr = extract_compile_time_pointer(enter);
        ffi_call(&cif, FFI_FN(ptr),
            get_pointer(result.type, result, true), avalues);
        return result;
    }

};

Specializer::Traces Specializer::traceback;
int Specializer::solve_refs = 0;
bool Specializer::enable_step_debugger = false;
Specializer::CLICmd Specializer::clicmd = CmdNone;

Label *specialize(Frame *frame, Label *label, const ArgTypes &argtypes) {
    Specializer solver;
    return solver.typify(frame, label, argtypes);
}

void enable_specializer_step_debugger() {
    Specializer::enable_step_debugger = true;
}

} // namespace scopes

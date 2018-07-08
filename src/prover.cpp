/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "prover.hpp"
#include "value.hpp"
#include "types.hpp"
#include "error.hpp"
#include "closure.hpp"
#include "stream_ast.hpp"
#include "hash.hpp"
#include "timer.hpp"
#include "gc.hpp"
#include "builtin.hpp"
#include "verify_tools.inc"
#include "dyn_cast.inc"
#include "coro/coro.h"
#include "compiler_flags.hpp"
#include "gen_llvm.hpp"
#include "list.hpp"
#include "expander.hpp"

#include <unordered_set>
#include <deque>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

#define SCOPES_ARITH_OPS() \
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

//------------------------------------------------------------------------------

namespace FunctionSet {
    struct Hash {
        std::size_t operator()(const Function *s) const {
            std::size_t h = std::hash<Function *>{}(s->frame);
            h = hash2(h, std::hash<Template *>{}(s->original));
            for (auto arg : s->instance_args) {
                h = hash2(h, std::hash<const Type *>{}(arg));
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const Function *lhs, const Function *rhs ) const {
            if (lhs->frame != rhs->frame) return false;
            if (lhs->original != rhs->original) return false;
            if (lhs->instance_args.size() != rhs->instance_args.size()) return false;
            for (size_t i = 0; i < lhs->instance_args.size(); ++i) {
                auto lparam = lhs->instance_args[i];
                auto rparam = rhs->instance_args[i];
                if (lparam != rparam) return false;
            }
            return true;
        }
    };
} // namespace FunctionSet

static std::unordered_set<Function *, FunctionSet::Hash, FunctionSet::KeyEqual> functions;

//------------------------------------------------------------------------------

// reduce typekind to compatible
static TypeKind canonical_typekind(TypeKind k) {
    if (k == TK_Real)
        return TK_Integer;
    return k;
}

//------------------------------------------------------------------------------

enum EvalTarget {
    EvalTarget_Void,
    EvalTarget_Symbol,
    EvalTarget_Return,
};

struct ASTContext {
    Function *frame;
    EvalTarget target;
    Loop *loop;

    const Type *transform_return_type(const Type *T) const {
        if (is_returning(T) && is_target_void())
            return TYPE_Void;
        return T;
    }

    bool is_target_void() const {
        return target == EvalTarget_Void;
    }

    ASTContext with_target(EvalTarget target) const {
        return ASTContext(frame, target, loop);
    }

    ASTContext for_loop(Loop *loop) const {
        return ASTContext(frame, EvalTarget_Symbol, loop);
    }

    ASTContext() {}

    ASTContext(Function *_frame, EvalTarget _target, Loop *_loop = nullptr) :
        frame(_frame), target(_target), loop(_loop) {
    }
};

// returns
static SCOPES_RESULT(Value *) specialize(const ASTContext &ctx, Value *node);

struct SpecializeJob {
    ASTContext ctx;
    Value *node;
    Result<Value *> result;
    coro_stack stack;
    coro_context from;
    coro_context job;
    bool done;
};

static std::deque<SpecializeJob *> jobs;

static int process_jobs() {
    int processed = 0;
    while (!jobs.empty()) {
        auto job = jobs.front();
        jobs.pop_front();
        coro_create(&job->from, nullptr, nullptr, nullptr, 0);
        coro_transfer(&job->from, &job->job);
        processed++;
    }
    return processed;
}

static void specialize_coroutine(void *ptr) {
    SpecializeJob *job = (SpecializeJob *)ptr;
    StyledStream ss;
    ss << "processing: ";
    stream_ast(ss, job->node, StreamASTFormat());
    job->result = specialize(job->ctx, job->node);
    job->done = true;
    coro_transfer(&job->job, &job->from);
}

static bool wait_for_return_type(Function *f) {
    // do more branches and try again
    int processed = process_jobs();
    return (f->return_type != nullptr);
}

static SCOPES_RESULT(void) specialize_jobs(const ASTContext &ctx, int count, Value **nodes) {
    SCOPES_RESULT_TYPE(void);
    SpecializeJob local_jobs[count];
    for (int i = 0; i < count; ++i) {
        auto &&job = local_jobs[i];
        job.ctx = ctx;
        job.node = nodes[i];
        job.done = false;
        coro_stack_alloc(&job.stack, 0);
        coro_create(&job.job, specialize_coroutine, &job, job.stack.sptr, job.stack.ssze);
        jobs.push_back(&job);
    }
    process_jobs();
    for (int i = 0; i < count; ++i) {
        assert(local_jobs[i].done);
    }
    for (int i = 0; i < count; ++i) {
        auto &&job = local_jobs[i];
        coro_destroy(&job.job);
        coro_stack_free(&job.stack);
    }
    for (int i = 0; i < count; ++i) {
        auto &&job = local_jobs[i];
        auto result = job.result;
        nodes[i] = SCOPES_GET_RESULT(job.result);
    }
    return true;
}

static SCOPES_RESULT(Value *) specialize_inline(const ASTContext &ctx, Function *frame, Template *func, const Values &nodes);

static SCOPES_RESULT(const Type *) merge_value_type(const ASTContext &ctx, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(const Type *);
    assert(T2);
    T2 = ctx.transform_return_type(T2);
    if (!T1)
        return T2;
    if (T1 == T2)
        return T1;
    if (!is_returning(T1))
        return T2;
    if (!is_returning(T2))
        return T1;
    SCOPES_EXPECT_ERROR(error_cannot_merge_expression_types(T1, T2));
}

static SCOPES_RESULT(const Type *) merge_return_type(const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(const Type *);
    assert(T2);
    if (!T1)
        return T2;
    if (T1 == T2)
        return T1;
    if (!is_returning(T1))
        return T2;
    if (!is_returning(T2))
        return T1;
    SCOPES_EXPECT_ERROR(error_cannot_merge_expression_types(T1, T2));
}

static bool is_useless(Value *node) {
    if (Const::classof(node)) return true;
    switch(node->kind()) {
    case VK_Template:
    case VK_Function:
    case VK_Symbol:
        return true;
    case VK_Let: {
        auto let = cast<Let>(node);
        if (!let->params.size())
            return true;
    } break;
    default: break;
    }
    return false;
}

static SCOPES_RESULT(Value *) specialize_Block(const ASTContext &ctx, Block *block) {
    SCOPES_RESULT_TYPE(Value *);
    Block *newblock = Block::from(block->anchor());
    auto subctx = ctx.with_target(EvalTarget_Void);
    for (auto &&src : block->body) {
        auto newsrc = SCOPES_GET_RESULT(specialize(subctx, src));
        if (!is_returning(newsrc->get_type())) {
            set_active_anchor(newsrc->anchor());
            SCOPES_CHECK_RESULT(error_noreturn_not_last_expression());
        }
        if (!is_useless(newsrc)) {
            newblock->append(newsrc);
        }
    }
    newblock->value = SCOPES_GET_RESULT(specialize(ctx, block->value));
    auto rtype = ctx.transform_return_type(newblock->value->get_type());
    newblock->set_type(rtype);
    return newblock->canonicalize();
}

static Value *extract_argument(Value *value, int index) {
    const Anchor *anchor = value->anchor();
    const Type *T = value->get_type();
    if (!is_returning(T))
        return value;
    auto rt = dyn_cast<ReturnType>(value->get_type());
    if (rt) {
        const Type *T = rt->type_at_index(index);
        if (T == TYPE_Nothing) {
            return ConstTuple::none_from(anchor);
        } else {
            auto arglist = dyn_cast<ArgumentList>(value);
            if (arglist) {
                return arglist->values[index];
            } else {
                auto result = ExtractArgument::from(anchor, value, index);
                result->set_type(T);
                return result;
            }
        }
    } else if (index == 0) {
        return value;
    } else {
        return ConstTuple::none_from(anchor);
    }
}

// used by Let, Loop, ArgumentList, Repeat, Call
static SCOPES_RESULT(void) specialize_arguments(
    const ASTContext &ctx, Values &outargs, const Values &values) {
    SCOPES_RESULT_TYPE(void);
    auto subctx = ctx.with_target(EvalTarget_Symbol);
    int count = (int)values.size();
    for (int i = 0; i < count; ++i) {
        auto value = SCOPES_GET_RESULT(specialize(subctx, values[i]));
        const Type *T = value->get_type();
        auto rt = dyn_cast<ReturnType>(T);
        if (rt) {
            if (!rt->is_returning()) {
                SCOPES_EXPECT_ERROR(error_noreturn_not_last_expression());
            }
            if ((i + 1) == count) {
                // last argument is appended in full
                int starti = (rt->is_raising()?1:0);
                int valcount = (int)rt->values.size();
                for (int j = starti; j < valcount; ++j) {
                    outargs.push_back(extract_argument(value, j));
                }
                break;
            } else {
                value = extract_argument(value, 0);
            }
        }
        outargs.push_back(value);
    }
    return true;
}

static const Type *return_type_from_arguments(const Values &values) {
    ArgTypes types;
    for (auto arg : values) {
        types.push_back(arg->get_type());
    }
    return return_type(types);
}

static SCOPES_RESULT(Value *) specialize_ArgumentList(const ASTContext &ctx, ArgumentList *nlist) {
    SCOPES_RESULT_TYPE(Value *);
    Values values;
    SCOPES_CHECK_RESULT(specialize_arguments(ctx, values, nlist->values));
    if (values.size() == 1) {
        return values[0];
    }
    ArgumentList *newnlist = ArgumentList::from(nlist->anchor(), values);
    newnlist->set_type(return_type_from_arguments(values));
    return newnlist;
}

static SCOPES_RESULT(Value *) specialize_ExtractArgument(
    const ASTContext &ctx, ExtractArgument *node) {
    SCOPES_RESULT_TYPE(Value *);
    auto value = SCOPES_GET_RESULT(specialize(ctx, node->value));
    return extract_argument(value, node->index);
}

// used by Let and Loop
static SCOPES_RESULT(void) specialize_bind_arguments(const ASTContext &ctx,
    SymbolValues &outparams, Values &outargs,
    const SymbolValues &params, const Values &values) {
    SCOPES_RESULT_TYPE(void);
    Values tmpargs;
    SCOPES_CHECK_RESULT(specialize_arguments(ctx, tmpargs, values));
    int count = (int)params.size();
    for (int i = 0; i < count; ++i) {
        auto oldsym = params[i];
        Value *newval = nullptr;
        if (oldsym->is_variadic()) {
            if ((i + 1) < count) {
                set_active_anchor(oldsym->anchor());
                SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
            }
            if ((i + 1) == (int)tmpargs.size()) {
                newval = tmpargs[i];
            } else {
                auto arglist = ArgumentList::from(oldsym->anchor());
                for (int j = i; j < tmpargs.size(); ++j) {
                    arglist->append(tmpargs[j]);
                }
                arglist->set_type(return_type_from_arguments(arglist->values));
                newval = arglist;
            }
        } else if (i < tmpargs.size()) {
            newval = tmpargs[i];
        } else {
            newval = ConstTuple::none_from(oldsym->anchor());
        }
        auto newsym = SymbolValue::from(oldsym->anchor(), oldsym->name, newval->get_type());
        ctx.frame->bind(oldsym, newsym);
        outparams.push_back(newsym);
        outargs.push_back(newval);
    }
    return true;
}

static SCOPES_RESULT(Value *) specialize_Let(const ASTContext &ctx, Let *let) {
    SCOPES_RESULT_TYPE(Value *);
    Let *newlet = Let::from(let->anchor());
    SCOPES_CHECK_RESULT(specialize_bind_arguments(ctx,
        newlet->params, newlet->args, let->params, let->args));
    newlet->set_type(TYPE_Void);
    return newlet;
}

static SCOPES_RESULT(Loop *) specialize_Loop(const ASTContext &ctx, Loop *loop) {
    SCOPES_RESULT_TYPE(Loop *);
    Loop *newloop = Loop::from(loop->anchor());
    SCOPES_CHECK_RESULT(specialize_bind_arguments(ctx,
        newloop->params, newloop->args, loop->params, loop->args));
    newloop->value = SCOPES_GET_RESULT(specialize(ctx.for_loop(newloop), loop->value));
    auto rtype = newloop->value->get_type();
    newloop->return_type = SCOPES_GET_RESULT(merge_value_type(ctx, newloop->return_type, rtype));
    newloop->set_type(newloop->return_type);
    return newloop;
}

#define CONST_SPECIALIZER(NAME) \
    static SCOPES_RESULT(Value *) specialize_ ## NAME(const ASTContext &ctx, NAME *node) { return node; }

CONST_SPECIALIZER(ConstInt)
CONST_SPECIALIZER(ConstReal)
CONST_SPECIALIZER(ConstPointer)
CONST_SPECIALIZER(ConstTuple)
CONST_SPECIALIZER(ConstArray)
CONST_SPECIALIZER(ConstVector)
CONST_SPECIALIZER(Extern)

const Type *try_get_const_type(Value *node) {
    if (isa<Const>(node))
        return node->get_type();
    return TYPE_Unknown;
}

const String *try_extract_string(Value *node) {
    auto ptr = dyn_cast<ConstPointer>(node);
    if (ptr && (ptr->get_type() == TYPE_String))
        return (const String *)ptr->value;
    return nullptr;
}

static SCOPES_RESULT(Break *) specialize_Break(const ASTContext &ctx, Break *_break) {
    SCOPES_RESULT_TYPE(Break *);
    if (!ctx.loop) {
        set_active_anchor(_break->anchor());
        SCOPES_EXPECT_ERROR(error_illegal_break_outside_loop());
    }
    auto subctx = ctx.with_target(EvalTarget_Symbol);
    Value *value = SCOPES_GET_RESULT(specialize(subctx, _break->value));
    ctx.loop->return_type = SCOPES_GET_RESULT(merge_value_type(subctx, ctx.loop->return_type, value->get_type()));
    auto newbreak = Break::from(_break->anchor(), value);
    newbreak->set_type(no_return_type());
    return newbreak;
}

static SCOPES_RESULT(Repeat *) specialize_Repeat(const ASTContext &ctx, Repeat *_repeat) {
    SCOPES_RESULT_TYPE(Repeat *);
    if (!ctx.loop) {
        set_active_anchor(_repeat->anchor());
        SCOPES_EXPECT_ERROR(error_illegal_repeat_outside_loop());
    }
    auto newrepeat = Repeat::from(_repeat->anchor());
    SCOPES_CHECK_RESULT(specialize_arguments(ctx, newrepeat->args, _repeat->args));
    newrepeat->set_type(no_return_type());
    return newrepeat;
}

static SCOPES_RESULT(Return *) make_return(const ASTContext &ctx, const Anchor *anchor, Value *value) {
    SCOPES_RESULT_TYPE(Return *);
    assert(ctx.frame);
    if (ctx.frame->original
        && ctx.frame->original->is_inline()) {
        set_active_anchor(anchor);
        SCOPES_EXPECT_ERROR(error_illegal_return_in_inline());
    }
    ctx.frame->return_type = SCOPES_GET_RESULT(merge_return_type(ctx.frame->return_type, value->get_type()));
    auto newreturn = Return::from(anchor, value);
    newreturn->set_type(no_return_type());
    return newreturn;
}

static SCOPES_RESULT(Value *) specialize_Return(const ASTContext &ctx, Return *_return) {
    SCOPES_RESULT_TYPE(Value *);
    Value *value = SCOPES_GET_RESULT(specialize(ctx.with_target(EvalTarget_Symbol), _return->value));
    if (ctx.target == EvalTarget_Return) {
        return value;
    }
    return SCOPES_GET_RESULT(make_return(ctx, _return->anchor(), value));
}

static SCOPES_RESULT(Value *) specialize_SyntaxExtend(const ASTContext &ctx, SyntaxExtend *sx) {
    SCOPES_RESULT_TYPE(Value *);
    assert(sx->func->scope);
    Function *frame = ctx.frame->find_frame(sx->func->scope);
    if (!frame) {
        set_active_anchor(sx->func->anchor());
        SCOPES_EXPECT_ERROR(error_cannot_find_frame(sx->func));
    }
    Function *fn = SCOPES_GET_RESULT(specialize(frame, sx->func, {TYPE_Scope}));
    //StyledStream ss;
    //stream_ast(ss, fn, StreamASTFormat());
    auto ftype = native_ro_pointer_type(function_type(TYPE_Scope, {TYPE_Scope}));
    const void *ptr = SCOPES_GET_RESULT(compile(fn, 0/*CF_DumpModule*/))->value;
    Scope *env = nullptr;
    if (fn->get_type() == ftype) {
        typedef Scope *(*SyntaxExtendFuncType)(Scope *);
        SyntaxExtendFuncType fptr = (SyntaxExtendFuncType)ptr;
        env = fptr(sx->env);
        assert(env);
    } else {
        set_active_anchor(sx->anchor());
        StyledString ss;
        ss.out << "syntax-extend has wrong return type (expected function of type "
            << ftype
            //<< " or "
            //<< ReturnLabel({unknown_of(TYPE_Scope)}, RLF_Raising)
            << ", got " << fn->get_type() << ")";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    auto anchor = sx->next?sx->next->at->anchor():fn->anchor();
    auto nextfn = SCOPES_GET_RESULT(expand_inline(
        ctx.frame->original,
        ConstPointer::list_from(anchor, sx->next), env));
    return specialize(ctx, nextfn->value);
}

static SCOPES_RESULT(Keyed *) specialize_Keyed(const ASTContext &ctx, Keyed *keyed) {
    SCOPES_RESULT_TYPE(Keyed *);
    return Keyed::from(keyed->anchor(), keyed->key,
        SCOPES_GET_RESULT(specialize(ctx, keyed->value)));
}

template<typename T>
SCOPES_RESULT(T *) extract_constant(Value *value) {
    SCOPES_RESULT_TYPE(T *);
    auto constval = dyn_cast<T>(value);
    if (!constval) {
        set_active_anchor(value->anchor());
        SCOPES_CHECK_RESULT(error_constant_expected(value));
    }
    return constval;
}

SCOPES_RESULT(const Type *) extract_type_constant(Value *value) {
    SCOPES_RESULT_TYPE(const Type *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_constant<ConstPointer>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_Type));
    return (const Type *)x->value;
}

SCOPES_RESULT(const Closure *) extract_closure_constant(Value *value) {
    SCOPES_RESULT_TYPE(const Closure *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_constant<ConstPointer>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_Closure));
    return (const Closure *)x->value;
}

SCOPES_RESULT(const List *) extract_list_constant(Value *value) {
    SCOPES_RESULT_TYPE(const List *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_constant<ConstPointer>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_List));
    return (const List *)x->value;
}

SCOPES_RESULT(const String *) extract_string_constant(Value *value) {
    SCOPES_RESULT_TYPE(const String *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_constant<ConstPointer>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_String));
    return (const String *)x->value;
}

SCOPES_RESULT(Builtin) extract_builtin_constant(Value *value) {
    SCOPES_RESULT_TYPE(Builtin);
    ConstInt* x = SCOPES_GET_RESULT(extract_constant<ConstInt>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_Builtin));
    return Builtin((KnownSymbol)x->value);
}

SCOPES_RESULT(Symbol) extract_symbol_constant(Value *value) {
    SCOPES_RESULT_TYPE(Symbol);
    ConstInt* x = SCOPES_GET_RESULT(extract_constant<ConstInt>(value));
    set_active_anchor(value->anchor());
    SCOPES_CHECK_RESULT(verify(x->get_type(), TYPE_Symbol));
    return Symbol::wrap(x->value);
}

SCOPES_RESULT(uint64_t) extract_integer_constant(Value *value) {
    SCOPES_RESULT_TYPE(uint64_t);
    ConstInt* x = SCOPES_GET_RESULT(extract_constant<ConstInt>(value));
    return x->value;
}

static SCOPES_RESULT(const Type *) bool_op_return_type(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    T = SCOPES_GET_RESULT(storage_type(T));
    if (T->kind() == TK_Vector) {
        auto vi = cast<VectorType>(T);
        return vector_type(TYPE_Bool, vi->count);
    } else {
        return TYPE_Bool;
    }
}

static SCOPES_RESULT(void) verify_integer_ops(const Type *x) {
    SCOPES_RESULT_TYPE(void);
    return verify_integer_vector(SCOPES_GET_RESULT(storage_type(x)));
}

static SCOPES_RESULT(void) verify_real_ops(const Type *x) {
    SCOPES_RESULT_TYPE(void);
    return verify_real_vector(SCOPES_GET_RESULT(storage_type(x)));
}

static SCOPES_RESULT(void) verify_integer_ops(const Type *a, const Type *b) {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(verify_integer_vector(SCOPES_GET_RESULT(storage_type(a))));
    return verify(a, b);
}

static SCOPES_RESULT(void) verify_real_ops(const Type *a, const Type *b) {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(verify_real_vector(SCOPES_GET_RESULT(storage_type(a))));
    return verify(a, b);
}

static SCOPES_RESULT(void) verify_real_ops(const Type *a, const Type *b, const Type *c) {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(verify_real_vector(SCOPES_GET_RESULT(storage_type(a))));
    SCOPES_CHECK_RESULT(verify(a, b));
    return verify(a, c);
}

#define CHECKARGS(MINARGS, MAXARGS) \
    SCOPES_CHECK_RESULT((checkargs<MINARGS, MAXARGS>(argcount)))
#define RETARGTYPES(...) \
    { \
        Call *newcall = Call::from(call->anchor(), callee, values); \
        newcall->set_type(return_type({ __VA_ARGS__ })); \
        return newcall; \
    }
#define READ_TYPEOF(NAME) \
        assert(argn <= argcount); \
        auto _ ## NAME = values[argn++]; \
        const Type *NAME = _ ## NAME->get_type();
#define READ_STORAGETYPEOF(NAME) \
        assert(argn <= argcount); \
        const Type *NAME = SCOPES_GET_RESULT(storage_type(values[argn++]->get_type()));
#define READ_INT_CONST(NAME) \
        assert(argn <= argcount); \
        auto NAME = SCOPES_GET_RESULT(extract_integer_constant(values[argn++]));
#define READ_TYPE_CONST(NAME) \
        assert(argn <= argcount); \
        auto NAME = SCOPES_GET_RESULT(extract_type_constant(values[argn++]));

static const Type *get_function_type(Function *fn) {
    ArgTypes params;
    for (int i = 0; i < fn->params.size(); ++i) {
        params.push_back(fn->params[i]->get_type());
    }
    return native_ro_pointer_type(function_type(fn->return_type, params));
}

static SCOPES_RESULT(Value *) specialize_Call(const ASTContext &ctx, Call *call) {
    SCOPES_RESULT_TYPE(Value *);
    auto subctx = ctx.with_target(EvalTarget_Symbol);
    Value *callee = SCOPES_GET_RESULT(specialize(subctx, call->callee));
    Values values;
    SCOPES_CHECK_RESULT(specialize_arguments(ctx, values, call->args));
    const Type *T = callee->get_type();
    if (T == TYPE_Closure) {
        const Closure *cl = SCOPES_GET_RESULT((extract_closure_constant(callee)));
        if (cl->func->is_inline()) {
            return SCOPES_GET_RESULT(specialize_inline(ctx, cl->frame, cl->func, values));
        } else {
            ArgTypes types;
            for (auto &&arg : values) {
                types.push_back(arg->get_type());
            }
            callee = SCOPES_GET_RESULT(specialize(cl->frame, cl->func, types));
            Function *f = cast<Function>(callee);
            if (f->complete) {
                T = callee->get_type();
            } else if (f->return_type) {
                T = get_function_type(f);
            } else {
                if (wait_for_return_type(f)) {
                    T = get_function_type(f);
                } else {
                    set_active_anchor(call->anchor());
                    SCOPES_EXPECT_ERROR(error_untyped_recursive_call());
                }
            }
        }
    } else if (T == TYPE_Builtin) {
        //SCOPES_CHECK_RESULT(anycl.verify(TYPE_Builtin));
        Builtin b = SCOPES_GET_RESULT(extract_builtin_constant(callee));
        size_t argcount = values.size();
        size_t argn = 0;
        set_active_anchor(call->anchor());
        switch(b.value()) {
        case FN_Dump: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump:";
            for (auto arg : values) {
                ss << " ";
                stream_ast(ss, arg, StreamASTFormat());
            }
        } break;
        case FN_Undef: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            RETARGTYPES(T);
        } break;
        case FN_TypeOf: {
            CHECKARGS(1, 1);
            READ_TYPEOF(A);
            return ConstPointer::type_from(call->anchor(), A);
        } break;
        case FN_Bitcast: {
            CHECKARGS(2, 2);
            READ_TYPEOF(SrcT);
            READ_TYPE_CONST(DestT);
            if (SrcT == DestT) {
                return _SrcT;
            } else {
                const Type *SSrcT = SCOPES_GET_RESULT(storage_type(SrcT));
                const Type *SDestT = SCOPES_GET_RESULT(storage_type(DestT));
                if (canonical_typekind(SSrcT->kind())
                        != canonical_typekind(SDestT->kind())) {
                    StyledString ss;
                    ss.out << "can not bitcast value of type " << SrcT
                        << " to type " << DestT
                        << " because storage types are not of compatible category";
                    SCOPES_LOCATION_ERROR(ss.str());
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
                        SCOPES_LOCATION_ERROR(ss.str());
                    } break;
                    default: break;
                    }
                }
                RETARGTYPES(DestT);
            }
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT((verify_kind<TK_Pointer>(SCOPES_GET_RESULT(storage_type(DestT)))));
            RETARGTYPES(DestT);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            RETARGTYPES(DestT);
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_INT_CONST(idx);
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                RETARGTYPES(SCOPES_GET_RESULT(ai->type_at_index(idx)));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                RETARGTYPES(SCOPES_GET_RESULT(ti->type_at_index(idx)));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                RETARGTYPES(SCOPES_GET_RESULT(ui->type_at_index(idx)));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            }
        } break;
        case FN_InsertValue: {
            CHECKARGS(3, 3);
            READ_TYPEOF(AT);
            READ_STORAGETYPEOF(ET);
            READ_INT_CONST(idx);
            auto T = SCOPES_GET_RESULT(storage_type(AT));
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                SCOPES_CHECK_RESULT(verify(SCOPES_GET_RESULT(storage_type(SCOPES_GET_RESULT(ai->type_at_index(idx)))), ET));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                SCOPES_CHECK_RESULT(verify(SCOPES_GET_RESULT(storage_type(SCOPES_GET_RESULT(ti->type_at_index(idx)))), ET));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                SCOPES_CHECK_RESULT(verify(SCOPES_GET_RESULT(storage_type(SCOPES_GET_RESULT(ui->type_at_index(idx)))), ET));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not insert value into type " << T;
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            }
            RETARGTYPES(AT);
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
            READ_TYPEOF(A); READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B));
            RETARGTYPES(SCOPES_GET_RESULT(bool_op_return_type(A)));
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
            READ_TYPEOF(A); READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_real_ops(A, B));
            RETARGTYPES(SCOPES_GET_RESULT(bool_op_return_type(A)));
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
        case OP_ ## NAME: \
        case OP_ ## NAME ## NUW: \
        case OP_ ## NAME ## NSW: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define IARITH_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define FARITH_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define FTRI_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(3, 3); \
            READ_TYPEOF(A); READ_TYPEOF(B); READ_TYPEOF(C); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B, C)); \
            RETARGTYPES(A); \
        } break;
#define IUN_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A)); \
            RETARGTYPES(A); \
        } break;
#define FUN_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_real_ops(A)); \
            RETARGTYPES(A); \
        } break;
        SCOPES_ARITH_OPS()

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP
        default: {
            SCOPES_EXPECT_ERROR(error_cannot_type_builtin(b));
        } break;
        }

        return specialize(ctx, ArgumentList::from(call->anchor()));
    }
    Call *newcall = Call::from(call->anchor(), callee, values);
    const FunctionType *ft = nullptr;
    if (is_function_pointer(T)) {
        ft = extract_function_type(T);
    } else {
        set_active_anchor(call->anchor());
        SCOPES_CHECK_RESULT(error_invalid_call_type(callee));
    }
    newcall->set_type(ft->return_type);
    return newcall;
}

static SCOPES_RESULT(Value *) specialize_SymbolValue(const ASTContext &ctx, SymbolValue *sym) {
    SCOPES_RESULT_TYPE(Value *);
    assert(ctx.frame);
    auto value = ctx.frame->resolve(sym);
    if (!value) {
        SCOPES_CHECK_RESULT(error_unbound_symbol(sym));
    }
    return value;
}

static SCOPES_RESULT(Value *) specialize_If(const ASTContext &ctx, If *_if) {
    SCOPES_RESULT_TYPE(Value *);
    assert(!_if->clauses.empty());
    auto subctx = ctx.with_target(EvalTarget_Symbol);
    const Type *rtype = nullptr;
    Clauses clauses;
    Clause else_clause;
    for (auto &&clause : _if->clauses) {
        auto newcond = SCOPES_GET_RESULT(specialize(subctx, clause.cond));
        if (newcond->get_type() != TYPE_Bool) {
            set_active_anchor(clause.anchor);
            SCOPES_EXPECT_ERROR(error_invalid_condition_type(newcond));
        }
        auto maybe_const = dyn_cast<ConstInt>(newcond);
        if (maybe_const) {
            bool istrue = maybe_const->value;
            if (istrue) {
                // always true - the remainder will not be evaluated
                else_clause = Clause(clause.anchor, clause.value);
                goto finalize;
            } else {
                // always false - this block will never be evaluated
                continue;
            }
        }
        clauses.push_back(Clause(clause.anchor, newcond, clause.value));
    }
    else_clause = Clause(_if->else_clause.anchor, _if->else_clause.value);
finalize:
    // run a suspendable job for each branch
    int numclauses = clauses.size() + 1;
    Value *values[numclauses];
    for (int i = 0; i < numclauses; ++i) {
        if ((i + 1) == numclauses) {
            values[i] = else_clause.value;
        } else {
            values[i] = clauses[i].value;
        }
    }
    SCOPES_CHECK_RESULT(specialize_jobs(ctx, numclauses, values));
    for (int i = 0; i < numclauses; ++i) {
        set_active_anchor(values[i]->anchor());
        rtype = SCOPES_GET_RESULT(merge_value_type(ctx, rtype, values[i]->get_type()));
        if ((i + 1) == numclauses) {
            else_clause.value = values[i];
        } else {
            clauses[i].value = values[i];
        }
    }
    if (clauses.empty()) {
        // else is always selected
        return else_clause.value;
    }
    If *newif = If::from(_if->anchor(), clauses);
    newif->else_clause = else_clause;
    rtype = ctx.transform_return_type(rtype);
    newif->set_type(rtype);
    return newif;
}

// this must never happen
static SCOPES_RESULT(Value *) specialize_Template(const ASTContext &ctx, Template *_template) {
    SCOPES_RESULT_TYPE(Value *);
    assert(_template->scope);
    Function *frame = ctx.frame->find_frame(_template->scope);
    if (!frame) {
        set_active_anchor(_template->anchor());
        SCOPES_EXPECT_ERROR(error_cannot_find_frame(_template));
    }
    return ConstPointer::closure_from(_template->anchor(), Closure::from(_template, frame));
}

static SCOPES_RESULT(Function *) specialize_Function(const ASTContext &ctx, Function *fn) {
    SCOPES_RESULT_TYPE(Function *);
    return fn;
}

SCOPES_RESULT(Value *) specialize(const ASTContext &ctx, Value *node) {
    SCOPES_RESULT_TYPE(Value *);
    assert(node);
    Value *result = nullptr; //ctx.frame->resolve(node);
    if (!result) {
        set_active_anchor(node->anchor());
        //SCOPES_CHECK_RESULT(verify_stack());
        switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
        case NAME: result = SCOPES_GET_RESULT(specialize_ ## CLASS(ctx, cast<CLASS>(node))); break;
        SCOPES_VALUE_KIND()
#undef T
        default: assert(false);
        }
        if (ctx.target == EvalTarget_Return) {
            if (is_returning(result->get_type())) {
                result = SCOPES_GET_RESULT(make_return(ctx, result->anchor(), result));
            }
        }
    }
    assert(result);
    #if 0
    if (node != result)
        ctx.frame->bind(node, result);
    #endif
    return result;
}

SCOPES_RESULT(Value *) specialize_inline(const ASTContext &ctx,
    Function *frame, Template *func, const Values &nodes) {
    SCOPES_RESULT_TYPE(Value *);
    Timer sum_specialize_time(TIMER_Specialize);
    assert(func);
    int count = (int)func->params.size();
    Function *fn = Function::from(func->anchor(), func->name, {}, func->value);
    fn->original = func;
    fn->frame = frame;
    auto let = Let::from(fn->anchor());
    let->params = func->params;
    let->args = nodes;
    auto block = Block::from(func->anchor(), {let}, fn->value);

    ASTContext subctx(fn, ctx.target);
    return SCOPES_GET_RESULT(specialize(subctx, block));
}

SCOPES_RESULT(Function *) specialize(Function *frame, Template *func, const ArgTypes &types) {
    SCOPES_RESULT_TYPE(Function *);
    Timer sum_specialize_time(TIMER_Specialize);
    assert(func);
    Function key(func->anchor(), func->name, {}, nullptr);
    key.original = func;
    key.frame = frame;
    key.instance_args = types;
    auto it = functions.find(&key);
    if (it != functions.end())
        return *it;
    int count = (int)func->params.size();
    Function *fn = Function::from(func->anchor(), func->name, {}, func->value);
    fn->original = func;
    fn->frame = frame;
    fn->instance_args = types;
    for (int i = 0; i < count; ++i) {
        auto oldparam = func->params[i];
        if (oldparam->is_variadic()) {
            if ((i + 1) < count) {
                set_active_anchor(oldparam->anchor());
                SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
            }
            if ((i + 1) == (int)types.size()) {
                auto newparam = SymbolValue::from(oldparam->anchor(), oldparam->name, types[i]);
                fn->append_param(newparam);
                fn->bind(oldparam, newparam);
            } else {
                ArgTypes vtypes;
                auto args = ArgumentList::from(oldparam->anchor());
                for (int j = i; j < types.size(); ++j) {
                    vtypes.push_back(types[j]);
                    auto newparam = SymbolValue::from(oldparam->anchor(), oldparam->name, types[j]);
                    fn->append_param(newparam);
                    args->values.push_back(newparam);
                }
                args->set_type(return_type(vtypes));
                fn->bind(oldparam, args);
            }
        } else {
            const Type *T = TYPE_Nothing;
            if (i < types.size()) {
                T = types[i];
            }
            if (oldparam->is_typed()) {
                set_active_anchor(oldparam->anchor());
                SCOPES_CHECK_RESULT(verify(oldparam->get_type(), T));
            }
            auto newparam = SymbolValue::from(oldparam->anchor(), oldparam->name, T);
            fn->append_param(newparam);
            fn->bind(oldparam, newparam);
        }
    }
    functions.insert(fn);

    ASTContext subctx(fn, EvalTarget_Return);
    fn->value = SCOPES_GET_RESULT(specialize(subctx, fn->value));
    assert(!is_returning(fn->value->get_type()));
    fn->complete = true;
    fn->set_type(get_function_type(fn));
    return fn;
}

} // namespace scopes

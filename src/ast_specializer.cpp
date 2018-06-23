/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "ast_specializer.hpp"
#include "ast.hpp"
#include "types.hpp"
#include "error.hpp"
#include "closure.hpp"
#include "stream_ast.hpp"
#include "hash.hpp"
#include "verify_tools.inc"
#include "dyn_cast.inc"

#include <unordered_set>

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

namespace ASTFunctionSet {
    struct Hash {
        std::size_t operator()(const ASTFunction *s) const {
            std::size_t h = std::hash<ASTFunction *>{}(s->frame);
            h = hash2(h, std::hash<Template *>{}(s->original));
            for (auto arg : s->instance_args) {
                h = hash2(h, std::hash<const Type *>{}(arg));
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ASTFunction *lhs, const ASTFunction *rhs ) const {
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
} // namespace ASTFunctionSet

static std::unordered_set<ASTFunction *, ASTFunctionSet::Hash, ASTFunctionSet::KeyEqual> astfunctions;

//------------------------------------------------------------------------------

struct ASTContext {
    ASTFunction *frame;
    Loop *loop;
    bool ignored;

    ASTContext to_used() const {
        ASTContext ctx(*this);
        ctx.ignored = false;
        return ctx;
    }

    ASTContext to_ignored() const {
        ASTContext ctx(*this);
        ctx.ignored = true;
        return ctx;
    }

    ASTContext for_loop(Loop *loop) const {
        ASTContext ctx(*this);
        ctx.loop = loop;
        return ctx;
    }

    ASTContext(ASTFunction *_frame, Loop *_loop = nullptr) :
        frame(_frame), loop(_loop), ignored(false) {
    }
};

// returns
static SCOPES_RESULT(ASTNode *) specialize(const ASTContext &ctx, ASTNode *node);

static SCOPES_RESULT(const Type *) merge_value_type(const ASTContext &ctx, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(const Type *);
    assert(T2);
    if (ctx.ignored && is_returning(T2))
        T2 = TYPE_Void;
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

static SCOPES_RESULT(ASTNode *) specialize_Block(const ASTContext &ctx, Block *block) {
    SCOPES_RESULT_TYPE(ASTNode *);
    Block *newblock = Block::from(block->anchor());
    auto subctx = ctx.to_ignored();
    for (auto &&src : block->body) {
        auto newsrc = SCOPES_GET_RESULT(specialize(subctx, src));
        if (!is_returning(newsrc->get_type())) {
            set_active_anchor(newsrc->anchor());
            SCOPES_CHECK_RESULT(error_noreturn_not_last_expression());
        }
        newblock->append(newsrc);
    }
    newblock->value = SCOPES_GET_RESULT(specialize(ctx, block->value));
    auto rtype = newblock->value->get_type();
    if (is_returning(rtype) && ctx.ignored)
        rtype = TYPE_Void;
    newblock->set_type(rtype);
    return newblock->canonicalize();
}

static SCOPES_RESULT(ASTArgumentList *) specialize_ASTArgumentList(
    const ASTContext &ctx, ASTArgumentList *nlist) {
    SCOPES_RESULT_TYPE(ASTArgumentList *);
    ArgTypes types;
    ASTArgumentList *newnlist = ASTArgumentList::from(nlist->anchor());
    for (auto &&value : nlist->values) {
        auto newvalue = SCOPES_GET_RESULT(specialize(ctx, value));
        types.push_back(newvalue->get_type());
        newnlist->append(newvalue);
    }
    newnlist->set_type(Return(types));
    return newnlist;
}

static SCOPES_RESULT(ASTNode *) specialize_Let(const ASTContext &ctx, Let *let) {
    SCOPES_RESULT_TYPE(ASTNode *);
    Let *newlet = Let::from(let->anchor());
    auto subctx = ctx.to_used();
    for (auto &&binding : let->bindings) {
        auto oldsym = binding.sym;
        auto expr = SCOPES_GET_RESULT(specialize(subctx, binding.expr));
        auto newsym = ASTSymbol::from(oldsym->anchor(), oldsym->name, expr->get_type());
        ctx.frame->bind(oldsym, newsym);
        newlet->append(newsym, expr);
    }
    newlet->value = SCOPES_GET_RESULT(specialize(ctx, let->value));
    auto rtype = newlet->value->get_type();
    if (is_returning(rtype) && ctx.ignored)
        rtype = TYPE_Void;
    newlet->set_type(rtype);
    return newlet;
}

static SCOPES_RESULT(Loop *) specialize_Loop(const ASTContext &ctx, Loop *loop) {
    SCOPES_RESULT_TYPE(Loop *);
    Loop *newloop = Loop::from(loop->anchor());
    auto subctx = ctx.to_used();
    for (auto &&binding : loop->bindings) {
        auto oldsym = binding.sym;
        auto expr = SCOPES_GET_RESULT(specialize(subctx, binding.expr));
        auto newsym = ASTSymbol::from(oldsym->anchor(), oldsym->name, expr->get_type());
        ctx.frame->bind(oldsym, newsym);
        newloop->append(newsym, expr);
    }
    newloop->value = SCOPES_GET_RESULT(specialize(ctx.for_loop(newloop), loop->value));
    auto rtype = newloop->value->get_type();
    newloop->return_type = SCOPES_GET_RESULT(merge_value_type(ctx, newloop->return_type, rtype));
    newloop->set_type(newloop->return_type);
    return newloop;
}

static SCOPES_RESULT(Const *) specialize_Const(const ASTContext &ctx, Const *vconst) {
    SCOPES_RESULT_TYPE(Const *);
    return vconst;
}

static SCOPES_RESULT(Break *) specialize_Break(const ASTContext &ctx, Break *_break) {
    SCOPES_RESULT_TYPE(Break *);
    if (!ctx.loop) {
        set_active_anchor(_break->anchor());
        SCOPES_EXPECT_ERROR(error_illegal_break_outside_loop());
    }
    auto subctx = ctx.to_used();
    ASTNode *value = SCOPES_GET_RESULT(specialize(subctx, _break->value));
    ctx.loop->return_type = SCOPES_GET_RESULT(merge_value_type(subctx, ctx.loop->return_type, value->get_type()));
    auto newbreak = Break::from(_break->anchor(), value);
    newbreak->set_type(NoReturn());
    return newbreak;
}

static SCOPES_RESULT(Repeat *) specialize_Repeat(const ASTContext &ctx, Repeat *_repeat) {
    SCOPES_RESULT_TYPE(Repeat *);
    if (!ctx.loop) {
        set_active_anchor(_repeat->anchor());
        SCOPES_EXPECT_ERROR(error_illegal_repeat_outside_loop());
    }
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx.to_used(), _repeat->args));
    auto newrepeat = Repeat::from(_repeat->anchor(), args);
    newrepeat->set_type(NoReturn());
    return newrepeat;
}

static SCOPES_RESULT(ASTReturn *) specialize_ASTReturn(const ASTContext &ctx, ASTReturn *_return) {
    SCOPES_RESULT_TYPE(ASTReturn *);
    ASTNode *value = SCOPES_GET_RESULT(specialize(ctx.to_used(), _return->value));
    assert(ctx.frame);
    ctx.frame->return_type = SCOPES_GET_RESULT(merge_return_type(ctx.frame->return_type, value->get_type()));
    auto newreturn = ASTReturn::from(_return->anchor(), value);
    newreturn->set_type(NoReturn());
    return newreturn;
}

static SCOPES_RESULT(ASTNode *) specialize_SyntaxExtend(const ASTContext &ctx, SyntaxExtend *sx) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(false);
    return nullptr;
}

static SCOPES_RESULT(Keyed *) specialize_Keyed(const ASTContext &ctx, Keyed *keyed) {
    SCOPES_RESULT_TYPE(Keyed *);
    return Keyed::from(keyed->anchor(), keyed->key,
        SCOPES_GET_RESULT(specialize(ctx, keyed->value)));
}

SCOPES_RESULT(Any) extract_constant(ASTNode *value) {
    SCOPES_RESULT_TYPE(Any);
    auto constval = dyn_cast<Const>(value);
    if (!constval) {
        SCOPES_CHECK_RESULT(error_constant_expected(value));
    }
    return constval->value;
}

static SCOPES_RESULT(const Type *) bool_op_return_type(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    T = SCOPES_GET_RESULT(storage_type(T));
    if (T->kind() == TK_Vector) {
        auto vi = cast<VectorType>(T);
        return Vector(TYPE_Bool, vi->count);
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
        Call *newcall = Call::from(call->anchor(), callee, args); \
        newcall->set_type(Return({ __VA_ARGS__ })); \
        return newcall; \
    }
#define READ_TYPE(NAME) \
        assert(argn <= argcount); \
        const Type *NAME = values[argn++]->get_type();

static SCOPES_RESULT(ASTNode *) specialize_Call(const ASTContext &ctx, Call *call) {
    SCOPES_RESULT_TYPE(ASTNode *);
    auto subctx = ctx.to_used();
    ASTNode *callee = SCOPES_GET_RESULT(specialize(subctx, call->callee));
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(subctx, call->args));
    const Type *T = callee->get_type();
    if (T == TYPE_Closure) {
        auto anycl = SCOPES_GET_RESULT(extract_constant(callee));
        //SCOPES_CHECK_RESULT(anycl.verify(TYPE_Closure));
        const Closure *cl = anycl.closure;
        ArgTypes types;
        for (auto &&arg : args->values) {
            types.push_back(arg->get_type());
        }
        callee = SCOPES_GET_RESULT(specialize(cl->frame, cl->func, types));
        T = callee->get_type();
    } else if (T == TYPE_Builtin) {
        auto anycl = SCOPES_GET_RESULT(extract_constant(callee));
        //SCOPES_CHECK_RESULT(anycl.verify(TYPE_Builtin));
        Builtin b = anycl.builtin;
        auto &&values = args->values;
        size_t argcount = values.size();
        size_t argn = 0;
        set_active_anchor(call->anchor());
        switch(b.value()) {
        case FN_Dump: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump: ";
            if (args->values.size() == 1)
                stream_ast(ss, args->values[0], StreamASTFormat());
            else
                stream_ast(ss, args, StreamASTFormat());
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
            READ_TYPE(A); READ_TYPE(B);
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
            READ_TYPE(A); READ_TYPE(B);
            SCOPES_CHECK_RESULT(verify_real_ops(A, B));
            RETARGTYPES(SCOPES_GET_RESULT(bool_op_return_type(A)));
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
        case OP_ ## NAME: \
        case OP_ ## NAME ## NUW: \
        case OP_ ## NAME ## NSW: { \
            CHECKARGS(2, 2); \
            READ_TYPE(A); READ_TYPE(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define IARITH_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPE(A); READ_TYPE(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define FARITH_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPE(A); READ_TYPE(B); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B)); \
            RETARGTYPES(A); \
        } break;
#define FTRI_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(3, 3); \
            READ_TYPE(A); READ_TYPE(B); READ_TYPE(C); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B, C)); \
            RETARGTYPES(A); \
        } break;
#define IUN_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPE(A); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A)); \
            RETARGTYPES(A); \
        } break;
#define FUN_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPE(A); \
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

        return specialize(ctx, ASTArgumentList::from(call->anchor()));
    }
    Call *newcall = Call::from(call->anchor(), callee, args);
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

static SCOPES_RESULT(ASTNode *) specialize_ASTSymbol(const ASTContext &ctx, ASTSymbol *sym) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(ctx.frame);
    auto value = ctx.frame->resolve(sym);
    if (!value) {
        SCOPES_CHECK_RESULT(error_unbound_symbol(sym));
    }
    return value;
}

static SCOPES_RESULT(If *) specialize_If(const ASTContext &ctx, If *_if) {
    SCOPES_RESULT_TYPE(If *);
    If *newif = If::from(_if->anchor());
    assert(!_if->clauses.empty());
    auto subctx = ctx.to_used();
    const Type *rtype = nullptr;
    for (auto &&clause : _if->clauses) {
        auto newcond = SCOPES_GET_RESULT(specialize(subctx, clause.cond));
        if (newcond->get_type() != TYPE_Bool) {
            set_active_anchor(clause.anchor);
            SCOPES_EXPECT_ERROR(error_invalid_condition_type(newcond));
        }
        auto value = SCOPES_GET_RESULT(specialize(ctx, clause.value));
        set_active_anchor(value->anchor());
        rtype = SCOPES_GET_RESULT(merge_value_type(ctx, rtype, value->get_type()));
        newif->append(clause.anchor, newcond, value);
    }
    {
        auto value = SCOPES_GET_RESULT(specialize(ctx, _if->else_clause.value));
        set_active_anchor(value->anchor());
        rtype = SCOPES_GET_RESULT(merge_value_type(ctx, rtype, value->get_type()));
        newif->append(_if->else_clause.anchor, value);
    }
    if (is_returning(rtype) && ctx.ignored) {
        rtype = TYPE_Void;
    }
    newif->set_type(rtype);
    return newif;
}

// this must never happen
static SCOPES_RESULT(ASTNode *) specialize_Template(const ASTContext &ctx, Template *_template) {
    SCOPES_RESULT_TYPE(ASTNode *);
    ASTFunction *frame = ctx.frame->find_frame(_template->scope);
    if (!frame) {
        SCOPES_LOCATION_ERROR(String::from("couldn't find frame"));
    }
    return Const::from(_template->anchor(), Closure::from(_template, frame));
}

static SCOPES_RESULT(ASTFunction *) specialize_ASTFunction(const ASTContext &ctx, ASTFunction *fn) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    assert(!ctx.ignored);
    fn->value = SCOPES_GET_RESULT(specialize(ctx, fn->value));
    fn->return_type = SCOPES_GET_RESULT(merge_return_type(fn->return_type, fn->value->get_type()));
    ArgTypes params;
    for (int i = 0; i < fn->params.size(); ++i) {
        params.push_back(fn->params[i]->get_type());
    }
    fn->set_type(NativeROPointer(Function(fn->return_type, params)));
    return fn;
}

SCOPES_RESULT(ASTNode *) specialize(const ASTContext &ctx, ASTNode *node) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(node);
    set_active_anchor(node->anchor());
    switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME: return SCOPES_GET_RESULT(specialize_ ## CLASS(ctx, cast<CLASS>(node))); break;
    SCOPES_AST_KIND()
#undef T
    default: assert(false);
    }
    return node;
}

SCOPES_RESULT(ASTFunction *) specialize(ASTFunction *frame, Template *func, const ArgTypes &types) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    assert(func);
    ASTFunction key(func->anchor(), func->name, {}, nullptr);
    key.original = func;
    key.frame = frame;
    key.instance_args = types;
    auto it = astfunctions.find(&key);
    if (it != astfunctions.end())
        return *it;
    int count = (int)func->params.size();
    assert(count == types.size());
    ASTFunction *fn = ASTFunction::from(func->anchor(), func->name, {}, func->value);
    fn->original = func;
    fn->frame = frame;
    fn->instance_args = types;
    for (int i = 0; i < count; ++i) {
        auto oldparam = func->params[i];
        assert(!oldparam->is_variadic());
        const Type *T = types[i];
        if (oldparam->is_typed()) {
            assert(oldparam->get_type() == T);
        }
        auto newparam = ASTSymbol::from(oldparam->anchor(), oldparam->name, T);
        fn->bind(oldparam, newparam);
        fn->append_param(newparam);
    }
    astfunctions.insert(fn);
    return specialize_ASTFunction(ASTContext(fn), fn);
}

#if 0
static SCOPES_RESULT(ASTFunction *) type_function(const ASTContext &ctx, Template *fn, ASTArgumentList *args) {
    ASTSymbols params;
    assert(fn->body);
    ASTFunction *newfn = ASTFunction::from(fn->anchor(), fn->name, params, fn->body);


    return specialize_ASTFunction(ctx, newfn);
}
#endif

} // namespace scopes

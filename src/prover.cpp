/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "prover.hpp"
#include "value.hpp"
#include "types.hpp"
#include "qualifiers.hpp"
#include "error.hpp"
#include "stream_expr.hpp"
#include "hash.hpp"
#include "timer.hpp"
#include "gc.hpp"
#include "builtin.hpp"
#include "verify_tools.inc"
#include "dyn_cast.inc"
#include "compiler_flags.hpp"
#include "gen_llvm.hpp"
#include "list.hpp"
#include "expander.hpp"
#include "globals.hpp"
#include "quote.hpp"
#include "anchor.hpp"
#include "scopes/scopes.h"
#include "qualifier.inc"

#include <algorithm>
#include <unordered_set>
#include <deque>

#pragma GCC diagnostic ignored "-Wvla-extension"
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

#define SCOPES_DEBUG_SYNTAX_EXTEND 0
#define SCOPES_ANNOTATE_TRACKING 1

// the old specializer is at
// https://bitbucket.org/duangle/scopes/raw/dfb69b02546e859b702176c58e92a63de3461d77/src/specializer.cpp

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
            std::size_t h = std::hash<Function *>{}(s->frame.unref());
            h = hash2(h, std::hash<Template *>{}(s->original.unref()));
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

static SCOPES_RESULT(const ReferQualifier *) verify_refer(const Type *T) {
    SCOPES_RESULT_TYPE(const ReferQualifier *);
    auto rq = try_qualifier<ReferQualifier>(T);
    if (!rq) {
        SCOPES_ERROR(ValueMustBeReference, T);
    }
    return rq;
}

static SCOPES_RESULT(void) verify_readable(const ReferQualifier *Q, const Type *T) {
    SCOPES_RESULT_TYPE(void);
    if (!pointer_flags_is_readable(Q->flags)) {
        SCOPES_ERROR(NonReadableReference, T);
    }
    return {};
}

static SCOPES_RESULT(void) verify_writable(const ReferQualifier *Q, const Type *T) {
    SCOPES_RESULT_TYPE(void);
    if (!pointer_flags_is_writable(Q->flags)) {
        SCOPES_ERROR(NonWritableReference, T);
    }
    return {};
}

static SCOPES_RESULT(void) verify_readable(const Type *T) {
    SCOPES_RESULT_TYPE(void);
    auto pi = cast<PointerType>(T);
    if (!pi->is_readable()) {
        SCOPES_ERROR(NonReadablePointer, T);
    }
    return {};
}

static SCOPES_RESULT(void) verify_writable(const Type *T) {
    SCOPES_RESULT_TYPE(void);
    auto pi = cast<PointerType>(T);
    if (!pi->is_writable()) {
        SCOPES_ERROR(NonWritablePointer, T);
    }
    return {};
}

#if 0
static SCOPES_RESULT(Const *) nullof(const Anchor *anchor, const Type *T) {
    SCOPES_RESULT_TYPE(Const *);
    SCOPES_ANCHOR(anchor);
    const Type *ST = SCOPES_GET_RESULT(storage_type(T));
    switch(ST->kind()) {
    case TK_Integer: return ConstInt::from(anchor, T, 0);
    case TK_Real: return ConstReal::from(anchor, T, 0.0);
    case TK_Pointer: return ConstPointer::from(anchor, T, nullptr);
    case TK_Array: {
        auto at = cast<ArrayType>(ST);
        Constants fields;
        if (at->count) {
            auto elem = SCOPES_GET_RESULT(nullof(anchor, at->element_type));
            for (size_t i = 0; i < at->count; ++i) {
                fields.push_back(elem);
            }
        }
        return ConstAggregate::from(anchor, T, fields);
    } break;
    case TK_Vector: {
        auto at = cast<VectorType>(ST);
        Constants fields;
        if (at->count) {
            auto elem = SCOPES_GET_RESULT(nullof(anchor, at->element_type));
            for (size_t i = 0; i < at->count; ++i) {
                fields.push_back(elem);
            }
        }
        return ConstAggregate::from(anchor, T, fields);
    } break;
    case TK_Tuple: {
        auto at = cast<TupleType>(ST);
        Constants fields;
        for (auto valT : at->values) {
            fields.push_back(SCOPES_GET_RESULT(nullof(anchor, valT)));
        }
        return ConstAggregate::from(anchor, T, fields);
    } break;
    case TK_Tuple: {
        auto at = cast<TupleType>(ST);
        Constants fields;
        for (auto valT : at->values) {
            fields.push_back(SCOPES_GET_RESULT(nullof(anchor, valT)));
        }
        return ConstAggregate::from(anchor, T, fields);
    } break;
    default: {
        SCOPES_LOCATION_ERROR(String::from(
            "can't create constant of type"));
    } break;
    }
    return nullptr;
}
#endif

//------------------------------------------------------------------------------

static const ASTContext *ast_context = nullptr;

struct ScopedASTContext {
    ScopedASTContext(const ASTContext &ctx) {
        old_ctx = ast_context;
        ast_context = &ctx;
    }

    ~ScopedASTContext() {
        ast_context = old_ctx;
    }

    const ASTContext *old_ctx;
};

#define SCOPES_ASTCONTEXT(CTX) ScopedASTContext SCOPES_CAT(_scoped_ast_context, __LINE__)(CTX)

//------------------------------------------------------------------------------

ASTContext ASTContext::for_loop(const LoopLabelRef &loop) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext ASTContext::for_break(const LabelRef &xbreak) const {
    return ASTContext(function, frame, loop, except, xbreak, block);
}

ASTContext ASTContext::for_try(const LabelRef &except) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext ASTContext::with_block(Block &_block) const {
    return ASTContext(function, frame, loop, except, _break, &_block);
}

ASTContext ASTContext::with_frame(const FunctionRef &frame) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext::ASTContext() {}

ASTContext::ASTContext(const FunctionRef &_function, const FunctionRef &_frame,
    const LoopLabelRef &_loop, const LabelRef &_except,
    const LabelRef &xbreak, Block *_block) :
    function(_function), frame(_frame), loop(_loop),
    except(_except), _break(xbreak), block(_block) {
}

const Type *ASTContext::fix_merge_type(const Type *T) const {
    if (!is_returning_value(T))
        return T;
    int count = get_argument_count(T);
    Types newtypes;
    newtypes.reserve(count);
    for (int i = 0; i < count; ++i) {
        auto argT = get_argument(T, i);
        if (!is_view(argT) && !is_plain(argT)) {
            argT = unique_type(argT, unique_id());
        }
        newtypes.push_back(argT);
    }
    return arguments_type(newtypes);
}

int ASTContext::unique_id() const {
    return function->unique_id();
}

void ASTContext::move(int id, const ValueRef &mover) const {
    block->move(id);
    function->hint_mover(id, mover);
}

void ASTContext::merge_block(Block &_block) const {
    block->migrate_from(_block);
}

void ASTContext::append(const TypedValueRef &value) const {
    assert(block);
    if (block->append(value)) {
        function->try_bind_unique(value);
    }
}

ASTContext ASTContext::from_function(const FunctionRef &fn) {
    return ASTContext(fn, fn, LoopLabelRef(), LabelRef(), LabelRef(), nullptr);
}

//------------------------------------------------------------------------------

static SCOPES_RESULT(TypedValueRef) prove_inline(const ASTContext &ctx,
    const Closure *cl, const TypedValues &nodes);

static const Type *merge_single_value_type(const char *context, const Type *T1, const Type *T2) {
    assert(T1);
    assert(T2);
    if (T1 == T2)
        return T1;
    auto vq = try_view(T2);
    // are both types views of the same type?
    if (vq && is_view(T1)
        && (strip_view(T1) == strip_view(T2))) {
        // merge all ids
        return view_type(T1, vq->ids);
    }
    if (is_unique(T1) && is_unique(T2)
        && (strip_unique(T1) == strip_unique(T2))) {
        // return stand-in unique tag
        return unique_type(T1, UnknownUnique);
    }
    return nullptr;
}

static SCOPES_RESULT(const Type *) merge_value_type(const char *context, const Type *T1, const Type *T2) {
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
    auto count = get_argument_count(T1);
    Types newargs;
    if (get_argument_count(T2) == count) {
        for (int i = 0; i < count; ++i) {
            auto argT1 = get_argument(T1, i);
            auto argT2 = get_argument(T2, i);
            const Type *T = merge_single_value_type(context, argT1, argT2);
            if (!T) {
                SCOPES_ERROR(MergeConflict, context, T1, T2);
            }
            newargs.push_back(T);
        }
        return arguments_type(newargs);
    }
    SCOPES_ERROR(MergeConflict, context, T1, T2);
}

SCOPES_RESULT(TypedValueRef) prove_block(const ASTContext &ctx, Block &block,
    const ValueRef &node, ASTContext &newctx) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    block.set_parent(ctx.block);
    newctx = ctx.with_block(block);
    auto result = SCOPES_GET_RESULT(prove(newctx, node));
    return result;
}

static bool split_return_values(TypedValues &values, const TypedValueRef &value) {
    auto T = value->get_type();
    if (!is_returning(T)) return false;
    auto count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        values.push_back(ExtractArgument::from(value, i));
    }
    return true;
}

void map_arguments_to_block(const ASTContext &ctx, const TypedValueRef &src) {
    const Type *T = src->get_type();
    int count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        auto argT = get_argument(T, i);
        auto uq = try_unique(argT);
        if (uq) {
            ctx.function->bind_unique(Function::UniqueInfo(ValueIndex(src, i)));
            ctx.block->valid.insert(uq->id);
        }
    }
}

static void write_annotation(const ASTContext &ctx,
    const Anchor *anchor, const String *msg, Values values) {
    values.insert(values.begin(),
        ref(anchor, ConstPointer::string_from(msg)));
    auto expr = ref(anchor,
            CallTemplate::from(
                ref(anchor, ConstInt::builtin_from(Builtin(FN_Annotate))),
                values));
    auto result = prove(ctx, expr);
    if (!result.ok()) {
        print_error(result.assert_error());
        assert(false && "error while annotating");
    }
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, int id, const char *by) {
    SCOPES_RESULT_TYPE(void);
    if (!ctx.block->is_valid(id)) {
        auto info = ctx.function->get_unique_info(id);
        SCOPES_ERROR(InaccessibleValue, info.value.get_type(),
            ctx.function->get_best_mover_anchor(id));
    }
    return {};
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, const IDSet &ids, const char *by) {
    SCOPES_RESULT_TYPE(void);
    for (auto id : ids) {
        SCOPES_CHECK_RESULT(verify_valid(ctx, id, by));
    }
    return {};
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, const TypedValueRef &val, const char *by) {
    SCOPES_RESULT_TYPE(void);
    auto T = val->get_type();
    if (!is_returning_value(T))
        return {};
    int id = 0;
    if (!ctx.block->is_valid(ValueIndex(val), id)) {
        SCOPES_ERROR(InaccessibleValue, T,
            ctx.function->get_best_mover_anchor(id));
    }
    return {};
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, const TypedValues &values, const char *by) {
    SCOPES_RESULT_TYPE(void);
    for (auto &&value : values) {
        SCOPES_TRACE_PROVE_ARG_LIFETIME(value);
        SCOPES_CHECK_RESULT(verify_valid(ctx, value, by));
    }
    return {};
}

static SCOPES_RESULT(TypedValueRef) build_drop(const ASTContext &ctx,
    const Anchor *anchor, const ValueIndex &arg) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    // generate destructor
    auto argT = arg.get_type();
    IDSet ids;
    auto uq = try_unique(argT);
    if (uq) {
        ids.insert(uq->id);
    } else {
        auto vq = try_view(argT);
        if (vq) {
            ids = vq->ids;
        }
    }
    argT = strip_qualifiers(argT);
    ValueRef handler;
    if (!argT->lookup(SYM_DropHandler, handler)) {
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "forget";
        write_annotation(ctx, anchor, ss.str(), {
            ref(anchor, ConstPointer::type_from(arg.get_type())) });
        #endif
        return ref(anchor, ArgumentList::from({}));
    } else {
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "destruct";
        write_annotation(ctx, anchor, ss.str(), {
            ref(anchor, ConstPointer::type_from(arg.get_type())) });
        #endif
        auto expr =
            ref(anchor, CallTemplate::from(handler, {
                ref(anchor, ExtractArgument::from(arg.value, arg.index)) }));
        auto result = SCOPES_GET_RESULT(prove(ctx, expr));
        auto RT = result->get_type();
        if (!is_returning(RT) || is_returning_value(RT)) {
            SCOPES_ERROR(DropReturnsArguments);
        }
        SCOPES_CHECK_RESULT(verify_valid(ctx, ids, "drop"));
        return result;
    }
}

static TypedValueRef build_free(
    const ASTContext &ctx, const Anchor *anchor, const TypedValueRef &val) {
    auto call = ref(anchor, Call::from(empty_arguments_type(), g_free, { val }));
    ctx.append(call);
    return call;
}

static bool needs_autofree(const Type *T) {
    auto rq = try_qualifier<ReferQualifier>(T);
    if (!rq) return false;
    // must be default (heap) class
    if (rq->storage_class != SYM_Unnamed) return false;
    // must be writable
    if (!pointer_flags_is_writable(rq->flags)) return false;
    return true;
}

static SCOPES_RESULT(void) drop_value(const ASTContext &ctx,
    const Anchor *anchor, const ValueIndex &arg) {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(build_drop(ctx, anchor, arg));
    auto argT = arg.get_type();
    int id = get_unique(argT)->id;
    #if 0
    if (needs_autofree(argT)) {
        build_free(ctx, anchor, ref(anchor, ExtractArgument::from(arg.value, arg.index)));
    }
    #endif
    ctx.move(id, ref(anchor, arg.value));
    return {};
}

static void build_view(
    const ASTContext &ctx, const Anchor *anchor, TypedValueRef &val) {
    auto T = val->get_type();
    auto uq = try_unique(T);
    if (uq) {
        assert(ctx.block->is_valid(ValueIndex(val)));
        auto retT = view_type(T, {});
        auto call = ref(anchor, Call::from(retT, g_view, { val }));
        ctx.append(call);
        val = call;
    }
}

static void build_move(
    const ASTContext &ctx, const Anchor *anchor, TypedValueRef &val) {
    assert(ctx.block->is_valid(ValueIndex(val)));
    auto T = val->get_type();
    auto uq = get_unique(T);
    auto retT = unique_type(T, ctx.unique_id());
    auto call = ref(anchor, Call::from(retT, g_move, { val }));
    ctx.append(call);
    ctx.move(uq->id, val);
    val = call;
}

static SCOPES_RESULT(void) validate_pass_block(const ASTContext &ctx, const Block &src) {
    SCOPES_RESULT_TYPE(void);
    // see if pass deleted any values
    IDSet deleted = difference_idset(ctx.block->valid, src.valid);
    if (!deleted.empty()) {
        int id = *deleted.begin();
        auto info = ctx.function->get_unique_info(id);
        assert (info.get_depth() < src.depth);
        SCOPES_ERROR(SwitchPassMovedValue, info.value.get_type());
    }
    return {};
}

static void merge_back_valid(const ASTContext &ctx, IDSet &valid, const ValueRef &mover) {
    IDSet deleted = difference_idset(ctx.block->valid, valid);
    for (auto id : deleted) {
        ctx.move(id, mover);
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "merge-forgetting " << id;
        write_annotation(ctx, unknown_anchor(), ss.str(), {});
        #endif
    }
}

// from a parent set of valid values, only keep the ones in both sets
static void collect_valid_values(const ASTContext &ctx, IDSet &valid) {
    valid = intersect_idset(valid, ctx.block->valid);
}

// from a parent set of valid values, only keep the ones in both sets
static void collect_valid_function_values(const ASTContext &ctx) {
    ctx.function->valid = intersect_idset(ctx.function->valid, ctx.block->valid);
}

// check if all merge view arguments are still valid
static SCOPES_RESULT(void) validate_merge_values(const IDSet &valid, const TypedValues &values) {
    SCOPES_RESULT_TYPE(void);
    for (auto &&value : values) {
        auto vq = try_view(value->get_type());
        if (!vq) continue;
        for (auto id : vq->ids) {
            if (!valid.count(id)) {
                SCOPES_ERROR(ViewExitingScope, value->get_type());
            }
        }
    }
    return {};
}

static void sort_drop_ids(const IDSet &invalid, IDs &drop_ids) {
    drop_ids.reserve(invalid.size());
    for (auto id : invalid) {
        drop_ids.push_back(id);
    }
    // drop newest IDs first
    std::sort(drop_ids.rbegin(), drop_ids.rend());
}

static SCOPES_RESULT(void) merge_drop_values(const ASTContext &ctx,
    const Anchor *anchor, const IDs &drop_ids) {
    SCOPES_RESULT_TYPE(void);
    for (auto &&id : drop_ids) {
        if (!ctx.block->is_valid(id)) // already moved
            continue;
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "merge-dropping " << id;
        write_annotation(ctx, anchor, ss.str(), {});
        #endif
        auto info = ctx.function->get_unique_info(id);
        SCOPES_CHECK_RESULT(drop_value(ctx, anchor, info.value));
    }
    return {};
}

static SCOPES_RESULT(void) move_merge_values(const ASTContext &ctx,
    const Anchor *anchor, int retdepth, TypedValues &values, const char *by) {
    SCOPES_RESULT_TYPE(void);
    assert(retdepth >= 0);
    IDSet saved;
    for (auto &&value : values) {
        SCOPES_CHECK_RESULT(verify_valid(ctx, value, by));
        auto T = value->get_type();
        auto uq = try_unique(T);
        if (uq) {
            auto info = ctx.function->get_unique_info(uq->id);
            int depth = info.get_depth();
            if (depth <= retdepth) {
                // must move
                build_move(ctx, anchor, value);
                T = value->get_type();
                uq = get_unique(T);
            }
            // must save
            saved.insert(uq->id);
            continue;
        }
        auto vq = try_view(T);
        if (vq) {
            for (auto &&id : vq->ids) {
                auto info = ctx.function->get_unique_info(id);
                int depth = info.get_depth();
                if (depth > retdepth) {
                    // cannot move id of value that is going to be deleted
                    SCOPES_ERROR(ViewExitingScope, value->get_type());
                } else {
                    // will still be live
                }
            }
            continue;
        }
    }

    // auto-drop all locally valid uniques
    Block *block = ctx.block;
    assert(block);
    IDSet valid = block->valid;
    for (auto id : valid) {
        if (saved.count(id))
            continue;
        auto info = ctx.function->get_unique_info(id);
        if (info.get_depth() <= retdepth)
            continue;
        SCOPES_CHECK_RESULT(drop_value(ctx, anchor, info.value));
    }

    return {};
}

static SCOPES_RESULT(TypedValueRef) move_single_merge_value(const ASTContext &ctx,
    int retdepth, TypedValueRef result, const char *by) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    TypedValues values;
    if (is_returning_value(result->get_type())
        && split_return_values(values, result)) {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, result.anchor(),
            retdepth, values, by));
        result = ref(result.anchor(), ArgumentList::from(values));
    } else {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, result.anchor(),
            retdepth, values, by));
    }
    return result;
}

// must be called before the return type is computed
// don't forget to call merge_back_invalid(...) when the label has been added
SCOPES_RESULT(void) finalize_merges(const ASTContext &ctx, const LabelRef &label, IDSet &valid, const char *by) {
    SCOPES_RESULT_TYPE(void);
    valid = ctx.block->valid;
    // patch merges
    int retdepth = label->body.depth - 1;
    for (auto merge : label->merges) {
        auto mctx = ctx.with_block(*merge->block);
        assert(merge->block);
        SCOPES_CHECK_RESULT(move_merge_values(mctx,
            merge.anchor(), retdepth, merge->values, by));
        collect_valid_values(mctx, valid);
    }
    // deleted values
    IDSet deleted = difference_idset(ctx.block->valid, valid);
    for (auto merge : label->merges) {
        auto mctx = ctx.with_block(*merge->block);
        SCOPES_CHECK_RESULT(validate_merge_values(valid, merge->values));
        // values to drop: deleted values which are still alive in merge block
        IDSet todrop = intersect_idset(deleted, merge->block->valid);
        IDs drop_ids;
        drop_ids.reserve(todrop.size());
        sort_drop_ids(todrop, drop_ids);
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge.anchor(), drop_ids));
    }
    return {};
}

SCOPES_RESULT(void) finalize_repeats(const ASTContext &ctx, const LoopLabelRef &label, const char *by) {
    SCOPES_RESULT_TYPE(void);
    IDSet valid = ctx.block->valid;
    // patch repeats
    int retdepth = label->body.depth - 1;
    for (auto merge : label->repeats) {
        auto mctx = ctx.with_block(*merge->block);
        assert(merge->block);
        SCOPES_CHECK_RESULT(move_merge_values(mctx,
            merge.anchor(), retdepth, merge->values, by));
        collect_valid_values(mctx, valid);
    }
    IDSet deleted = difference_idset(ctx.block->valid, valid);
    if (!deleted.empty()) {
        // parent values were deleted, which we can't repeat
        int id = *deleted.begin();
        auto info = ctx.function->get_unique_info(id);
        SCOPES_ERROR(LoopMovedValue, info.value.get_type());
    }
    for (auto merge : label->repeats) {
        SCOPES_CHECK_RESULT(validate_merge_values(valid, merge->values));
    }
    return {};
}

SCOPES_RESULT(void) finalize_returns_raises(const ASTContext &ctx) {
    SCOPES_RESULT_TYPE(void);
    const IDSet &valid = ctx.function->valid;
    IDSet deleted = difference_idset(
        ctx.function->original_valid, valid);
    for (auto merge : ctx.function->returns) {
        auto mctx = ctx.with_block(*merge->block);
        SCOPES_CHECK_RESULT(validate_merge_values(valid, merge->values));
        IDSet todrop = intersect_idset(deleted, merge->block->valid);
        IDs drop_ids;
        drop_ids.reserve(todrop.size());
        sort_drop_ids(todrop, drop_ids);
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge.anchor(), drop_ids));
    }
    for (auto merge : ctx.function->raises) {
        auto mctx = ctx.with_block(*merge->block);
        SCOPES_CHECK_RESULT(validate_merge_values(valid, merge->values));
        IDSet todrop = intersect_idset(deleted, merge->block->valid);
        IDs drop_ids;
        drop_ids.reserve(todrop.size());
        sort_drop_ids(todrop, drop_ids);
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge.anchor(), drop_ids));
    }
    return {};
}

static TypedValueRef make_merge1(const ASTContext &ctx, const Anchor *anchor, const LabelRef &label, const TypedValues &values) {
    assert(label);

    auto newmerge = ref(anchor, Merge::from(label, values));

    ctx.append(newmerge);
    label->merges.push_back(newmerge);

    return newmerge;
}

static TypedValueRef make_merge(const ASTContext &ctx, const Anchor *anchor, const LabelRef &label, const TypedValueRef &value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_merge1(ctx, anchor, label, results);
    } else {
        return value;
    }
}

static TypedValueRef make_repeat1(const ASTContext &ctx, const Anchor *anchor, const LoopLabelRef &label, const TypedValues &values) {
    assert(label);

    auto newrepeat = ref(anchor, Repeat::from(label, values));

    ctx.append(newrepeat);
    label->repeats.push_back(newrepeat);

    return newrepeat;
}

static TypedValueRef make_repeat(const ASTContext &ctx, const Anchor *anchor, const LoopLabelRef &label, const TypedValueRef &value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_repeat1(ctx, anchor, label, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValueRef) make_return1(const ASTContext &ctx, const Anchor *anchor, TypedValues values) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    assert(ctx.block);
    assert(ctx.function);
    SCOPES_CHECK_RESULT(move_merge_values(ctx, anchor, 0, values, "return"));
    collect_valid_function_values(ctx);

    auto newreturn = ref(anchor, Return::from(values));

    ctx.append(newreturn);
    ctx.function->returns.push_back(newreturn);
    return TypedValueRef(newreturn);
}

static SCOPES_RESULT(TypedValueRef) make_return(const ASTContext &ctx, const Anchor *anchor, const TypedValueRef &value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_return1(ctx, anchor, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValueRef) make_raise1(const ASTContext &ctx, const Anchor *anchor, TypedValues values) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    if (ctx.except) {
        return make_merge1(ctx, anchor, ctx.except, values);
    } else {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, anchor, 0, values, "raise"));
        collect_valid_function_values(ctx);

        auto newraise = ref(anchor, Raise::from(values));

        ctx.append(newraise);
        ctx.function->raises.push_back(newraise);
        return TypedValueRef(newraise);
    }
}

static SCOPES_RESULT(TypedValueRef) make_raise(const ASTContext &ctx, const Anchor *anchor, const TypedValueRef &value) {
    #if 1
    if (ctx.block->tag_traceback) {
        if (value->get_type() == TYPE_Error) {
            // add info
            auto tracecall = ref(anchor, Call::from(empty_arguments_type(),
                g_sc_error_append_calltrace, {
                value, ConstAggregate::ast_from(value)
            }));
            ctx.append(tracecall);
        }
    }
    #endif
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_raise1(ctx, anchor, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValueRef) prove_LabelTemplate(const ASTContext &ctx, const LabelTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    LabelRef label = ref(node.anchor(), Label::from(node->label_kind, node->name));
    assert(ctx.frame);
    assert(ctx.block);
    ctx.frame->bind(node, label);
    TypedValueRef result;
    const char *by = "label merge";
    ASTContext labelctx;
    switch (label->label_kind) {
    case LK_Except: {
        by = "exception";
        result = SCOPES_GET_RESULT(
            prove_block(ctx.for_try(label), label->body, node->value, labelctx));
    } break;
    case LK_Break: {
        by = "break";
        result = SCOPES_GET_RESULT(
            prove_block(ctx.for_break(label), label->body, node->value, labelctx));
    } break;
    case LK_Try:
        by = "try block";
    default: {
        result = SCOPES_GET_RESULT(
            prove_block(ctx, label->body, node->value, labelctx));
    } break;
    }
    assert(result);
    if (label->merges.empty()) {
        // label does not need a merge label
        assert(ctx.block);
        result = SCOPES_GET_RESULT(
            move_single_merge_value(labelctx, ctx.block->depth, result, by));
        ctx.merge_block(label->body);
        return result;
    } else {
        make_merge(labelctx, result.anchor(), label, result);
        #if 1
        if (label->body.empty()) {
            StyledStream ss;
            stream_value(ss, label);
            stream_value(ss, node);
            stream_value(ss, result);
        }
        #endif
        assert(!label->body.empty());
        IDSet valid;
        SCOPES_CHECK_RESULT(finalize_merges(ctx, label, valid, by));
        const Type *rtype = nullptr;
        for (auto merge : label->merges) {
            rtype = SCOPES_GET_RESULT(merge_value_type(by,
                rtype, arguments_type_from_typed_values(merge->values)));
        }
        rtype = ctx.fix_merge_type(rtype);
        label->change_type(rtype);
        merge_back_valid(ctx, valid, label);
        ctx.append(label);
        return TypedValueRef(label);
    }
}

static SCOPES_RESULT(TypedValueRef) prove_Expression(const ASTContext &ctx, const ExpressionRef &expr) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    int count = (int)expr->body.size();
    if (expr->scoped) {
        Block block;
        block.set_parent(ctx.block);
        ASTContext subctx = ctx.with_block(block);
        for (int i = 0; i < count; ++i) {
            auto newsrc = SCOPES_GET_RESULT(prove(subctx, expr->body[i]));
            if (!is_returning(newsrc->get_type())) {
                SCOPES_ERROR(PrematureReturnFromExpression);
            }
        }
        TypedValueRef result;
        if (expr->value) {
            result = SCOPES_GET_RESULT(prove(subctx, expr->value));
        } else {
            result = ref(expr.anchor(), ArgumentList::from({}));
        }
        result = SCOPES_GET_RESULT(
            move_single_merge_value(subctx, block.depth - 1, result, "expression block"));
        ctx.merge_block(block);
        return result;
    } else {
        for (int i = 0; i < count; ++i) {
            SCOPES_CHECK_RESULT(prove(ctx, expr->body[i]));
        }
        if (!expr->value)
            return ref(expr.anchor(), ArgumentList::from({}));
        return SCOPES_GET_RESULT(prove(ctx, expr->value));
    }
}

static int find_key(const Symbols &symbols, Symbol key) {
    for (int i = 0; i < symbols.size(); ++i) {
        if (symbols[i] == key)
            return i;
    }
    return -1;
}

SCOPES_RESULT(void) map_keyed_arguments(const Anchor *anchor, const TypedValueRef &callee,
    TypedValues &outargs, const TypedValues &values, const Symbols &symbols, bool varargs) {
    SCOPES_RESULT_TYPE(void);
    outargs.reserve(values.size());
    std::vector<bool> mapped;
    mapped.reserve(values.size());
    size_t next_index = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        TypedValueRef arg = values[i];
        auto kt = type_key(arg->get_type());
        Symbol key = kt._0;
        int index = -1;
        if (key == SYM_Unnamed) {
            // argument without key

            // find next argument that is unmapped
            while ((next_index < mapped.size()) && mapped[next_index])
                next_index++;
            // fill up argument slots until index
            while (mapped.size() <= next_index) {
                mapped.push_back(false);
                outargs.push_back(TypedValueRef());
            }
            index = next_index;
            next_index++;
        } else {
            // find desired parameter index of key
            auto ki = find_key(symbols, key);
            if (ki >= 0) {
                // parameter with key exists
                // fill up argument slots until index
                while (mapped.size() <= (size_t)ki) {
                    mapped.push_back(false);
                    outargs.push_back(TypedValueRef());
                }
                if (mapped[ki]) {
                    SCOPES_ERROR(DuplicateParameterKey, key);
                }
                index = ki;
                // strip key from value
                arg = ref(anchor, Keyed::from(SYM_Unnamed, arg));
            } else if (varargs) {
                // no parameter with that name, but we accept varargs
                while (mapped.size() < symbols.size()) {
                    mapped.push_back(false);
                    outargs.push_back(TypedValueRef());
                }
                index = (int)outargs.size();
                mapped.push_back(false);
                outargs.push_back(TypedValueRef());
            } else {
                SCOPES_ERROR(UnknownParameterKey, key, symbols);
            }
        }
        mapped[index] = true;
        outargs[index] = arg;
    }
    TypedValueRef noneval;
    for (size_t i = 0; i < outargs.size(); ++i) {
        if (!outargs[i]) {
            if (!noneval) {
                noneval = ref(anchor, ConstAggregate::none_from());
            }
            outargs[i] = noneval;
        }
    }
    return {};
}

// used by ArgumentList & Call
static SCOPES_RESULT(TypedValueRef) prove_arguments(
    const ASTContext &ctx, TypedValues &outargs, const Values &values) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    int count = (int)values.size();
    for (int i = 0; i < count; ++i) {
        auto value = SCOPES_GET_RESULT(prove(ctx, values[i]));
        const Type *T = value->get_type();
        if (!is_returning(T)) {
            return value;
        }
        if (is_arguments_type(T)) {
            if ((i + 1) == count) {
                // last argument is appended in full
                int valcount = get_argument_count(T);
                for (int j = 0; j < valcount; ++j) {
                    outargs.push_back(
                        ExtractArgument::from(value, j));
                }
                break;
            } else {
                value = ExtractArgument::from(value, 0);
            }
        }
        outargs.push_back(ref(values[i].anchor(), value));
    }
    return TypedValueRef();
}

static SCOPES_RESULT(TypedValueRef) prove_ArgumentListTemplate(const ASTContext &ctx, const ArgumentListTemplateRef &nlist) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    TypedValues values;
    TypedValueRef noret = SCOPES_GET_RESULT(prove_arguments(ctx, values, nlist->values));
    if (noret) {
        return noret;
    }
    return ref(nlist.anchor(), ArgumentList::from(values));
}

static SCOPES_RESULT(TypedValueRef) prove_ExtractArgumentTemplate(
    const ASTContext &ctx, const ExtractArgumentTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    auto value = SCOPES_GET_RESULT(prove(ctx, node->value));
    assert(node->index >= 0);
    if (node->vararg)
        return ref(node.anchor(), ExtractArgument::variadic_from(value, node->index));
    else
        return ref(node.anchor(), ExtractArgument::from(value, node->index));
}

static SCOPES_RESULT(TypedValueRef) prove_Loop(const ASTContext &ctx, const LoopRef &loop) {
    SCOPES_RESULT_TYPE(TypedValueRef);

    TypedValues init_values;
    auto noret = SCOPES_GET_RESULT(
        prove_arguments(ctx, init_values, { loop->init }));
    if (noret) return noret;

    Types loop_types;
    for (auto &&value : init_values) {
        SCOPES_CHECK_RESULT(verify_valid(ctx, value, "loop init"));
        auto T = value->get_type();
        auto uq = try_unique(T);
        if (uq) {
            ctx.move(uq->id, value);
            // move into loop
            T = unique_type(T, ctx.unique_id());
        }
        loop_types.push_back(T);
    }

    auto loopargs = ref(loop.anchor(), LoopLabelArguments::from(
        arguments_type(loop_types)));

    LoopLabelRef newloop = ref(loop.anchor(),
        LoopLabel::from(init_values, loopargs));
    // anchor loop to the local block to avoid it landing in the wrong place
    // ctx.append(newloop);
    map_arguments_to_block(ctx.with_block(newloop->body), loopargs);
    ctx.frame->bind(loop->args, newloop->args);

    auto subctx = ctx.for_loop(newloop);
    ASTContext newctx;
    auto result = SCOPES_GET_RESULT(prove_block(subctx, newloop->body, loop->value, newctx));
    //auto rtype = result->get_type();
    make_repeat(newctx, result.anchor(), newloop, result);

    SCOPES_CHECK_RESULT(finalize_repeats(ctx, newloop, "loop repeat"));

    const Type *rtype = newloop->args->get_type();
    for (auto repeat : newloop->repeats) {
        SCOPES_CHECK_RESULT(merge_value_type("loop repeat", rtype,
            arguments_type_from_typed_values(repeat->values)));
    }

    return TypedValueRef(newloop);
}

static SCOPES_RESULT(TypedValueRef) prove_LoopArguments(const ASTContext &ctx, const ValueRef &node) { assert(false); return TypedValueRef(); }
static SCOPES_RESULT(TypedValueRef) prove_ParameterTemplate(const ASTContext &ctx, const ValueRef &node) { assert(false); return TypedValueRef(); }

const Type *try_get_const_type(const ValueRef &node) {
    if (node.isa<Const>())
        return node.cast<Const>()->get_type();
    return TYPE_Unknown;
}

const String *try_extract_string(const ValueRef &node) {
    auto ptr = node.dyn_cast<ConstPointer>();
    if (ptr && (ptr->get_type() == TYPE_String))
        return (const String *)ptr->value;
    return nullptr;
}

static SCOPES_RESULT(TypedValueRef) prove_Break(const ASTContext &ctx, const BreakRef &_break) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    if (!ctx._break) {
        SCOPES_ERROR(BreakOutsideLoop);
    }
    TypedValueRef value = SCOPES_GET_RESULT(prove(ctx, _break->value));
    return make_merge(ctx, _break.anchor(), ctx._break, value);
}

static SCOPES_RESULT(TypedValueRef) prove_RepeatTemplate(const ASTContext &ctx, const RepeatTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    if (!ctx.loop) {
        SCOPES_ERROR(RepeatOutsideLoop);
    }
    TypedValueRef value = SCOPES_GET_RESULT(prove(ctx, node->value));
    return make_repeat(ctx, node.anchor(), ctx.loop, value);
}

static SCOPES_RESULT(TypedValueRef) prove_ReturnTemplate(const ASTContext &ctx, const ReturnTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    TypedValueRef value = SCOPES_GET_RESULT(prove(ctx, node->value));
    if (ctx.frame->label) {
        assert(ctx.frame->original && ctx.frame->original->is_inline());
        // generate a merge
        return make_merge(ctx, node.anchor(), ctx.frame->label, value);
    } else {
        assert(!(ctx.frame->original && ctx.frame->original->is_inline()));
        // generate a return
        return make_return(ctx, node.anchor(), value);
    }
}

static SCOPES_RESULT(TypedValueRef) prove_MergeTemplate(const ASTContext &ctx, const MergeTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    TypedValueRef label = SCOPES_GET_RESULT(ctx.frame->resolve(node->label, ctx.function));
    if (!label.isa<Label>()) {
        SCOPES_ERROR(LabelExpected, label->get_type());
    }
    TypedValueRef value = SCOPES_GET_RESULT(prove(ctx, node->value));
    if (!is_returning(value->get_type()))
        return value;
    return make_merge(ctx, node.anchor(), label.cast<Label>(), value);
}

static SCOPES_RESULT(TypedValueRef) prove_RaiseTemplate(const ASTContext &ctx, const RaiseTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    assert(ctx.frame);
    TypedValueRef value = SCOPES_GET_RESULT(prove(ctx, node->value));
    return make_raise(ctx, node.anchor(), value);
}

bool is_value_stage_constant(const ValueRef &value) {
    return value.isa<Pure>() && (value.cast<Pure>()->get_type() != TYPE_ASTMacro);
}

static SCOPES_RESULT(TypedValueRef) prove_CompileStage(const ASTContext &ctx, const CompileStageRef &sx) {
    SCOPES_RESULT_TYPE(TypedValueRef);

    auto anchor = sx.anchor();
    auto scope = sx->env;

    auto docstr = sc_scope_get_docstring(scope, SYM_Unnamed);
    auto constant_scope = sc_scope_new_subscope(scope);
    if (sc_string_count(docstr)) {
        sc_scope_set_docstring(constant_scope, SYM_Unnamed, docstr);
    }

    Symbol last_key = SYM_Unnamed;
    auto tmp =
        ref(anchor, CallTemplate::from(g_sc_scope_new_subscope,
            { ref(sx.anchor(), ConstPointer::scope_from(constant_scope)) }));

    auto block = ref(anchor, Expression::unscoped_from());
    block->append(tmp);
    while (true) {
        auto key_value = sc_scope_next(scope, last_key);
        auto key = key_value._0;
        //StyledStream ss;
        //ss << key << std::endl;
        auto value = key_value._1;
        if (key == SYM_Unnamed)
            break;
        last_key = key;
        if (!is_value_stage_constant(value)) {
            auto keydocstr = sc_scope_get_docstring(scope, key);
            auto value1 = ref(value.anchor(), sc_extract_argument_new(value, 0));
            auto typedvalue1 = SCOPES_GET_RESULT(prove(ctx, value1));
            if (is_value_stage_constant(typedvalue1)) {
                sc_scope_set_symbol(constant_scope, key, typedvalue1);
                sc_scope_set_docstring(constant_scope, key, keydocstr);
            } else {
                ValueRef wrapvalue;
                if (typedvalue1->get_type() == TYPE_ValueRef) {
                    wrapvalue = typedvalue1;
                } else {
                    wrapvalue = wrap_value(typedvalue1->get_type(), typedvalue1);
                }
                if (wrapvalue) {
                    auto vkey = ref(anchor, ConstInt::symbol_from(key));
                    block->append(ref(anchor,
                        CallTemplate::from(g_sc_scope_set_symbol, { tmp, vkey, wrapvalue })));
                    block->append(ref(anchor,
                        CallTemplate::from(g_sc_scope_set_docstring, { tmp, vkey,
                            ref(anchor, ConstPointer::string_from(keydocstr)) })));
                }
            }
        }
    }

    block->append(
        ref(anchor, CallTemplate::from(g_bitcast, {
            ref(anchor, CallTemplate::from(g_sc_eval, {
                ref(anchor, ConstPointer::anchor_from(anchor)),
                ref(anchor, ConstPointer::list_from(sx->next)),
                tmp })),
            ref(anchor, ConstPointer::type_from(TYPE_CompileStage))
        }))
    );
    //StyledStream ss;
    //stream_ast(ss, block, StreamASTFormat());
    return prove(ctx, block);
}

static SCOPES_RESULT(TypedValueRef) prove_KeyedTemplate(const ASTContext &ctx,
    const KeyedTemplateRef &keyed) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    auto value = SCOPES_GET_RESULT(prove(ctx, keyed->value));
    assert(!value.isa<ArgumentList>());
    return ref(keyed.anchor(), Keyed::from(keyed->key, value));
}

template<typename T, ValueKind kind>
static SCOPES_RESULT(TValueRef<T>) extract_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(TValueRef<T>);
    auto constval = value.dyn_cast<T>();
    if (!constval) {
        SCOPES_ERROR(ConstantValueKindMismatch,
            kind, value->kind());
    }
    return constval;
}

template<typename T>
static SCOPES_RESULT(TValueRef<T>) extract_typed_constant(const Type *want, ValueRef value) {
    SCOPES_RESULT_TYPE(TValueRef<T>);
    const Type *TT = nullptr;
    if (value.isa<PureCast>()) {
        TT = value.cast<PureCast>()->get_type();
        value = ref(value.anchor(), value.cast<PureCast>()->value);
    }
    auto constval = value.dyn_cast<T>();
    if (!constval) {
        SCOPES_ERROR(TypedConstantValueKindMismatch, want, value->kind());
    }
    if (!TT) {
        TT = constval->get_type();
    }
    SCOPES_CHECK_RESULT(verify(TT, want));
    return constval;
}

SCOPES_RESULT(const Type *) extract_type_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(const Type *);
    ConstPointerRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_Type, value));
    return (const Type *)x->value;
}

SCOPES_RESULT(const Closure *) extract_closure_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(const Closure *);
    ConstPointerRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_Closure, value));
    return (const Closure *)x->value;
}

SCOPES_RESULT(FunctionRef) extract_function_constant(const ValueRef &value) {
    return extract_constant<Function, VK_Function>(value);
}

SCOPES_RESULT(TemplateRef) extract_template_constant(const ValueRef &value) {
    return extract_constant<Template, VK_Template>(value);
}

SCOPES_RESULT(sc_ast_macro_func_t) extract_astmacro_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(sc_ast_macro_func_t);
    ConstPointerRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_ASTMacro, value));
    return (sc_ast_macro_func_t)x->value;
}

SCOPES_RESULT(const List *) extract_list_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(const List *);
    ConstPointerRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_List, value));
    return (const List *)x->value;
}

SCOPES_RESULT(const String *) extract_string_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(const String *);
    ConstPointerRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_String, value));
    return (const String *)x->value;
}

SCOPES_RESULT(Builtin) extract_builtin_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(Builtin);
    ConstIntRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstInt>(TYPE_Builtin, value));
    return Builtin((KnownSymbol)x->value);
}

SCOPES_RESULT(Symbol) extract_symbol_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(Symbol);
    ConstIntRef x = SCOPES_GET_RESULT(extract_typed_constant<ConstInt>(TYPE_Symbol, value));
    return Symbol::wrap(x->value);
}

SCOPES_RESULT(uint64_t) extract_integer_constant(const ValueRef &value) {
    SCOPES_RESULT_TYPE(uint64_t);
    auto val = extract_constant<ConstInt, VK_ConstInt>(value);
    ConstIntRef x = SCOPES_GET_RESULT(val);
    return x->value;
}

SCOPES_RESULT(ConstAggregateRef) extract_vector_constant(const ValueRef &value) {
    return extract_constant<ConstAggregate, VK_ConstAggregate>(value);
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

static const Type *unique_result_type(const ASTContext &ctx, const Type *T) {
    T = strip_lifetime(T);
    if (!is_plain(T)) {
        return unique_type(T, ctx.unique_id());
    }
    return T;
}

static void collect_view_argument(const ASTContext &ctx, TypedValueRef &arg, IDSet &ids) {
    auto T = arg->get_type();
    if (is_unique(T)) {
        build_view(ctx, arg.anchor(), arg);
        T = arg->get_type();
        assert(is_view(T));
    }
    auto vq = try_view(T);
    if (vq) {
        ids = union_idset(ids, vq->ids);
    } else {
        if (!is_plain(T)) {
            StyledStream ss;
            ss << "internal error: trying to view " << arg << " but it is untracked" << std::endl;
        }
        assert(is_plain(T));
    }
}

static const Type *view_result_type(const Type *T, const IDSet &ids) {
    T = strip_lifetime(T);
    if (ids.empty()) {
        if (!is_plain(T)) {
            StyledStream ss;
            ss << "internal error: " << T << " has no views" << std::endl;
        }
        assert(is_plain(T));
        return T;
    }
    return view_type(T, ids);
}

static const Type *view_result_type(const ASTContext &ctx, const Type *T,
    TypedValueRef &arg1) {
    IDSet ids;
    collect_view_argument(ctx, arg1, ids);
    return view_result_type(T, ids);
}

static const Type *view_result_type(const ASTContext &ctx, const Type *T,
    TypedValueRef &arg1, TypedValueRef &arg2) {
    IDSet ids;
    collect_view_argument(ctx, arg1, ids);
    collect_view_argument(ctx, arg2, ids);
    return view_result_type(T, ids);
}

static const Type *view_result_type(const ASTContext &ctx, const Type *T,
    TypedValueRef &arg1, TypedValueRef &arg2, TypedValueRef &arg3) {
    IDSet ids;
    collect_view_argument(ctx, arg1, ids);
    collect_view_argument(ctx, arg2, ids);
    collect_view_argument(ctx, arg3, ids);
    return view_result_type(T, ids);
}

static SCOPES_RESULT(void) build_deref(
    const ASTContext &ctx, const Anchor *anchor, TypedValueRef &val) {
    SCOPES_RESULT_TYPE(void);
    auto T = val->get_type();
    auto rq = try_qualifier<ReferQualifier>(T);
    if (rq) {
        SCOPES_CHECK_RESULT(verify_readable(rq, T));
        auto retT = strip_qualifier<ReferQualifier>(T);
        retT = view_result_type(ctx, retT, val);
        auto call = ref(anchor, Call::from(retT, g_deref, { val }));
        ctx.append(call);
        val = call;
        return {};
    }
    return {};
}

static SCOPES_RESULT(void) build_deref_move(
    const ASTContext &ctx, const Anchor *anchor, TypedValueRef &val) {
    SCOPES_RESULT_TYPE(void);
    auto T = val->get_type();
    auto rq = try_qualifier<ReferQualifier>(T);
    if (rq) {
        SCOPES_CHECK_RESULT(verify_readable(rq, T));
        auto retT = strip_qualifier<ReferQualifier>(T);
        auto call = ref(anchor,
            Call::from(unique_result_type(ctx, retT), g_deref, { val }));
        ctx.append(call);
        auto uq = try_unique(T);
        if (uq) {
            ctx.move(uq->id, val);
        }
        val = call;
    }
    return {};
}

#define CHECKARGS(MINARGS, MAXARGS) \
    SCOPES_CHECK_RESULT((checkargs<MINARGS, MAXARGS>(argcount)))
#define ARGTYPE0() ({ \
        TypedValueRef newcall = ref(call.anchor(), \
            Call::from(empty_arguments_type(), callee, values)); \
        newcall; \
    })
#define ARGTYPE1(ARGT) ({ \
        TypedValueRef newcall = ref(call.anchor(), \
            Call::from(ARGT, callee, values)); \
        newcall; \
    })
#define NEW_ARGTYPE1(ARGT) ({ \
        TypedValueRef newcall = ref(call.anchor(), \
            Call::from(unique_result_type(ctx, ARGT), callee, values)); \
        newcall; \
    })
#define DEP_ARGTYPE1(ARGT, ...) ({ \
        const Type *_retT = view_result_type(ctx, ARGT, __VA_ARGS__); \
        TypedValueRef newcall = ref(call.anchor(), \
            Call::from(_retT, callee, values)); \
        newcall; \
    })
#define DEREF(NAME) \
        SCOPES_CHECK_RESULT(build_deref(ctx, call.anchor(), NAME));
#define MOVE_DEREF(NAME) \
        SCOPES_CHECK_RESULT(build_deref_move(ctx, call.anchor(), NAME));
#define READ_VALUE(NAME) \
        assert(argn < argcount); \
        auto && NAME = values[argn++]; \
        DEREF(NAME);
#define READ_NODEREF_VALUE(NAME) \
        assert(argn < argcount); \
        auto && NAME = values[argn++];
#define READ_NODEREF_TYPEOF(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        const Type *NAME = _ ## NAME->get_type();
#define READ_TYPEOF(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        DEREF(_ ## NAME); \
        const Type *NAME = _ ## NAME->get_type();
#define READ_NODEREF_STORAGETYPEOF(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        const Type *typeof_ ## NAME = _ ## NAME->get_type(); \
        const Type *NAME = SCOPES_GET_RESULT(storage_type(typeof_ ## NAME));
#define READ_STORAGETYPEOF(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        DEREF(_ ## NAME); \
        const Type *typeof_ ## NAME = _ ## NAME->get_type(); \
        const Type *NAME = SCOPES_GET_RESULT(storage_type(typeof_ ## NAME));
#define MOVE_STORAGETYPEOF(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        MOVE_DEREF(_ ## NAME); \
        const Type *typeof_ ## NAME = _ ## NAME->get_type(); \
        const Type *NAME = SCOPES_GET_RESULT(storage_type(typeof_ ## NAME));
#define READ_INT_CONST(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        auto NAME = SCOPES_GET_RESULT(extract_integer_constant(_ ## NAME));
#define READ_TYPE_CONST(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        auto NAME = SCOPES_GET_RESULT(extract_type_constant(_ ## NAME));
#define READ_VECTOR_CONST(NAME) \
        assert(argn < argcount); \
        auto NAME = SCOPES_GET_RESULT(extract_vector_constant(values[argn++]));
#define READ_SYMBOL_CONST(NAME) \
        assert(argn < argcount); \
        auto &&_ ## NAME = values[argn++]; \
        auto NAME = SCOPES_GET_RESULT(extract_symbol_constant(_ ## NAME));

static const Type *canonical_return_type(const FunctionRef &fn, const Type *rettype,
    bool is_except = false) {
    if (!is_returning_value(rettype))
        return rettype;
    std::unordered_map<int, int> idmap;
    Types rettypes;
    int acount = get_argument_count(rettype);
    for (int i = 0; i < acount; ++i) {
        auto T = get_argument(rettype, i);
        auto uq = try_qualifier<UniqueQualifier>(T);
        if (uq) {
            int newid = (is_except?FirstUniqueError:FirstUniqueOutput) - i;
            idmap.insert({uq->id, newid});
            T = unique_type(T, newid);
        }
        rettypes.push_back(T);
    }
    for (int i = 0; i < acount; ++i) {
        auto &&T = rettypes[i];
        auto vq = try_qualifier<ViewQualifier>(T);
        if (vq) {
            IDSet ids;
            for (auto id : vq->ids) {
                auto it = idmap.find(id);
                if (it != idmap.end()) {
                    ids.insert(it->second);
                } else {
                    ids.insert(id);
                }
            }
            T = view_type(strip_qualifier<ViewQualifier>(T), ids);
        }
    }
    return arguments_type(rettypes);
}

static SCOPES_RESULT(const Type *) get_function_type(const FunctionRef &fn) {
    SCOPES_RESULT_TYPE(const Type *);

    //const Block *block = &fn->body;

    Types params;
    for (int i = 0; i < fn->params.size(); ++i) {
        auto param = fn->params[i];
        auto T = param->get_type();
        auto uq = try_unique(T);
        if (uq && fn->valid.count(uq->id)) {
            // has this parameter been spent? if not, it's a view
            T = view_type(T, {});
            param->retype(T);
        }
        params.push_back(T);
    }

    const Type *rettype = TYPE_NoReturn;
    for (auto _return : fn->returns) {
        rettype = SCOPES_GET_RESULT(merge_value_type("return", rettype,
            arguments_type_from_typed_values(_return->values)));
    }
    rettype = canonical_return_type(fn, rettype);
    const Type *raisetype = TYPE_NoReturn;
    for (auto _raise : fn->raises) {
        raisetype = SCOPES_GET_RESULT(merge_value_type("raise", raisetype,
            arguments_type_from_typed_values(_raise->values)));
    }
    raisetype = canonical_return_type(fn, raisetype, true);

    return native_ro_pointer_type(raising_function_type(raisetype, rettype, params));
}

static SCOPES_RESULT(const Type *) ensure_function_type(const FunctionRef &fn) {
    SCOPES_RESULT_TYPE(const Type *);
    const Type *fT = SCOPES_GET_RESULT(get_function_type(fn));
    if (fn->is_typed()) {
        if (fT != fn->get_type()) {
            SCOPES_ERROR(RecursiveFunctionChangedType, fn->get_type(), fT);
        }
    } else {
        fn->set_type(fT);
    }
    return fT;
}

static void keys_from_function_type(Symbols &keys, const FunctionType *ft) {
    for (auto T : ft->argument_types) {
        keys.push_back(type_key(T)._0);
    }
}

static bool keys_from_parameters(Symbols &keys, const ParameterTemplates &params) {
    bool vararg = false;
    for (auto param : params) {
        if (param->variadic) {
            vararg = true;
        } else {
            keys.push_back(param->name);
        }
    }
    return vararg;
}

static SCOPES_RESULT(const Type *) ptr_to_ref(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    T = SCOPES_GET_RESULT(storage_type(T));
    SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
    auto pt = cast<PointerType>(T);
    return refer_type(
        pt->element_type, pt->flags, pt->storage_class);
}

static SCOPES_RESULT(const Type *) ref_to_ptr(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    auto rq = SCOPES_GET_RESULT(verify_refer(T));
    return copy_qualifiers(
        rq->get_pointer_type(strip_qualifiers(T)),
        strip_qualifier<ReferQualifier>(T));
}

template<typename T>
SCOPES_RESULT(void) sanitize_tuple_index(const Anchor *anchor, const Type *ST,
    const T *type, uint64_t &arg, TypedValueRef &_arg) {
    SCOPES_RESULT_TYPE(void);
    if (_arg->get_type() == TYPE_Symbol) {
        auto sym = Symbol::wrap(arg);
        size_t idx = type->field_index(sym);
        if (idx == (size_t)-1) {
            SCOPES_ERROR(UnknownField, sym, ST);
        }
        // rewrite field
        arg = idx;
        _arg = ref(anchor, ConstInt::from(TYPE_I32, idx));
    } else if (_arg->get_type() != TYPE_I32) {
        _arg = ref(anchor, ConstInt::from(TYPE_I32, arg));
    }
    return {};
}

const Type *remap_unique_return_arguments(
    const ASTContext &ctx, ID2SetMap &idmap, const Type *rt) {
    if (is_returning_value(rt)) {
        // remap return type
        int acount = get_argument_count(rt);
        Types rettypes;
        rettypes.reserve(acount);
        for (int i = 0; i < acount; ++i) {
            const Type *argT = get_argument(rt, i);
            auto uq = try_unique(argT);
            if (uq) {
                auto newid = ctx.unique_id();
                argT = unique_type(argT, newid);
                map_unique_id(idmap, uq->id, newid);
            } else {
                auto vq = try_view(argT);
                if (vq) {
                    IDSet newids;
                    for (auto vid : vq->ids) {
                        auto it = idmap.find(vid);
                        assert(it != idmap.end());
                        for (auto destid : it->second) {
                            newids.insert(destid);
                        }
                    }
                    argT = view_type(strip_view(argT), newids);
                }
            }
            rettypes.push_back(argT);
        }
        rt = arguments_type(rettypes);
    }
    return rt;
}

SCOPES_RESULT(void) verify_cast_lifetime(const Type *SrcT, const Type *DestT) {
    SCOPES_RESULT_TYPE(void);
    if (!is_plain(DestT) && is_plain(SrcT) && !is_view(SrcT)) {
        SCOPES_ERROR(PlainToUniqueCast, SrcT, DestT);
    }
    return {};
}

static SCOPES_RESULT(TypedValueRef) prove_CallTemplate(
    const ASTContext &ctx, const CallTemplateRef &call) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    //const Anchor *anchor = get_best_anchor(call);
    TypedValueRef callee = ref(call->callee.anchor(),
        SCOPES_GET_RESULT(prove(ctx, call->callee)));
    TypedValues values;
    auto noret = SCOPES_GET_RESULT(prove_arguments(ctx, values, call->args));
    if (noret) return noret;
    bool rawcall = call->is_rawcall();
    int redirections = 0;
repeat:
    SCOPES_CHECK_RESULT(verify_valid(ctx, callee, "callee"));
    SCOPES_TRACE_PROVE_EXPR(call);
    const Type *T = callee->get_type();
    if (!rawcall) {
        assert(redirections < 16);
        ValueRef dest;
        if (T->lookup_call_handler(dest)) {
            values.insert(values.begin(), callee);
            callee = SCOPES_GET_RESULT(prove(ctx, dest));
            //call = ref(dest.anchor(), call);
            redirections++;
            goto repeat;
        }
    }
    if (is_function_pointer(T)) {
        TypedValues args;
        Symbols keys;
        keys_from_function_type(keys, extract_function_type(T));
        SCOPES_CHECK_RESULT(map_keyed_arguments(call.anchor(), callee, args, values, keys, false));
        values = args;
        SCOPES_CHECK_RESULT(verify_valid(ctx, values, "function call"));
    } else if (T == TYPE_Closure) {
        const Closure *cl = SCOPES_GET_RESULT((extract_closure_constant(callee)));
        {
            TypedValues args;
            Symbols keys;
            bool vararg = keys_from_parameters(keys, cl->func->params);
            SCOPES_CHECK_RESULT(map_keyed_arguments(call.anchor(), callee, args, values, keys, vararg));
            values = args;
        }
        if (cl->func->is_inline()) {
            return SCOPES_GET_RESULT(prove_inline(ctx, cl, values));
        } else {
            SCOPES_CHECK_RESULT(verify_valid(ctx, values, "call"));
            Types types;
            for (auto &&arg : values) {
                types.push_back(arg->get_type());
            }
            callee = SCOPES_GET_RESULT(prove(
                ref(callee.anchor(), cl->frame),
                ref(callee.anchor(), cl->func),
                types));
            FunctionRef f = callee.cast<Function>();
            if (f->complete) {
                T = callee->get_type();
            } else {
                T = SCOPES_GET_RESULT(ensure_function_type(f));
            }
        }
    } else if (T == TYPE_ASTMacro) {
        auto fptr = SCOPES_GET_RESULT(extract_astmacro_constant(callee));
        assert(fptr);
            SCOPES_CHECK_RESULT(verify_valid(ctx, values, "spice call"));
        SCOPES_ASTCONTEXT(ctx);
        auto result = fptr(ref(call.anchor(), ArgumentList::from(values)));
        if (result.ok) {
            ValueRef value = result._0;
            if (!value) {
                SCOPES_ERROR(SpiceMacroReturnedNull);
            }
            //value = ref(call.anchor(), result._0);
            return SCOPES_GET_RESULT(prove(ctx, value));
        } else {
            SCOPES_RETURN_TRACE_ERROR(result.except);
        }
    } else if (T == TYPE_Builtin) {
        //SCOPES_CHECK_RESULT(anycl.verify(TYPE_Builtin));
        Builtin b = SCOPES_GET_RESULT(extract_builtin_constant(callee));
        size_t argcount = values.size();
        size_t argn = 0;
        if (b.value() == FN_IsValid) {
            CHECKARGS(1, 1);
            bool valid = ctx.block->is_valid(ValueIndex(values[0]));
            return TypedValueRef(call.anchor(), ConstInt::from(TYPE_Bool, valid));
        }
        SCOPES_CHECK_RESULT(verify_valid(ctx, values, "builtin call"));
        switch(b.value()) {
        /*** DEBUGGING ***/
        case FN_DumpUniques: {
            StyledStream ss(SCOPES_CERR);
            ss << call.anchor() << " dump-uniques:";
            for (auto id : ctx.block->valid) {
                ss << " ";
                ss << id;
                auto info = ctx.function->get_unique_info(id);
                ss << "[" << info.get_depth() << "](" << info.value << ")";
            }
            ss << std::endl;
            return ref(call.anchor(), ArgumentList::from(values));
        } break;
        case FN_Dump: {
            StyledStream ss(SCOPES_CERR);
            ss << call.anchor() << " dump:";
            for (auto arg : values) {
                ss << " ";
                stream_value(ss, arg, StreamValueFormat::singleline());
            }
            ss << std::endl;
            return ref(call.anchor(), ArgumentList::from(values));
        } break;
        case FN_DumpDebug: {
            StyledStream ss(SCOPES_CERR);
            ss << call.anchor() << " dump-debug:";
            for (auto arg : values) {
                ss << std::endl;
                stream_value(ss, arg, StreamValueFormat::debug());
            }
            ss << std::endl;
            return ref(call.anchor(), ArgumentList::from(values));
        } break;
        case FN_DumpAST: {
            StyledStream ss(SCOPES_CERR);
            ss << call.anchor() << " dump-ast:";
            for (auto arg : values) {
                ss << std::endl;
                stream_value(ss, arg);
            }
            ss << std::endl;
            return ref(call.anchor(), ArgumentList::from(values));
        } break;
        case FN_DumpTemplate: {
            StyledStream ss(SCOPES_CERR);
            ss << call.anchor() << " dump-template:";
            for (auto arg : call->args) {
                ss << std::endl;
                stream_value(ss, arg);
            }
            return ref(call.anchor(), ArgumentList::from(values));
        } break;
        /*** ANNOTATION ***/
        case FN_Annotate: {
            // takes any kind of argument
            return ARGTYPE0();
        } break;
        case FN_HideTraceback: {
            CHECKARGS(0, 0);
            ctx.block->tag_traceback = false;
            return ref(call.anchor(), ArgumentList::from({}));
        } break;
        /*** VIEW INFERENCE ***/
        case FN_Move: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            auto uq = try_unique(X);
            if (!uq) {
                SCOPES_ERROR(UniqueValueExpected, _X->get_type());
            }
            build_move(ctx, call.anchor(), _X);
            return _X;
        } break;
        case SYM_DropHandler: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            (void)X;
            return SCOPES_GET_RESULT(build_drop(ctx, call.anchor(), _X));
        } break;
        case FN_View: {
            CHECKARGS(0, -1);
            while (argn < argcount) {
                READ_NODEREF_TYPEOF(X);
                auto uq = try_unique(X);
                if (!uq) {
                    // no effect
                    continue;
                }
                build_view(ctx, call.anchor(), _X);
            }
            return ref(call.anchor(), ArgumentList::from(values));
            //return _X;
        } break;
        case FN_Lose: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            auto uq = try_unique(X);
            if (!uq) {
                SCOPES_ERROR(UniqueValueExpected, _X->get_type());
            }

            const Type *DestT = SCOPES_GET_RESULT(storage_type(X));

            auto rq = try_qualifier<ReferQualifier>(X);
            if (rq) {
                DestT = qualify(DestT, { rq });
            }

            ctx.move(uq->id, _X);
            return ARGTYPE1(DestT);
        } break;
        case FN_Dupe: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);

            const Type *DestT = SCOPES_GET_RESULT(storage_type(X));

            auto rq = try_qualifier<ReferQualifier>(X);
            if (rq) {
                DestT = qualify(DestT, { rq });
            }
            return ARGTYPE1(DestT);
        } break;
        case FN_Track: {
            CHECKARGS(2, 2);
            READ_NODEREF_TYPEOF(SrcT);
            READ_TYPE_CONST(DestT);

            const Type *SDestT = SCOPES_GET_RESULT(storage_type(DestT));

            DestT = strip_qualifiers(DestT);

            if (strip_qualifier<ReferQualifier>(SrcT) != SDestT) {
                SCOPES_ERROR(IncompatibleStorageTypeForUnique, SrcT, DestT);
            }

            auto rq = try_qualifier<ReferQualifier>(SrcT);
            if (rq) {
                DestT = qualify(DestT, { rq });
                _DestT = ref(_DestT.anchor(), ConstPointer::type_from(DestT));
            }

            return NEW_ARGTYPE1(DestT);
        } break;
        case FN_Viewing: {
            for (size_t i = 0; i < values.size(); ++i) {
                READ_NODEREF_VALUE(value);
                auto param = value.dyn_cast<Parameter>();
                if (!param) {
                    SCOPES_ERROR(ValueKindMismatch, VK_Parameter, value->kind());
                }
                param->retype(view_type(param->get_type(), {}));
            }
            return ref(call.anchor(), ArgumentList::from({}));
        } break;
        /*** ARGUMENTS ***/
        case FN_VaCountOf: {
            return TypedValueRef(call.anchor(), ConstInt::from(TYPE_I32, argcount));
        } break;
        case FN_NullOf: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return NEW_ARGTYPE1(T);
        } break;
        case FN_Undef: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return NEW_ARGTYPE1(T);
        } break;
        case FN_TypeOf: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(A);
            return TypedValueRef(call.anchor(),
                ConstPointer::type_from(strip_qualifiers(A)));
        } break;
        /*** SPIR-V FORMS ***/
        case FN_Sample: {
            CHECKARGS(2, -1);
            READ_STORAGETYPEOF(ST);
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = SCOPES_GET_RESULT(storage_type(sit->type));
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
            auto it = cast<ImageType>(ST);
            while (argn < argcount) {
                READ_VALUE(val);
            }
            return DEP_ARGTYPE1(it->type, _ST);
        } break;
        case FN_ImageQuerySize: {
            CHECKARGS(1, -1);
            READ_STORAGETYPEOF(ST);
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = SCOPES_GET_RESULT(storage_type(sit->type));
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
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
                SCOPES_ERROR(UnsupportedDimensionality, it->dim);
                break;
            }
            if (it->arrayed) {
                comps++;
            }

            const Type *retT = TYPE_I32;
            if (comps != 1) {
                retT = vector_type(TYPE_I32, comps).assert_ok();
            }
            return DEP_ARGTYPE1(retT, _ST);
        } break;
        case FN_ImageQueryLod: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ST);
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = SCOPES_GET_RESULT(storage_type(sit->type));
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));

            return DEP_ARGTYPE1(vector_type(TYPE_F32, 2).assert_ok(), _ST);
        } break;
        case FN_ImageQueryLevels:
        case FN_ImageQuerySamples: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(ST);
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = SCOPES_GET_RESULT(storage_type(sit->type));
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
            return DEP_ARGTYPE1(TYPE_I32, _ST);
        } break;
        case FN_ImageRead: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ST);
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
            auto it = cast<ImageType>(ST);
            return DEP_ARGTYPE1(it->type, _ST);
        } break;
        case SFXFN_ExecutionMode: {
            CHECKARGS(1, 4);
            READ_SYMBOL_CONST(sym);
            switch(sym.value()) {
            #define T(NAME) \
                case SYM_SPIRV_ExecutionMode ## NAME: break;
                B_SPIRV_EXECUTION_MODE()
            #undef T
                default:
                    SCOPES_ERROR(UnsupportedExecutionMode, sym);
                    break;
            }
            for (size_t i = 1; i < values.size(); ++i) {
                READ_INT_CONST(x);
                (void)x;
            }
            return ARGTYPE0();
        } break;
        /*** MISC ***/
        case OP_Tertiary: {
            CHECKARGS(3, 3);
            READ_STORAGETYPEOF(T1);
            READ_TYPEOF(T2);
            READ_TYPEOF(T3);
            {
                SCOPES_TRACE_PROVE_ARG(_T1);
                SCOPES_CHECK_RESULT(verify_bool_vector(T1));
            }
            {
                SCOPES_TRACE_PROVE_ARG(_T3);
                SCOPES_CHECK_RESULT(verify(T2, T3));
            }
            if (T1->kind() == TK_Vector) {
                SCOPES_TRACE_PROVE_ARG(_T2);
                auto ST2 = SCOPES_GET_RESULT(storage_type(T2));
                SCOPES_CHECK_RESULT(verify_vector_sizes(T1, ST2));
            }
            return DEP_ARGTYPE1(T2, _T1, _T2, _T3);
        } break;
        case FN_Bitcast: {
            CHECKARGS(2, 2);
            READ_NODEREF_TYPEOF(SrcT);
            READ_TYPE_CONST(DestT);
            if (SrcT == DestT) {
                return _SrcT;
            } else {
                //DEREF(_SrcT);
                const Type *SSrcT = SCOPES_GET_RESULT(storage_type(SrcT));
                const Type *SDestT = SCOPES_GET_RESULT(storage_type(DestT));
                if (canonical_typekind(SSrcT->kind())
                        != canonical_typekind(SDestT->kind())) {
                    SCOPES_ERROR(CastCategoryError, SrcT, DestT);
                }
                if (SSrcT != SDestT) {
                    switch (SSrcT->kind()) {
                    case TK_Array:
                    //case TK_Vector:
                    case TK_Tuple:
                    case TK_Union: {
                        SCOPES_ERROR(CastIncompatibleAggregateType, SSrcT);
                    } break;
                    default: break;
                    }
                }

                bool target_is_plain = is_plain(DestT);

                DestT = strip_qualifiers(DestT);
                SCOPES_CHECK_RESULT(verify_cast_lifetime(SrcT, DestT));

                auto rq = try_qualifier<ReferQualifier>(SrcT);
                if (rq) {
                    DestT = qualify(DestT, { rq });
                    _DestT = ref(_DestT.anchor(), ConstPointer::type_from(DestT));
                }
                if (_SrcT.isa<Pure>() && target_is_plain) {
                    return TypedValueRef(ref(call.anchor(),
                        PureCast::from(DestT, _SrcT.cast<Pure>())));
                } else {
                    return DEP_ARGTYPE1(DestT, _SrcT);
                }
            }
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT((verify_kind<TK_Pointer>(SCOPES_GET_RESULT(storage_type(DestT)))));
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if (cast<RealType>(T)->width < cast<RealType>(DestT)->width) {
                SCOPES_ERROR(InvalidOperands, b, T, DestT);
            }
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if (cast<RealType>(T)->width > cast<RealType>(DestT)->width) {
                SCOPES_ERROR(InvalidOperands, b, T, DestT);
            }
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_FPToUI:
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            if ((T != TYPE_F32) && (T != TYPE_F64)) {
                SCOPES_ERROR(InvalidOperands, b, T, DestT);
            }
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_UIToFP:
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if ((DestT != TYPE_F32) && (DestT != TYPE_F64)) {
                SCOPES_ERROR(InvalidOperands, b, T, DestT);
            }
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_ZExt:
        case FN_SExt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return DEP_ARGTYPE1(DestT, _T);
        } break;
        case FN_ExtractElement: {
            CHECKARGS(2, 2);
            READ_NODEREF_STORAGETYPEOF(T);
            READ_STORAGETYPEOF(idx);
            SCOPES_CHECK_RESULT(verify_kind<TK_Vector>(T));
            auto vi = cast<VectorType>(T);
            SCOPES_CHECK_RESULT(verify_integer(idx));
            auto rq = try_qualifier<ReferQualifier>(typeof_T);
            auto RT = vi->element_type;
            if (rq) {
                const Type *retT = view_result_type(ctx, qualify(RT, { rq }), _T, _idx);
                return TypedValueRef(call.anchor(),
                    Call::from(retT, g_getelementref, { _T, _idx }));
            } else {
                return DEP_ARGTYPE1(RT, _T, _idx);
            }
        } break;
        case FN_InsertElement: {
            CHECKARGS(3, 3);
            READ_STORAGETYPEOF(T);
            READ_STORAGETYPEOF(ET);
            READ_STORAGETYPEOF(idx);
            SCOPES_CHECK_RESULT(verify_integer(idx));
            SCOPES_CHECK_RESULT(verify_kind<TK_Vector>(T));
            auto vi = cast<VectorType>(T);
            SCOPES_CHECK_RESULT(verify(SCOPES_GET_RESULT(storage_type(vi->element_type)), ET));
            return DEP_ARGTYPE1(typeof_T, _T, _ET, _idx);
        } break;
        case FN_ShuffleVector: {
            CHECKARGS(3, 3);
            READ_STORAGETYPEOF(TV1);
            READ_STORAGETYPEOF(TV2);
            READ_VECTOR_CONST(mask);
            const Type *TMask = mask->get_type();
            SCOPES_CHECK_RESULT(verify_kind<TK_Vector>(TV1));
            SCOPES_CHECK_RESULT(verify_kind<TK_Vector>(TV2));
            SCOPES_CHECK_RESULT(verify_kind<TK_Vector>(TMask));
            SCOPES_CHECK_RESULT(verify(TV1, TV2));
            auto vi = cast<VectorType>(TV1);
            auto mask_vi = cast<VectorType>(TMask);
            SCOPES_CHECK_RESULT(verify(TYPE_I32, mask_vi->element_type));
            size_t incount = vi->count * 2;
            size_t outcount = mask_vi->count;
            for (size_t i = 0; i < outcount; ++i) {
                SCOPES_CHECK_RESULT(verify_range(
                    cast<ConstInt>(mask->values[i])->value,
                    incount));
            }
            return DEP_ARGTYPE1(
                SCOPES_GET_RESULT(vector_type(vi->element_type, outcount)), _TV1, _TV2);
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(T);
            SCOPES_CHECK_RESULT(verify_real_vector(T));
            if (T->kind() == TK_Vector) {
                return DEP_ARGTYPE1(cast<VectorType>(T)->element_type, _T);
            } else {
                return DEP_ARGTYPE1(typeof_T, _T);
            }
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            READ_TYPEOF(A);
            SCOPES_CHECK_RESULT(verify_real_ops(A));
            return DEP_ARGTYPE1(A, _A);
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(A);
            READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_real_vector(A, 3));
            SCOPES_CHECK_RESULT(verify(typeof_A, B));
            return DEP_ARGTYPE1(typeof_A, _A);
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            READ_TYPEOF(A);
            READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_real_ops(A, B));
            const Type *T = SCOPES_GET_RESULT(storage_type(A));
            if (T->kind() == TK_Vector) {
                return DEP_ARGTYPE1(cast<VectorType>(T)->element_type, _A, _B);
            } else {
                return DEP_ARGTYPE1(A, _A, _B);
            }
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            READ_NODEREF_STORAGETYPEOF(T);
            READ_STORAGETYPEOF(idx);
            const Type *RT = nullptr;
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                auto rq = try_qualifier<ReferQualifier>(typeof_T);
                if (rq) {
                    SCOPES_CHECK_RESULT(verify_integer(idx));
                    RT = SCOPES_GET_RESULT(ai->type_at_index(0));
                } else {
                    // index must be constant
                    auto iidx = SCOPES_GET_RESULT(extract_integer_constant(_idx));
                    RT = SCOPES_GET_RESULT(ai->type_at_index(iidx));
                }
            } break;
            case TK_Tuple: {
                auto iidx = SCOPES_GET_RESULT(extract_integer_constant(_idx));
                auto ti = cast<TupleType>(T);
                SCOPES_CHECK_RESULT(sanitize_tuple_index(call.anchor(), T, ti, iidx, _idx));
                RT = SCOPES_GET_RESULT(ti->type_at_index(iidx));
            } break;
            case TK_Union: {
                auto iidx = SCOPES_GET_RESULT(extract_integer_constant(_idx));
                auto ui = cast<UnionType>(T);
                SCOPES_CHECK_RESULT(sanitize_tuple_index(call.anchor(), T, ui, iidx, _idx));
                RT = SCOPES_GET_RESULT(ui->type_at_index(iidx));
                _idx = ref(_idx.anchor(), ConstInt::from(TYPE_I32, 0));
                auto TT = tuple_type({RT}).assert_ok();
                RT = type_key(RT)._1;
                auto rq = try_qualifier<ReferQualifier>(typeof_T);
                if (rq) {
                    TT = qualify(TT, {rq});
                    auto retT = view_result_type(ctx, TT, _T);
                    TypedValueRef newcall2 = ref(call.anchor(), Call::from(retT, g_bitcast,
                        { _T, ref(call.anchor(), ConstPointer::type_from(TT)) }));
                    ctx.append(newcall2);
                    retT = view_result_type(ctx, qualify(RT, { rq }), newcall2);
                    TypedValueRef newcall3 = ref(call.anchor(), Call::from(retT,
                        g_getelementref, { newcall2, _idx }));
                    ctx.append(newcall3);
                    return newcall3;
                } else {
                    auto retT = view_result_type(ctx, TT, _T);
                    TypedValueRef newcall2 = ref(call.anchor(), Call::from(retT, g_bitcast,
                        { _T, ref(call.anchor(), ConstPointer::type_from(TT)) }));
                    ctx.append(newcall2);
                    _T = newcall2;
                    return DEP_ARGTYPE1(RT, _T);
                }
            } break;
            default: {
                SCOPES_ERROR(InvalidArgumentTypeForBuiltin, b, T);
            } break;
            }
            RT = type_key(RT)._1;
            auto rq = try_qualifier<ReferQualifier>(typeof_T);
            if (rq) {
                auto retT = view_result_type(ctx, qualify(RT, { rq }), _T);
                TypedValueRef newcall2 = ref(call.anchor(),
                    Call::from(retT, g_getelementref, { _T, _idx }));
                ctx.append(newcall2);
                return newcall2;
            } else {
                return DEP_ARGTYPE1(RT, _T);
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
                SCOPES_ERROR(InvalidArgumentTypeForBuiltin, b, T);
            } break;
            }
            return DEP_ARGTYPE1(AT, _AT, _ET);
        } break;
        case FN_GetElementRef:
        case FN_GetElementPtr: {
            CHECKARGS(2, -1);
            const Type *T;
            bool is_ref = (b.value() == FN_GetElementRef);
            TypedValueRef dep;
            if (is_ref) {
                READ_NODEREF_TYPEOF(argT);
                T = SCOPES_GET_RESULT(ref_to_ptr(argT));
                dep = _argT;
            } else {
                READ_STORAGETYPEOF(argT);
                T = argT;
                dep = _argT;
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            auto pi = cast<PointerType>(T);
            T = pi->element_type;
            if (!is_ref) {
                // first argument is pointer offset
                // not applicable to references
                READ_STORAGETYPEOF(arg);
                SCOPES_CHECK_RESULT(verify_integer(arg));
            }
            while (argn < argcount) {
                const Type *ST = SCOPES_GET_RESULT(storage_type(T));
                switch(ST->kind()) {
                case TK_Array: {
                    auto ai = cast<ArrayType>(ST);
                    T = ai->element_type;
                    READ_STORAGETYPEOF(arg);
                    SCOPES_CHECK_RESULT(verify_integer(arg));
                } break;
                case TK_Tuple: {
                    auto ti = cast<TupleType>(ST);
                    READ_INT_CONST(arg);
                    SCOPES_CHECK_RESULT(sanitize_tuple_index(call.anchor(), ST, ti, arg, _arg));
                    T = SCOPES_GET_RESULT(ti->type_at_index(arg));
                } break;
                default: {
                    SCOPES_ERROR(InvalidArgumentTypeForBuiltin, b, T);
                } break;
                }
            }
            T = pointer_type(T, pi->flags, pi->storage_class);
            if (is_ref) {
                T = SCOPES_GET_RESULT(ptr_to_ref(T));
            }
            return DEP_ARGTYPE1(T, dep);
        } break;
        case FN_Deref: {
            CHECKARGS(1, 1);
            READ_TYPEOF(T);
            (void)T;
            return _T;
        } break;
        case FN_Assign: {
            CHECKARGS(2, 2);
            MOVE_STORAGETYPEOF(ElemT);
            READ_NODEREF_STORAGETYPEOF(DestT);
            {
                SCOPES_TRACE_PROVE_ARG(_DestT);
                auto rq = SCOPES_GET_RESULT(verify_refer(typeof_DestT));
                if (rq) {
                    SCOPES_CHECK_RESULT(verify_writable(rq, typeof_DestT));
                }
            }
            typeof_DestT = strip_qualifier<ReferQualifier>(typeof_DestT);
            //strip_qualifiers(ElemT);
            //strip_qualifiers(DestT);

            SCOPES_CHECK_RESULT(verify(ElemT, DestT));

            if (!is_plain(typeof_ElemT)) {
                auto uq = try_unique(typeof_ElemT);
                if (!uq) {
                    SCOPES_ERROR(UniqueValueExpected, _ElemT->get_type());
                }
                ctx.move(uq->id, _ElemT);
            }
            return ARGTYPE0();
        } break;
        case FN_PtrToRef: {
            CHECKARGS(1, 1);
            READ_TYPEOF(T);
            auto NT = SCOPES_GET_RESULT(ptr_to_ref(T));
            if (is_plain(T) && _T.isa<Pure>()) {
                return TypedValueRef(
                    ref(call.anchor(), PureCast::from(NT, _T.cast<Pure>())));
            } else {
                if (!is_unique(T) && !is_view(T)) {
                    return NEW_ARGTYPE1(NT);
                } else {
                    return DEP_ARGTYPE1(NT, _T);
                }
            }
        } break;
        case FN_RefToPtr: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(T);
            auto NT = SCOPES_GET_RESULT(ref_to_ptr(T));
            if (is_plain(T) && _T.isa<Pure>()) {
                return TypedValueRef(
                    ref(call.anchor(), PureCast::from(NT, _T.cast<Pure>())));
            } else {
                return DEP_ARGTYPE1(NT, _T);
            }
        } break;
        case FN_VolatileLoad:
        case FN_Load: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(T);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            SCOPES_CHECK_RESULT(verify_readable(T));
            return DEP_ARGTYPE1(cast<PointerType>(T)->element_type, _T);
        } break;
        case FN_VolatileStore:
        case FN_Store: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ElemT);
            READ_STORAGETYPEOF(DestT);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(DestT));
            SCOPES_CHECK_RESULT(verify_writable(DestT));
            auto pi = cast<PointerType>(DestT);
            SCOPES_CHECK_RESULT(
                verify(SCOPES_GET_RESULT(storage_type(pi->element_type)), ElemT));
            if (!is_plain(typeof_DestT)) {
                auto uq = try_unique(typeof_ElemT);
                if (!uq) {
                    SCOPES_ERROR(UniqueValueExpected, _ElemT->get_type());
                }
                ctx.move(uq->id, _ElemT);
            }
            return ARGTYPE0();
        } break;
        case FN_Alloca: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return ARGTYPE1(local_pointer_type(T));
        } break;
        case FN_AllocaArray: {
            CHECKARGS(2, 2);
            READ_TYPE_CONST(T);
            READ_STORAGETYPEOF(size);
            SCOPES_CHECK_RESULT(verify_integer(size));
            return ARGTYPE1(local_pointer_type(T));
        } break;
        case FN_Malloc: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return ARGTYPE1(native_pointer_type(T));
        } break;
        case FN_MallocArray: {
            CHECKARGS(2, 2);
            READ_TYPE_CONST(T);
            READ_STORAGETYPEOF(size);
            SCOPES_CHECK_RESULT(verify_integer(size));
            return ARGTYPE1(native_pointer_type(T));
        } break;
        case FN_Free: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(T);
            SCOPES_CHECK_RESULT(verify_writable(T));
            if (cast<PointerType>(T)->storage_class != SYM_Unnamed) {
                SCOPES_ERROR(InvalidArgumentTypeForBuiltin, b, T);
            }
            if (!is_plain(T)) {
                auto uq = try_unique(T);
                if (!uq) {
                    SCOPES_ERROR(UniqueValueExpected, _T->get_type());
                }
                SCOPES_CHECK_RESULT(build_drop(ctx, call.anchor(), _T));
                ctx.move(uq->id, _T);
            }
            return ARGTYPE0();
        } break;
        case FN_StaticAlloc: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            void *dst = tracked_malloc(SCOPES_GET_RESULT(size_of(T)));
            return TypedValueRef(call.anchor(),
                ConstPointer::from(static_pointer_type(T), dst));
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
            return DEP_ARGTYPE1(SCOPES_GET_RESULT(bool_op_return_type(A)), _A, _B);
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
            return DEP_ARGTYPE1(SCOPES_GET_RESULT(bool_op_return_type(A)), _A, _B);
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
        case OP_ ## NAME: \
        case OP_ ## NAME ## NUW: \
        case OP_ ## NAME ## NSW: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            return DEP_ARGTYPE1(A, _A, _B); \
        } break;
#define IARITH_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            return DEP_ARGTYPE1(A, _A, _B); \
        } break;
#define FARITH_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B)); \
            return DEP_ARGTYPE1(A, _A, _B); \
        } break;
#define FTRI_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(3, 3); \
            READ_TYPEOF(A); READ_TYPEOF(B); READ_TYPEOF(C); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B, C)); \
            return DEP_ARGTYPE1(A, _A, _B, _C); \
        } break;
#define IUN_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A)); \
            return DEP_ARGTYPE1(A, _A); \
        } break;
#define FUN_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_real_ops(A)); \
            return DEP_ARGTYPE1(A, _A); \
        } break;
        SCOPES_ARITH_OPS()

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP
        default: {
            SCOPES_TRACE_PROVE_ARG(callee);

            SCOPES_ERROR(UnsupportedBuiltin, b);
        } break;
        }

        return ref(call.anchor(), ArgumentList::from({}));
    }
    if (!is_function_pointer(T)) {
        SCOPES_ERROR(InvalidCallee, callee->get_type());
    }
    const FunctionType *aft = extract_function_type(T);
    const FunctionType *ft = aft->strip_annotations();
    int numargs = (int)ft->argument_types.size();
    if ((!ft->vararg() && (values.size() != numargs))
        || (ft->vararg() && (values.size() < numargs))) {
        if (values.size() > numargs) {
            SCOPES_ERROR(TooManyFunctionArguments, ft, values.size());
        } else if (values.size() < numargs) {
            SCOPES_ERROR(NotEnoughFunctionArguments, ft, values.size());
        } else {
            assert(false && "how did we get here?");
        }
    }
    // verify_function_argument_signature
    for (int i = 0; i < numargs; ++i) {
        SCOPES_TRACE_PROVE_ARG(values[i]);
        const Type *Ta = strip_qualifiers(values[i]->get_type(),
            (1 << QK_Key));
        const Type *Tb = ft->argument_types[i];
        if (is_reference(Ta) && !is_reference(Tb)) {
            SCOPES_CHECK_RESULT(build_deref(ctx, call.anchor(), values[i]));
            Ta = values[i]->get_type();
        }
        if (is_view(Tb)) {
            if (!is_view(Ta)) {
                build_view(ctx, call.anchor(), values[i]);
                Ta = values[i]->get_type();
            }
            Tb = strip_view(Tb);
            Ta = strip_view(Ta);
        } else if (is_unique(Tb)) {
            if (!is_view(Ta)) {
                assert(is_unique(Ta));
                Tb = strip_unique(Tb);
                Ta = strip_unique(Ta);
            }
        } else if (is_plain(Tb)) {
            //Tb = strip_lifetime(Tb);
            Ta = strip_lifetime(Ta);
        }
        if (SCOPES_GET_RESULT(types_compatible(Tb, Ta))) {
            continue;
        }
        SCOPES_ERROR(ParameterTypeMismatch, Tb, Ta);
    }
    // build id map
    ID2SetMap idmap;
    idmap.reserve(numargs);
    // first map uniques
    for (int i = 0; i < numargs; ++i) {
        const Type *paramT = ft->argument_types[i];
        if (is_unique(paramT)) {
            const Type *argT = values[i]->get_type();
            auto paramu = get_unique(paramT);
            auto argu = get_unique(argT);
            // argument will be moved into the function
            ctx.move(argu->id, values[i]);
            map_unique_id(idmap, paramu->id, argu->id);
        }
    }
    // then map views
    for (int i = 0; i < numargs; ++i) {
        const Type *paramT = ft->argument_types[i];
        if (is_view(paramT)) {
            const Type *argT = values[i]->get_type();
            auto paramv = get_view(paramT);
            auto argv = get_view(argT);
            for (auto id : paramv->sorted_ids) {
                auto it = idmap.find(id);
                if (it != idmap.end())
                    continue;
                assert(!argv->ids.empty());
                // unseen view id, map to argument ids
                for (auto vid : argv->ids) {
                    map_unique_id(idmap, id, vid);
                }
            }
        }
    }
    //const Type *art = aft->return_type;
    const Type *rt = remap_unique_return_arguments(ctx, idmap, ft->return_type);
    CallRef newcall = ref(call.anchor(), Call::from(rt, callee, values));
    //newcall->set_def_anchor(call->def_anchor());
    if (ft->has_exception()) {
        // todo: remap exception type
        newcall->except_body.set_parent(ctx.block);
        auto exceptctx = ctx.with_block(newcall->except_body);
        const Type *et = remap_unique_return_arguments(exceptctx, idmap, ft->except_type);
        auto exc = ref(call.anchor(), Exception::from(et));

        map_arguments_to_block(exceptctx, exc);
        newcall->except = exc;

        SCOPES_CHECK_RESULT(make_raise(exceptctx, call.anchor(), exc));
    }

    #if 1
    // hack: rewrite valuerefs returned by globals matching sc_*_new
    //       to use the anchor of the calling expression
    //
    //       this could also be performed by wrapper macros inside the language
    if (rt == TYPE_ValueRef) {
        if (callee.isa<Global>()) {
            auto g = callee.cast<Global>();
            const char *name = g->name.name()->data;
            auto sz = g->name.name()->count;
            if (sz >= 7) {
                if (name[0] == 's' && name[1] == 'c' && name[2] == '_'
                    && name[sz-4] == '_' && name[sz-3] == 'n' && name[sz-2] == 'e' && name[sz-1] == 'w') {
                    ctx.append(newcall);
                    auto anchor = call.anchor();
                    // convert to valueref
                    newcall = ref(anchor, Call::from(TYPE_ValueRef,
                        g_sc_valueref_tag, {
                        ref(anchor, ConstPointer::anchor_from(anchor)),
                        newcall
                    }));
                }
            }
        }
    }
    #endif
    return TypedValueRef(newcall);
}

static LabelRef make_merge_label(const ASTContext &ctx, const Anchor *anchor) {
    LabelRef merge_label = ref(anchor, Label::from(LK_BranchMerge));
    merge_label->body.set_parent(ctx.block);
    return merge_label;
}

static SCOPES_RESULT(TypedValueRef) finalize_merge_label(const ASTContext &ctx,
    const LabelRef &merge_label, const char *by) {
    SCOPES_RESULT_TYPE(TypedValueRef);

    auto _void = empty_arguments_type();
    const Type *rtype = nullptr;
    for (auto merge : merge_label->merges) {
        if (merge->values.empty()) {
            rtype = _void;
            break;
        }
    }
    if (rtype == _void) {
        // ensure all merges return nothing
        for (auto merge : merge_label->merges) {
            merge->values.clear();
        }
    }

    IDSet valid;
    SCOPES_CHECK_RESULT(finalize_merges(ctx, merge_label, valid, by));
    if (!rtype) {
        for (auto merge : merge_label->merges) {
            rtype = SCOPES_GET_RESULT(merge_value_type(by, rtype,
                arguments_type_from_typed_values(merge->values)));
        }
        rtype = ctx.fix_merge_type(rtype);
    }
    merge_label->change_type(rtype);
    merge_back_valid(ctx, valid, merge_label);
    ctx.append(merge_label);

    return TypedValueRef(merge_label);
}

static SCOPES_RESULT(TypedValueRef) prove_SwitchTemplate(const ASTContext &ctx,
    const SwitchTemplateRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);

    auto newexpr = SCOPES_GET_RESULT(prove(ctx, node->expr));
    newexpr = ref(newexpr.anchor(), ExtractArgument::from(newexpr, 0));
    SCOPES_CHECK_RESULT(build_deref(ctx, newexpr.anchor(), newexpr));

    const Type *casetype = newexpr->get_type();

    auto _switch = ref(node.anchor(), Switch::from(newexpr));

    LabelRef merge_label = make_merge_label(ctx, node.anchor());

    ASTContext subctx = ctx.with_block(merge_label->body);
    subctx.append(_switch);

    Switch::Case *defaultcase = nullptr;
    for (auto &&_case : node->cases) {
        Switch::Case *newcase = nullptr;
        if (_case.kind == CK_Default) {
            if (defaultcase) {
                SCOPES_ERROR(DuplicateSwitchDefaultCase);
            }
            newcase = &_switch->append_default();
            defaultcase = newcase;
        } else {
            auto newlit = SCOPES_GET_RESULT(prove(ctx, _case.literal));
            if (!newlit.isa<ConstInt>()) {
                SCOPES_ERROR(ValueKindMismatch, VK_ConstInt, newlit->kind());
            }
            casetype = SCOPES_GET_RESULT(
                merge_value_type("switch case literal", casetype, newlit->get_type()));
            newcase = &_switch->append_pass(newlit.cast<ConstInt>());
        }
        assert(_case.value);
        ASTContext newctx;
        auto newvalue = SCOPES_GET_RESULT(prove_block(subctx, newcase->body, _case.value, newctx));
        if (_case.kind == CK_Pass) {
            SCOPES_CHECK_RESULT(validate_pass_block(subctx, newcase->body));
        } else {
            if (is_returning(newvalue->get_type())) {
                make_merge(newctx, newvalue.anchor(), merge_label, newvalue);
            }
        }
    }

    if (!defaultcase) {
        SCOPES_ERROR(MissingDefaultCase);
    }

    if (merge_label->merges.empty()) {
        // none of the paths are returning
        // cases do not need a merge label
        assert(ctx.block);
        ctx.merge_block(merge_label->body);
        return TypedValueRef(_switch);
    }

    return finalize_merge_label(ctx, merge_label, "switch case");
}

static SCOPES_RESULT(TypedValueRef) prove_If(const ASTContext &ctx, const IfRef &_if) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    assert(!_if->clauses.empty());
    int numclauses = _if->clauses.size();
    assert(numclauses >= 1);
    CondBrRef first_condbr;
    CondBrRef last_condbr;
    ASTContext subctx(ctx);
    LabelRef merge_label = make_merge_label(ctx, _if.anchor());
    for (int i = 0; i < numclauses; ++i) {
        auto &&clause = _if->clauses[i];
        //assert(clause.anchor);
        //SCOPES_ANCHOR(clause.anchor);
        if (clause.is_then()) {
            TypedValueRef newcond = SCOPES_GET_RESULT(prove(subctx, clause.cond));
            newcond = ref(newcond.anchor(),
                ExtractArgument::from(newcond, 0));
            SCOPES_CHECK_RESULT(build_deref(ctx, newcond.anchor(), newcond));
            auto condT = strip_qualifiers(newcond->get_type());
            if (condT != TYPE_Bool) {
                SCOPES_ERROR(ConditionNotBool, newcond->get_type());
            }
            CondBrRef condbr = ref(newcond.anchor(), CondBr::from(newcond));
            if (!first_condbr) {
                first_condbr = condbr;
            }
            condbr->then_body.set_parent(&merge_label->body);
            condbr->else_body.set_parent(&merge_label->body);
            auto thenctx = subctx.with_block(condbr->then_body);
            auto thenresult = SCOPES_GET_RESULT(prove(thenctx, clause.value));
            if (is_returning(thenresult->get_type())) {
                make_merge(thenctx, thenresult.anchor(), merge_label, thenresult);
            }
            if (last_condbr) {
                last_condbr->else_body.append(condbr);

            } else {
                merge_label->body.append(condbr);
            }
            last_condbr = condbr;
            subctx = subctx.with_block(condbr->else_body);
        } else {
            assert(last_condbr);
            auto elseresult = SCOPES_GET_RESULT(prove(subctx, clause.value));
            if (is_returning(elseresult->get_type())) {
                ASTContext elsectx = ctx.with_block(last_condbr->else_body);
                make_merge(elsectx, _if.anchor(), merge_label, elseresult);
            }
            last_condbr = CondBrRef();
        }
    }
    if (last_condbr) {
        // last else value missing
        ASTContext elsectx = ctx.with_block(last_condbr->else_body);
        make_merge(elsectx, _if.anchor(), merge_label,
            ref(_if.anchor(), ArgumentList::from({})));
    }

    if (merge_label->merges.empty()) {
        // none of the paths are returning
        // conditions do not need a merge label
        assert(ctx.block);
        ctx.merge_block(merge_label->body);
        return TypedValueRef(first_condbr);
    }

    return finalize_merge_label(ctx, merge_label, "branch");
}

static SCOPES_RESULT(TypedValueRef) prove_Template(const ASTContext &ctx, const TemplateRef &_template) {
    FunctionRef frame = ctx.frame;
    assert(frame);
    return TypedValueRef(_template.anchor(), ConstPointer::closure_from(
        Closure::from(_template, frame)));
}

static SCOPES_RESULT(TypedValueRef) prove_Quote(const ASTContext &ctx, const QuoteRef &node) {
    //StyledStream ss;
    //ss << "before quote" << std::endl;
    //stream_ast(ss, node, StreamASTFormat());
    return quote(ctx, node->value);
    //ss << "after quote" << std::endl;
    //stream_ast(ss, value, StreamASTFormat());
}

static SCOPES_RESULT(TypedValueRef) prove_Unquote(const ASTContext &ctx, const UnquoteRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    SCOPES_ERROR(UnexpectedValueKind, node->kind());
}

SCOPES_RESULT(TypedValueRef) prove(const ASTContext &ctx, const ValueRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    assert(node);
    TypedValueRef result = SCOPES_GET_RESULT(ctx.frame->resolve(node, ctx.function));
    if (!result) {
        if (node.isa<TypedValue>()) {
            result = node.cast<TypedValue>();
        } else {
            // we shouldn't set an anchor here because sometimes the parent context
            // is more indicative than the node position
            //SCOPES_CHECK_RESULT(verify_stack());
            switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
            case NAME: result = SCOPES_GET_RESULT(prove_ ## CLASS(ctx, node.cast<CLASS>())); break;
            SCOPES_UNTYPED_VALUE_KIND()
#undef T
            default: assert(false);
            }
            assert(result);
            ctx.frame->bind(node, result);
            assert (ctx.block);
            #if 0
            if (!result->is_typed()) {
                StyledStream ss;
                stream_ast(ss, result, StreamASTFormat());
            }
            #endif
            ctx.append(result);

        }
    }
    return result;
}

// used by inlined functions
static SCOPES_RESULT(void) prove_inline_arguments(const ASTContext &ctx,
    const ParameterTemplates &params, const TypedValues &tmpargs) {
    SCOPES_RESULT_TYPE(void);
    int count = (int)params.size();
    for (int i = 0; i < count; ++i) {
        auto oldsym = params[i];
        TypedValueRef newval;
        if (oldsym->is_variadic()) {
            if ((i + 1) < count) {
                SCOPES_ERROR(VariadicParameterNotLast);
            }
            if ((i + 1) == (int)tmpargs.size()) {
                newval = tmpargs[i];
            } else {
                TypedValues args;
                for (int j = i; j < tmpargs.size(); ++j) {
                    args.push_back(tmpargs[j]);
                }
                newval = ref(oldsym.anchor(), ArgumentList::from(args));
            }
        } else if (i < tmpargs.size()) {
            newval = tmpargs[i];
        } else {
            newval = ref(oldsym.anchor(),
                ConstAggregate::none_from());
        }
        ctx.frame->bind(oldsym, newval);
    }
    return {};
}

static SCOPES_RESULT(TypedValueRef) prove_inline_body(const ASTContext &ctx,
    const Closure *cl, const TypedValues &nodes) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    auto frame = cl->frame;
    auto func = cl->func;
    SCOPES_TRACE_PROVE_TEMPLATE(func);
    Timer sum_prove_time(TIMER_Specialize);
    assert(func);
    auto anchor = func.anchor();
    //int count = (int)func->params.size();
    FunctionRef fn = ref(anchor, Function::from(func->name, {}));
    //fn->set_def_anchor(anchor);
    fn->original = ref(anchor, func);
    fn->frame = ref(frame.anchor(), frame);
    LabelRef label = ref(anchor, Label::from(LK_Inline, func->name));
    fn->label = label;
    fn->boundary = ctx.function;

    // inlines may escape caller loops
    ASTContext subctx = ctx.with_frame(fn);
    SCOPES_CHECK_RESULT(prove_inline_arguments(subctx, func->params, nodes));
    ASTContext bodyctx;
    TypedValueRef result_value = SCOPES_GET_RESULT(
        prove_block(subctx, label->body, func->value, bodyctx));
    if (label->merges.empty()) {
        // label does not need a merge label
        assert(ctx.block);
        result_value = SCOPES_GET_RESULT(
            move_single_merge_value(bodyctx, ctx.block->depth, result_value, "inline return"));
        ctx.merge_block(label->body);
        return result_value;
    } else {
        if (is_returning(result_value->get_type())) {
            make_merge(bodyctx, result_value.anchor(), label, result_value);
        }
        IDSet valid;
        SCOPES_CHECK_RESULT(finalize_merges(ctx, label, valid, "inline return"));
        const Type *rtype = nullptr;
        for (auto merge : label->merges) {
            rtype = SCOPES_GET_RESULT(merge_value_type("inline return merge",
                rtype,
                arguments_type_from_typed_values(merge->values)));
        }
        rtype = ctx.fix_merge_type(rtype);
        label->change_type(rtype);
        merge_back_valid(ctx, valid, label);
        ctx.append(label);
        return TypedValueRef(label);
    }
}

SCOPES_RESULT(TypedValueRef) prove_inline(const ASTContext &ctx,
    const Closure *cl, const TypedValues &nodes) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    auto func = cl->func;
    if (func->recursion >= SCOPES_MAX_RECURSIONS) {
        SCOPES_ERROR(RecursionOverflow, func->recursion);
    }
    func->recursion++;
    auto result = prove_inline_body(ctx, cl, nodes);
    func->recursion--;
    return result;
}

static SCOPES_RESULT(FunctionRef) prove_body(
    const FunctionRef &frame, const TemplateRef &func, Types types) {
    SCOPES_RESULT_TYPE(FunctionRef);
    Timer sum_prove_time(TIMER_Specialize);
    assert(func);
    canonicalize_argument_types(types);
    Function key(func->name, {});
    key.original = func;
    key.frame = frame;
    key.instance_args = types;
    auto it = functions.find(&key);
    if (it != functions.end())
        return ref(func.anchor(), *it);
    SCOPES_TRACE_PROVE_TEMPLATE(func);
    int count = (int)func->params.size();
    FunctionRef fn = ref(func.anchor(), Function::from(func->name, {}));
    fn->original = func;
    fn->frame = frame;
    fn->instance_args = types;
    fn->boundary = fn;
    for (int i = 0; i < count; ++i) {
        auto oldparam = func->params[i];
        if (oldparam->is_variadic()) {
            if ((i + 1) < count) {
                SCOPES_ERROR(VariadicParameterNotLast);
            }
            if ((i + 1) == (int)types.size()) {
                auto newparam = ref(oldparam.anchor(),
                    Parameter::from(oldparam->name, types[i]));
                fn->append_param(newparam);
                fn->bind(oldparam, newparam);
            } else {
                Types vtypes;
                TypedValues args;
                for (int j = i; j < types.size(); ++j) {
                    vtypes.push_back(types[j]);
                    auto newparam = ref(oldparam.anchor(),
                        Parameter::from(oldparam->name, types[j]));
                    fn->append_param(newparam);
                    args.push_back(newparam);
                }
                fn->bind(oldparam, ref(oldparam.anchor(),
                    ArgumentList::from(args)));
            }
        } else {
            const Type *T = TYPE_Nothing;
            if (i < types.size()) {
                T = types[i];
            }
            #if 0
            if (oldparam->is_typed()) {
                SCOPES_ANCHOR(oldparam->anchor());
                SCOPES_CHECK_RESULT(verify(oldparam->get_type(), T));
            }
            #endif
            auto newparam = ref(oldparam.anchor(),
                Parameter::from(oldparam->name, T));
            fn->append_param(newparam);
            fn->bind(oldparam, newparam);
        }
    }
    fn->build_valids();
    functions.insert(fn.unref());

    ASTContext fnctx = ASTContext::from_function(fn);
    ASTContext bodyctx = fnctx.with_block(fn->body);
    fn->body.valid = fn->valid;
    auto expr = SCOPES_GET_RESULT(prove(bodyctx, func->value));
    SCOPES_CHECK_RESULT(make_return(bodyctx, expr.anchor(), expr));
    SCOPES_CHECK_RESULT(ensure_function_type(fn));
    SCOPES_CHECK_RESULT(finalize_returns_raises(bodyctx));
    //SCOPES_CHECK_RESULT(track(fnctx));
    fn->complete = true;
    return fn;
}

SCOPES_RESULT(FunctionRef) prove(const FunctionRef &frame, const TemplateRef &func, const Types &types) {
    SCOPES_RESULT_TYPE(FunctionRef);
    if (func->recursion >= SCOPES_MAX_RECURSIONS) {
        SCOPES_ERROR(RecursionOverflow, func->recursion);
    }
    func->recursion++;
    auto result = prove_body(frame, func, types);
    func->recursion--;
    return result;
}

SCOPES_RESULT(TypedValueRef) prove(const ValueRef &node) {
    SCOPES_RESULT_TYPE(TypedValueRef);
    if (!ast_context) {
        SCOPES_ERROR(ProveOutsideOfMacro);
    }
    return prove(*ast_context, node);
}

//------------------------------------------------------------------------------

} // namespace scopes
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
#include "closure.hpp"
#include "stream_ast.hpp"
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
#include "scopes/scopes.h"
#include "qualifier.inc"
#include "tracker.hpp"

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

static SCOPES_RESULT(const ReferQualifier *) verify_refer(const Type *T) {
    SCOPES_RESULT_TYPE(const ReferQualifier *);
    auto rq = try_qualifier<ReferQualifier>(T);
    if (!rq) {
        StyledString ss;
        ss.out << "value of type " << T << " must be reference";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return rq;
}

static SCOPES_RESULT(void) verify_readable(const ReferQualifier *Q, const Type *T) {
    SCOPES_RESULT_TYPE(void);
    if (!pointer_flags_is_readable(Q->flags)) {
        StyledString ss;
        ss.out << "can not dereference value of type " << T
            << " because the reference is non-readable";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return {};
}

static SCOPES_RESULT(void) verify_writable(const ReferQualifier *Q, const Type *T) {
    SCOPES_RESULT_TYPE(void);
    if (!pointer_flags_is_writable(Q->flags)) {
        StyledString ss;
        ss.out << "can not assign to value of type " << T
            << " because the reference is non-writable";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return {};
}

static SCOPES_RESULT(void) verify_readable(const Type *T) {
    SCOPES_RESULT_TYPE(void);
    auto pi = cast<PointerType>(T);
    if (!pi->is_readable()) {
        StyledString ss;
        ss.out << "can not load value from address of type " << T
            << " because the target is non-readable";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return {};
}

static SCOPES_RESULT(void) verify_writable(const Type *T) {
    SCOPES_RESULT_TYPE(void);
    auto pi = cast<PointerType>(T);
    if (!pi->is_writable()) {
        StyledString ss;
        ss.out << "can not store value at address of type " << T
            << " because the target is non-writable";
        SCOPES_LOCATION_ERROR(ss.str());
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

ASTContext ASTContext::for_loop(LoopLabel *loop) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext ASTContext::for_break(Label *xbreak) const {
    return ASTContext(function, frame, loop, except, xbreak, block);
}

ASTContext ASTContext::for_try(Label *except) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext ASTContext::with_block(Block &_block) const {
    return ASTContext(function, frame, loop, except, _break, &_block);
}

ASTContext ASTContext::with_frame(Function *frame) const {
    return ASTContext(function, frame, loop, except, _break, block);
}

ASTContext::ASTContext() {}

ASTContext::ASTContext(Function *_function, Function *_frame,
    LoopLabel *_loop, Label *_except, Label *xbreak, Block *_block) :
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
        auto uq = try_unique(argT);
        if (uq) {
            argT = unique_type(argT, unique_id());
        }
        newtypes.push_back(argT);
    }
    return arguments_type(newtypes);
}

int ASTContext::unique_id() const {
    return function->unique_id();
}

void ASTContext::move(int id) const {
    block->move(id);
}

void ASTContext::merge_block(Block &_block) const {
    block->migrate_from(_block);
}

void ASTContext::append(TypedValue *value) const {
    assert(block);
    if (block->append(value)) {
        function->try_bind_unique(value);
    }
}

ASTContext ASTContext::from_function(Function *fn) {
    return ASTContext(fn, fn, nullptr, nullptr, nullptr, nullptr);
}

//------------------------------------------------------------------------------

static SCOPES_RESULT(TypedValue *) prove_inline(const ASTContext &ctx, const Closure *cl, const TypedValues &nodes);

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
                SCOPES_EXPECT_ERROR(error_cannot_merge_expression_types(context, T1, T2));
            }
            newargs.push_back(T);
        }
        return arguments_type(newargs);
    }
    SCOPES_EXPECT_ERROR(error_cannot_merge_expression_types(context, T1, T2));
}

SCOPES_RESULT(TypedValue *) prove_block(const ASTContext &ctx, Block &block, Value *node, ASTContext &newctx) {
    SCOPES_RESULT_TYPE(TypedValue *);
    block.set_parent(ctx.block);
    newctx = ctx.with_block(block);
    auto result = SCOPES_GET_RESULT(prove(newctx, node));
    return result;
}

static bool split_return_values(TypedValues &values, TypedValue *value) {
    auto T = value->get_type();
    if (!is_returning(T)) return false;
    auto count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        values.push_back(ExtractArgument::from(value->anchor(), value, i));
    }
    return true;
}

void map_arguments_to_block(const ASTContext &ctx, TypedValue *src) {
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
    values.insert(values.begin(), ConstPointer::string_from(anchor, msg));
    auto expr =
        CallTemplate::from(anchor,
            ConstInt::builtin_from(anchor, Builtin(FN_Annotate)),
                values);
    auto result = prove(ctx, expr);
    if (!result.ok()) {
        print_error(result.assert_error());
        assert(false && "error while annotating");
    }
}

static SCOPES_RESULT(void) drop_value(const ASTContext &ctx,
    const Anchor *anchor, const ValueIndex &arg) {
    SCOPES_RESULT_TYPE(void);
    // generate destructor
    auto argT = arg.get_type();
    int id = get_unique(argT)->id;
    argT = strip_qualifiers(argT);
    Value *handler;
    if (!argT->lookup(SYM_DropHandler, handler)) {
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "forget";
        write_annotation(ctx, anchor, ss.str(), {
            ConstPointer::type_from(anchor, arg.get_type()) });
        #endif
    } else {
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "destruct";
        write_annotation(ctx, anchor, ss.str(), {
            ConstPointer::type_from(anchor, arg.get_type()) });
        #endif
        auto expr =
            CallTemplate::from(anchor, handler, {
                ExtractArgument::from(anchor, arg.value, arg.index) });
        auto result = SCOPES_GET_RESULT(prove(ctx, expr));
        auto RT = result->get_type();
        if (!is_returning(RT) || is_returning_value(RT)) {
            SCOPES_LOCATION_ERROR(String::from("drop operation must evaluate to void"));
        }
    }
    assert(ctx.block);
    ctx.move(id);
    return {};
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, TypedValue *val, const char *by) {
    SCOPES_RESULT_TYPE(void);
    if (!ctx.block->is_valid(val)) {
        SCOPES_EXPECT_ERROR(error_cannot_access_moved(val, by));
    }
    return {};
}

static SCOPES_RESULT(void) verify_valid(const ASTContext &ctx, const TypedValues &values, const char *by) {
    SCOPES_RESULT_TYPE(void);
    for (auto &&value : values) {
        SCOPES_CHECK_RESULT(verify_valid(ctx, value, by));
    }
    return {};
}

static void build_view(
    const ASTContext &ctx, const Anchor *anchor, TypedValue *&val) {
    auto T = val->get_type();
    auto uq = try_unique(T);
    if (uq) {
        assert(ctx.block->is_valid(val));
        auto retT = view_type(T, {});
        auto call = Call::from(anchor, retT, g_view, { val });
        ctx.append(call);
        val = call;
    }
}

static void build_move(
    const ASTContext &ctx, const Anchor *anchor, TypedValue *&val) {
    assert(ctx.block->is_valid(val));
    auto T = val->get_type();
    auto uq = get_unique(T);
    auto retT = unique_type(T, ctx.unique_id());
    auto call = Call::from(anchor, retT, g_move, { val });
    ctx.append(call);
    ctx.move(uq->id);
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
        SCOPES_EXPECT_ERROR(error_altering_parent_scope_in_pass(info.value.get_type()));
    }
    return {};
}

static void merge_back_valid(const ASTContext &ctx, IDSet &valid) {
    IDSet deleted = difference_idset(ctx.block->valid, valid);
    for (auto id : deleted) {
        ctx.move(id);
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << "merge-forgetting " << id;
        write_annotation(ctx, get_active_anchor(), ss.str(), {});
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
                SCOPES_EXPECT_ERROR(error_cannot_return_view(value));
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
    SCOPES_ANCHOR(anchor);
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
                    SCOPES_EXPECT_ERROR(error_cannot_return_view(value));
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

static SCOPES_RESULT(TypedValue *) move_single_merge_value(const ASTContext &ctx,
    int retdepth, TypedValue *result, const char *by) {
    SCOPES_RESULT_TYPE(TypedValue *);
    TypedValues values;
    if (is_returning_value(result->get_type())
        && split_return_values(values, result)) {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, result->anchor(),
            retdepth, values, by));
        result = ArgumentList::from(result->anchor(), values);
    } else {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, result->anchor(),
            retdepth, values, by));
    }
    return result;
}

// must be called before the return type is computed
// don't forget to call merge_back_invalid(...) when the label has been added
SCOPES_RESULT(void) finalize_merges(const ASTContext &ctx, Label *label, IDSet &valid, const char *by) {
    SCOPES_RESULT_TYPE(void);
    valid = ctx.block->valid;
    // patch merges
    int retdepth = label->body.depth - 1;
    for (auto merge : label->merges) {
        auto mctx = ctx.with_block(*merge->block);
        assert(merge->block);
        SCOPES_CHECK_RESULT(move_merge_values(mctx,
            merge->anchor(), retdepth, merge->values, by));
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
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge->anchor(), drop_ids));
    }
    return {};
}

SCOPES_RESULT(void) finalize_repeats(const ASTContext &ctx, LoopLabel *label, const char *by) {
    SCOPES_RESULT_TYPE(void);
    IDSet valid = ctx.block->valid;
    // patch repeats
    int retdepth = label->body.depth - 1;
    for (auto merge : label->repeats) {
        auto mctx = ctx.with_block(*merge->block);
        assert(merge->block);
        SCOPES_CHECK_RESULT(move_merge_values(mctx,
            merge->anchor(), retdepth, merge->values, by));
        collect_valid_values(mctx, valid);
    }
    IDSet deleted = difference_idset(ctx.block->valid, valid);
    if (!deleted.empty()) {
        // parent values were deleted, which we can't repeat
        int id = *deleted.begin();
        auto info = ctx.function->get_unique_info(id);
        SCOPES_EXPECT_ERROR(error_altering_parent_scope_in_loop(info.value.get_type()));
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
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge->anchor(), drop_ids));
    }
    for (auto merge : ctx.function->raises) {
        auto mctx = ctx.with_block(*merge->block);
        SCOPES_CHECK_RESULT(validate_merge_values(valid, merge->values));
        IDSet todrop = intersect_idset(deleted, merge->block->valid);
        IDs drop_ids;
        drop_ids.reserve(todrop.size());
        sort_drop_ids(todrop, drop_ids);
        SCOPES_CHECK_RESULT(merge_drop_values(mctx, merge->anchor(), drop_ids));
    }
    return {};
}

static TypedValue *make_merge1(const ASTContext &ctx, const Anchor *anchor, Label *label, const TypedValues &values) {
    assert(label);

    auto newmerge = Merge::from(anchor, label, values);

    ctx.append(newmerge);
    label->merges.push_back(newmerge);

    return newmerge;
}

static TypedValue *make_merge(const ASTContext &ctx, const Anchor *anchor, Label *label, TypedValue *value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_merge1(ctx, anchor, label, results);
    } else {
        return value;
    }
}

static TypedValue *make_repeat1(const ASTContext &ctx, const Anchor *anchor, LoopLabel *label, const TypedValues &values) {
    assert(label);

    auto newrepeat = Repeat::from(anchor, label, values);

    ctx.append(newrepeat);
    label->repeats.push_back(newrepeat);
    return newrepeat;
}

static TypedValue *make_repeat(const ASTContext &ctx, const Anchor *anchor, LoopLabel *label, TypedValue *value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_repeat1(ctx, anchor, label, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValue *) make_return1(const ASTContext &ctx, const Anchor *anchor, TypedValues values) {
    SCOPES_RESULT_TYPE(TypedValue *);
    assert(ctx.block);
    assert(ctx.function);
    SCOPES_CHECK_RESULT(move_merge_values(ctx, anchor, 0, values, "return"));
    collect_valid_function_values(ctx);

    auto newreturn = Return::from(anchor, values);

    ctx.append(newreturn);
    ctx.function->returns.push_back(newreturn);
    return newreturn;
}

static SCOPES_RESULT(TypedValue *) make_return(const ASTContext &ctx, const Anchor *anchor, TypedValue *value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_return1(ctx, anchor, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValue *) make_raise1(const ASTContext &ctx, const Anchor *anchor, TypedValues values) {
    SCOPES_RESULT_TYPE(TypedValue *);
    if (ctx.except) {
        return make_merge1(ctx,
            anchor, ctx.except, values);
    } else {
        SCOPES_CHECK_RESULT(move_merge_values(ctx, anchor, 0, values, "raise"));
        collect_valid_function_values(ctx);

        auto newraise = Raise::from(anchor, values);

        ctx.append(newraise);
        ctx.function->raises.push_back(newraise);
        return newraise;
    }
}

static SCOPES_RESULT(TypedValue *) make_raise(const ASTContext &ctx, const Anchor *anchor, TypedValue *value) {
    TypedValues results;
    if (split_return_values(results, value)) {
        return make_raise1(ctx, anchor, results);
    } else {
        return value;
    }
}

static SCOPES_RESULT(TypedValue *) prove_LabelTemplate(const ASTContext &ctx, LabelTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    Label *label = Label::from(node->anchor(), node->label_kind, node->name);
    assert(ctx.frame);
    assert(ctx.block);
    ctx.frame->bind(node, label);
    SCOPES_ANCHOR(node->anchor());
    TypedValue *result = nullptr;
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
        make_merge(labelctx, result->anchor(), label, result);
        #if 1
        if (label->body.empty()) {
            StyledStream ss;
            stream_ast(ss, label, StreamASTFormat());
            stream_ast(ss, node, StreamASTFormat());
            stream_ast(ss, result, StreamASTFormat());
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
        merge_back_valid(ctx, valid);
        ctx.append(label);
        return label;
    }
}

static SCOPES_RESULT(TypedValue *) prove_Expression(const ASTContext &ctx, Expression *expr) {
    SCOPES_RESULT_TYPE(TypedValue *);
    int count = (int)expr->body.size();
    if (expr->scoped) {
        Block block;
        block.set_parent(ctx.block);
        ASTContext subctx = ctx.with_block(block);
        for (int i = 0; i < count; ++i) {
            auto newsrc = SCOPES_GET_RESULT(prove(subctx, expr->body[i]));
            if (!is_returning(newsrc->get_type())) {
                SCOPES_ANCHOR(expr->body[i]->anchor());
                SCOPES_CHECK_RESULT(error_noreturn_not_last_expression());
            }
        }
        TypedValue *result = nullptr;
        if (expr->value) {
            result = SCOPES_GET_RESULT(prove(subctx, expr->value));
        } else {
            result = ArgumentList::from(expr->anchor(), {});
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
            return ArgumentList::from(expr->anchor(), {});
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

static std::vector<Symbol> find_closest_match(Symbol name, const Symbols &symbols) {
    const String *s = name.name();
    std::unordered_set<Symbol, Symbol::Hash> done;
    done.insert(SYM_Unnamed);
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    for (auto sym : symbols) {
        if (done.count(sym))
            continue;
        size_t dist = distance(s, sym.name());
        if (dist == best_dist) {
            best_syms.push_back(sym);
        } else if (dist < best_dist) {
            best_dist = dist;
            best_syms = { sym };
        }
        done.insert(sym);
    }
    std::sort(best_syms.begin(), best_syms.end());
    return best_syms;
}

static void print_name_suggestions(Symbol name, const Symbols &symbols, StyledStream &ss) {
    auto syms = find_closest_match(name, symbols);
    if (!syms.empty()) {
        ss << "Did you mean '" << syms[0].name()->data << "'";
        for (size_t i = 1; i < syms.size(); ++i) {
            if ((i + 1) == syms.size()) {
                ss << " or ";
            } else {
                ss << ", ";
            }
            ss << "'" << syms[i].name()->data << "'";
        }
        ss << "?";
    }
}

SCOPES_RESULT(void) map_keyed_arguments(const Anchor *anchor, TypedValue *callee,
    TypedValues &outargs, const TypedValues &values, const Symbols &symbols, bool varargs) {
    SCOPES_RESULT_TYPE(void);
    outargs.reserve(values.size());
    std::vector<bool> mapped;
    mapped.reserve(values.size());
    size_t next_index = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        TypedValue *arg = values[i];
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
                outargs.push_back(nullptr);
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
                    outargs.push_back(nullptr);
                }
                if (mapped[ki]) {
                    StyledString ss;
                    ss.out << "duplicate binding to parameter " << key;
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                index = ki;
                // strip key from value
                arg = Keyed::from(anchor, SYM_Unnamed, arg);
            } else if (varargs) {
                // no parameter with that name, but we accept varargs
                while (mapped.size() < symbols.size()) {
                    mapped.push_back(false);
                    outargs.push_back(nullptr);
                }
                index = (int)outargs.size();
                mapped.push_back(false);
                outargs.push_back(nullptr);
            } else {
                StyledString ss;
                ss.out << "no parameter named '" << key.name()->data << "'";
                const Type *T = callee->get_type();
                Template *func = nullptr;
                if (is_function_pointer(T)) {
                    ss.out << " in function of type " << T;
                } else if (T == TYPE_Closure) {
                    const Closure *cl = SCOPES_GET_RESULT((extract_closure_constant(callee)));
                    func = cl->func;
                    ss.out << " in untyped function ";
                    if (cl->func->name != SYM_Unnamed) {
                        ss.out << cl->func->name.name()->data;
                    }
                }
                ss.out << ". ";
                print_name_suggestions(key, symbols, ss.out);
                auto err = make_location_error(ss.str());
                if (func)
                    err->append_definition(func);
                SCOPES_RETURN_ERROR(err);
            }
        }
        mapped[index] = true;
        outargs[index] = arg;
    }
    TypedValue *noneval = nullptr;
    for (size_t i = 0; i < outargs.size(); ++i) {
        if (!outargs[i]) {
            if (!noneval) {
                noneval = ConstAggregate::none_from(anchor);
            }
            outargs[i] = noneval;
        }
    }
    return {};
}

// used by ArgumentList & Call
static SCOPES_RESULT(TypedValue *) prove_arguments(
    const ASTContext &ctx, TypedValues &outargs, const Values &values) {
    SCOPES_RESULT_TYPE(TypedValue *);
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
                    outargs.push_back(ExtractArgument::from(value->anchor(), value, j));
                }
                break;
            } else {
                value = ExtractArgument::from(value->anchor(), value, 0);
            }
        }
        outargs.push_back(value);
    }
    return nullptr;
}

static SCOPES_RESULT(TypedValue *) prove_ArgumentListTemplate(const ASTContext &ctx, ArgumentListTemplate *nlist) {
    SCOPES_RESULT_TYPE(TypedValue *);
    TypedValues values;
    TypedValue *noret = SCOPES_GET_RESULT(prove_arguments(ctx, values, nlist->values));
    if (noret) {
        return noret;
    }
    return ArgumentList::from(nlist->anchor(), values);
}

static SCOPES_RESULT(TypedValue *) prove_ExtractArgumentTemplate(
    const ASTContext &ctx, ExtractArgumentTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    auto value = SCOPES_GET_RESULT(prove(ctx, node->value));
    assert(node->index >= 0);
    if (node->vararg)
        return ExtractArgument::variadic_from(node->anchor(), value, node->index);
    else
        return ExtractArgument::from(node->anchor(), value, node->index);
}

static SCOPES_RESULT(TypedValue *) prove_Loop(const ASTContext &ctx, Loop *loop) {
    SCOPES_RESULT_TYPE(TypedValue *);
    SCOPES_ANCHOR(loop->anchor());

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
            ctx.move(uq->id);
            // move into loop
            T = unique_type(T, ctx.unique_id());
        }
        loop_types.push_back(T);
    }

    auto loopargs = LoopLabelArguments::from(
        loop->anchor(), arguments_type(loop_types));

    LoopLabel *newloop = LoopLabel::from(loop->anchor(), init_values, loopargs);
    // anchor loop to the local block to avoid it landing in the wrong place
    // ctx.append(newloop);
    map_arguments_to_block(ctx.with_block(newloop->body), loopargs);
    ctx.frame->bind(loop->args, newloop->args);

    auto subctx = ctx.for_loop(newloop);
    ASTContext newctx;
    auto result = SCOPES_GET_RESULT(prove_block(subctx, newloop->body, loop->value, newctx));
    //auto rtype = result->get_type();
    make_repeat(newctx, result->anchor(), newloop, result);

    SCOPES_CHECK_RESULT(finalize_repeats(ctx, newloop, "loop repeat"));

    const Type *rtype = newloop->args->get_type();
    for (auto repeat : newloop->repeats) {
        SCOPES_CHECK_RESULT(merge_value_type("loop repeat", rtype,
            arguments_type_from_typed_values(repeat->values)));
    }

    return newloop;
}

static SCOPES_RESULT(TypedValue *) prove_LoopArguments(const ASTContext &ctx, Value *node) { assert(false); return nullptr; }
static SCOPES_RESULT(TypedValue *) prove_ParameterTemplate(const ASTContext &ctx, Value *node) { assert(false); return nullptr; }

const Type *try_get_const_type(Value *node) {
    if (isa<Const>(node))
        return cast<Const>(node)->get_type();
    return TYPE_Unknown;
}

const String *try_extract_string(Value *node) {
    auto ptr = dyn_cast<ConstPointer>(node);
    if (ptr && (ptr->get_type() == TYPE_String))
        return (const String *)ptr->value;
    return nullptr;
}

static SCOPES_RESULT(TypedValue *) prove_Break(const ASTContext &ctx, Break *_break) {
    SCOPES_RESULT_TYPE(TypedValue *);
    SCOPES_ANCHOR(_break->anchor());
    if (!ctx._break) {
        SCOPES_EXPECT_ERROR(error_illegal_break_outside_loop());
    }
    TypedValue *value = SCOPES_GET_RESULT(prove(ctx, _break->value));
    return make_merge(ctx, _break->anchor(), ctx._break, value);
}

static SCOPES_RESULT(TypedValue *) prove_RepeatTemplate(const ASTContext &ctx, RepeatTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    SCOPES_ANCHOR(node->anchor());
    if (!ctx.loop) {
        SCOPES_EXPECT_ERROR(error_illegal_repeat_outside_loop());
    }
    TypedValue *value = SCOPES_GET_RESULT(prove(ctx, node->value));
    return make_repeat(ctx, node->anchor(), ctx.loop, value);
}

static SCOPES_RESULT(TypedValue *) prove_ReturnTemplate(const ASTContext &ctx, ReturnTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    TypedValue *value = SCOPES_GET_RESULT(prove(ctx, node->value));
    if (ctx.frame->label) {
        assert(ctx.frame->original && ctx.frame->original->is_inline());
        // generate a merge
        return make_merge(ctx, node->anchor(), ctx.frame->label, value);
    } else {
        assert(!(ctx.frame->original && ctx.frame->original->is_inline()));
        // generate a return
        return make_return(ctx, node->anchor(), value);
    }
}

static SCOPES_RESULT(TypedValue *) prove_MergeTemplate(const ASTContext &ctx, MergeTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    TypedValue *label = SCOPES_GET_RESULT(ctx.frame->resolve(node->label, ctx.function));
    if (!isa<Label>(label)) {
        SCOPES_EXPECT_ERROR(error_label_expected(label));
    }
    TypedValue *value = SCOPES_GET_RESULT(prove(ctx, node->value));
    if (!is_returning(value->get_type()))
        return value;
    return make_merge(ctx, node->anchor(), cast<Label>(label), value);
}

static SCOPES_RESULT(TypedValue *) prove_RaiseTemplate(const ASTContext &ctx, RaiseTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    assert(ctx.frame);
    TypedValue *value = SCOPES_GET_RESULT(prove(ctx, node->value));
    return make_raise(ctx, node->anchor(), value);
}

bool is_value_stage_constant(Value *value) {
    return isa<Pure>(value) && (cast<Pure>(value)->get_type() != TYPE_ASTMacro);
}

static SCOPES_RESULT(TypedValue *) prove_CompileStage(const ASTContext &ctx, CompileStage *sx) {
    SCOPES_RESULT_TYPE(TypedValue *);

    auto anchor = sx->anchor();
    sc_set_active_anchor(anchor);
    auto scope = sx->env;

    auto docstr = sc_scope_get_docstring(scope, SYM_Unnamed);
    auto constant_scope = sc_scope_new_subscope(scope);
    if (sc_string_count(docstr)) {
        sc_scope_set_docstring(constant_scope, SYM_Unnamed, docstr);
    }

    Symbol last_key = SYM_Unnamed;
    auto tmp =
        CallTemplate::from(anchor, g_sc_scope_new_subscope,
            { ConstPointer::scope_from(sx->anchor(), constant_scope) });

    auto block = Expression::unscoped_from(anchor);
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
            auto value1 = sc_extract_argument_new(value->anchor(), value, 0);
            auto typedvalue1 = SCOPES_GET_RESULT(prove(ctx, value1));
            if (is_value_stage_constant(typedvalue1)) {
                sc_scope_set_symbol(constant_scope, key, typedvalue1);
                sc_scope_set_docstring(constant_scope, key, keydocstr);
            } else {
                Value *wrapvalue = nullptr;
                if (typedvalue1->get_type() == TYPE_Value) {
                    wrapvalue = typedvalue1;
                } else {
                    wrapvalue = wrap_value(typedvalue1->get_type(), typedvalue1);
                }
                if (wrapvalue) {
                    auto vkey = ConstInt::symbol_from(anchor, key);
                    block->append(CallTemplate::from(anchor, g_sc_scope_set_symbol, { tmp, vkey, wrapvalue }));
                    block->append(CallTemplate::from(anchor, g_sc_scope_set_docstring, { tmp, vkey, ConstPointer::string_from(anchor, keydocstr) }));
                }
            }
        }
    }

    block->append(
        CallTemplate::from(anchor, g_bitcast, {
            CallTemplate::from(anchor, g_sc_eval, {
                ConstPointer::anchor_from(anchor),
                ConstPointer::list_from(anchor, sx->next),
                tmp }),
            ConstPointer::type_from(anchor, TYPE_CompileStage)
        })
    );
    //StyledStream ss;
    //stream_ast(ss, block, StreamASTFormat());
    return prove(ctx, block);
}

static SCOPES_RESULT(TypedValue *) prove_KeyedTemplate(const ASTContext &ctx, KeyedTemplate *keyed) {
    SCOPES_RESULT_TYPE(TypedValue *);
    auto value = SCOPES_GET_RESULT(prove(ctx, keyed->value));
    assert(!isa<ArgumentList>(value));
    return Keyed::from(keyed->anchor(), keyed->key, value);
}

template<typename T>
static SCOPES_RESULT(T *) extract_constant(const Type *want, Value *value) {
    SCOPES_RESULT_TYPE(T *);
    /*
    if (isa<PureCast>(value)) {
        value = cast<PureCast>(value)->value;
    }
    */
    auto constval = dyn_cast<T>(value);
    if (!constval) {
        SCOPES_ANCHOR(value->anchor());
        SCOPES_CHECK_RESULT(error_constant_expected(want, value));
    }
    return constval;
}

template<typename T>
static SCOPES_RESULT(T *) extract_typed_constant(const Type *want, Value *value) {
    SCOPES_RESULT_TYPE(T *);
    const Type *TT = nullptr;
    if (isa<PureCast>(value)) {
        TT = cast<PureCast>(value)->get_type();
        value = cast<PureCast>(value)->value;
    }
    auto constval = dyn_cast<T>(value);
    if (!constval) {
        SCOPES_ANCHOR(value->anchor());
        SCOPES_CHECK_RESULT(error_constant_expected(want, value));
    }
    if (!TT) {
        TT = constval->get_type();
    }
    SCOPES_ANCHOR(value->anchor());
    SCOPES_CHECK_RESULT(verify(TT, want));
    return constval;
}

SCOPES_RESULT(const Type *) extract_type_constant(Value *value) {
    SCOPES_RESULT_TYPE(const Type *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_Type, value));
    return (const Type *)x->value;
}

SCOPES_RESULT(const Closure *) extract_closure_constant(Value *value) {
    SCOPES_RESULT_TYPE(const Closure *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_Closure, value));
    return (const Closure *)x->value;
}

SCOPES_RESULT(Function *) extract_function_constant(Value *value) {
    return extract_constant<Function>(TYPE_Function, value);
}

SCOPES_RESULT(sc_ast_macro_func_t) extract_astmacro_constant(Value *value) {
    SCOPES_RESULT_TYPE(sc_ast_macro_func_t);
    ConstPointer* x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_ASTMacro, value));
    return (sc_ast_macro_func_t)x->value;
}

SCOPES_RESULT(const List *) extract_list_constant(Value *value) {
    SCOPES_RESULT_TYPE(const List *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_List, value));
    return (const List *)x->value;
}

SCOPES_RESULT(const String *) extract_string_constant(Value *value) {
    SCOPES_RESULT_TYPE(const String *);
    ConstPointer* x = SCOPES_GET_RESULT(extract_typed_constant<ConstPointer>(TYPE_String, value));
    return (const String *)x->value;
}

SCOPES_RESULT(Builtin) extract_builtin_constant(Value *value) {
    SCOPES_RESULT_TYPE(Builtin);
    ConstInt* x = SCOPES_GET_RESULT(extract_typed_constant<ConstInt>(TYPE_Builtin, value));
    return Builtin((KnownSymbol)x->value);
}

SCOPES_RESULT(Symbol) extract_symbol_constant(Value *value) {
    SCOPES_RESULT_TYPE(Symbol);
    ConstInt* x = SCOPES_GET_RESULT(extract_typed_constant<ConstInt>(TYPE_Symbol, value));
    return Symbol::wrap(x->value);
}

SCOPES_RESULT(uint64_t) extract_integer_constant(Value *value) {
    SCOPES_RESULT_TYPE(uint64_t);
    ConstInt* x = SCOPES_GET_RESULT(extract_constant<ConstInt>(TYPE_Integer, value));
    return x->value;
}

SCOPES_RESULT(ConstAggregate *) extract_vector_constant(Value *value) {
    return extract_constant<ConstAggregate>(TYPE_Vector, value);
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

static SCOPES_RESULT(void) build_deref(
    const ASTContext &ctx, const Anchor *anchor, TypedValue *&val) {
    SCOPES_RESULT_TYPE(void);
    auto T = val->get_type();
    auto rq = try_qualifier<ReferQualifier>(T);
    if (rq) {
        SCOPES_CHECK_RESULT(verify_readable(rq, T));
        if (is_plain(T)) {
            auto retT = strip_qualifier<ReferQualifier>(T);
            auto call = Call::from(anchor, retT, g_deref, { val });
            ctx.append(call);
            val = call;
            return {};
        }
    #if 0
        Value *handler = nullptr;
        auto TT = strip_qualifiers(T);
        if (TT->lookup(SYM_DerefHandler, handler)) {
            auto call = Call::from(anchor, handler, { val });
            val = SCOPES_GET_RESULT(prove(ctx.with_symbol_target(), call));
            return {};
        }
    #endif
        SCOPES_EXPECT_ERROR(error_cannot_deref_non_plain(T));
    }
    return {};
}

#define CHECKARGS(MINARGS, MAXARGS) \
    SCOPES_CHECK_RESULT((checkargs<MINARGS, MAXARGS>(argcount)))
#define ARGTYPE0() ({ \
        Call *newcall = Call::from(call->anchor(), empty_arguments_type(), callee, values); \
        newcall; \
    })
#define ARGTYPE1(ARGT) ({ \
        Call *newcall = Call::from(call->anchor(), ARGT, callee, values); \
        newcall; \
    })
#define DEREF(NAME) \
        SCOPES_CHECK_RESULT(build_deref(ctx, call->anchor(), NAME));
#define READ_VALUE(NAME) \
        assert(argn < argcount); \
        auto && NAME = values[argn++]; \
        DEREF(NAME);
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

static const Type *canonical_return_type(Function *fn, const Type *rettype,
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

static SCOPES_RESULT(const Type *) get_function_type(Function *fn) {
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

static SCOPES_RESULT(const Type *) ensure_function_type(Function *fn) {
    SCOPES_RESULT_TYPE(const Type *);
    const Type *fT = SCOPES_GET_RESULT(get_function_type(fn));
    if (fn->is_typed()) {
        if (fT != fn->get_type()) {
            SCOPES_EXPECT_ERROR(error_recursive_function_changed_type(
                fn, fn->get_type(), fT));
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
SCOPES_RESULT(void) sanitize_tuple_index(const Anchor *anchor, const Type *ST, const T *type, uint64_t &arg, TypedValue *&_arg) {
    SCOPES_RESULT_TYPE(void);
    if (_arg->get_type() == TYPE_Symbol) {
        auto sym = Symbol::wrap(arg);
        size_t idx = type->field_index(sym);
        if (idx == (size_t)-1) {
            StyledString ss;
            ss.out << "no such field " << sym << " in storage type " << ST;
            SCOPES_LOCATION_ERROR(ss.str());
        }
        // rewrite field
        arg = idx;
        _arg = ConstInt::from(anchor, TYPE_I32, idx);
    } else if (_arg->get_type() != TYPE_I32) {
        _arg = ConstInt::from(anchor, TYPE_I32, arg);
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

static SCOPES_RESULT(TypedValue *) prove_call_interior(const ASTContext &ctx, CallTemplate *call) {
    SCOPES_RESULT_TYPE(TypedValue *);
    SCOPES_ANCHOR(call->anchor());
    TypedValue *callee = SCOPES_GET_RESULT(prove(ctx, call->callee));
    TypedValues values;
    auto noret = SCOPES_GET_RESULT(prove_arguments(ctx, values, call->args));
    if (noret) return noret;
    bool rawcall = call->is_rawcall();
    int redirections = 0;
repeat:
    SCOPES_CHECK_RESULT(verify_valid(ctx, callee, "callee"));
    const Type *T = callee->get_type();
    if (!rawcall) {
        assert(redirections < 16);
        Value *dest;
        if (T->lookup_call_handler(dest)) {
            values.insert(values.begin(), callee);
            callee = SCOPES_GET_RESULT(prove(ctx, dest));
            redirections++;
            goto repeat;
        }
    }
    if (is_function_pointer(T)) {
        TypedValues args;
        Symbols keys;
        keys_from_function_type(keys, extract_function_type(T));
        SCOPES_CHECK_RESULT(map_keyed_arguments(call->anchor(), callee, args, values, keys, false));
        values = args;
        SCOPES_CHECK_RESULT(verify_valid(ctx, values, "function call"));
    } else if (T == TYPE_Closure) {
        const Closure *cl = SCOPES_GET_RESULT((extract_closure_constant(callee)));
        {
            TypedValues args;
            Symbols keys;
            bool vararg = keys_from_parameters(keys, cl->func->params);
            SCOPES_CHECK_RESULT(map_keyed_arguments(call->anchor(), callee, args, values, keys, vararg));
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
            callee = SCOPES_GET_RESULT(prove(cl->frame, cl->func, types));
            Function *f = cast<Function>(callee);
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
        auto result = fptr(ArgumentList::from(call->anchor(), values));
        if (result.ok) {
            Value *value = result._0;
            if (!value) {
                StyledString ss;
                ss.out << "AST macro returned illegal value";
                SCOPES_LOCATION_ERROR(ss.str());
            }
            return prove(ctx, value);
        } else {
            SCOPES_RETURN_ERROR(result.except);
        }
    } else if (T == TYPE_Builtin) {
        //SCOPES_CHECK_RESULT(anycl.verify(TYPE_Builtin));
        Builtin b = SCOPES_GET_RESULT(extract_builtin_constant(callee));
        size_t argcount = values.size();
        size_t argn = 0;
        SCOPES_ANCHOR(call->anchor());
        if (b.value() == FN_IsValid) {
            CHECKARGS(1, 1);
            bool valid = ctx.block->is_valid(values[0]);
            return ConstInt::from(call->anchor(), TYPE_Bool, valid);
        }
        SCOPES_CHECK_RESULT(verify_valid(ctx, values, "builtin call"));
        switch(b.value()) {
        /*** DEBUGGING ***/
        case FN_DumpUniques: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump-uniques:";
            for (auto id : ctx.block->valid) {
                ss << " ";
                ss << id;
                auto info = ctx.function->get_unique_info(id);
                ss << "[" << info.get_depth() << "](" << info.value << ")";
            }
            ss << std::endl;
            return ArgumentList::from(call->anchor(), values);
        } break;
        case FN_Dump: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump:";
            for (auto arg : values) {
                ss << " ";
                stream_ast(ss, arg, StreamASTFormat::singleline());
            }
            ss << std::endl;
            return ArgumentList::from(call->anchor(), values);
        } break;
        case FN_DumpAST: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump-ast:";
            for (auto arg : values) {
                ss << std::endl;
                stream_ast(ss, arg, StreamASTFormat());
            }
            ss << std::endl;
            return ArgumentList::from(call->anchor(), values);
        } break;
        case FN_DumpTemplate: {
            StyledStream ss(SCOPES_CERR);
            ss << call->anchor() << " dump-template:";
            for (auto arg : call->args) {
                ss << std::endl;
                stream_ast(ss, arg, StreamASTFormat());
            }
            return ArgumentList::from(call->anchor(), values);
        } break;
        case FN_Annotate: {
            return ARGTYPE0();
        } break;
        /*** VIEW INFERENCE ***/
        case FN_Move: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            auto uq = try_unique(X);
            if (!uq) {
                SCOPES_CHECK_RESULT(error_value_not_unique(_X));
            }
            build_move(ctx, call->anchor(), _X);
            return _X;
        } break;
        case FN_View: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            auto uq = try_unique(X);
            if (!uq) {
                // no effect
                return _X;
            }
            build_view(ctx, call->anchor(), _X);
            return _X;
        } break;
        case FN_Forget: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(X);
            (void)X;
            return ARGTYPE0();
        } break;
        case FN_Viewing: {
            for (size_t i = 0; i < values.size(); ++i) {
                READ_VALUE(value);
                auto param = dyn_cast<Parameter>(value);
                if (!param) {
                    SCOPES_EXPECT_ERROR(error_something_expected("parameter", value));
                }
                param->retype(view_type(param->get_type(), {}));
            }
            return ArgumentList::from(call->anchor(), {});
        } break;
        /*** ARGUMENTS ***/
        case FN_VaCountOf: {
            return ConstInt::from(call->anchor(), TYPE_I32, argcount);
        } break;
        case FN_NullOf: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return ARGTYPE1(T);
            //return SCOPES_GET_RESULT(nullof(call->anchor(), T));
        } break;
        case FN_Undef: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            return ARGTYPE1(T);
        } break;
        case FN_TypeOf: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(A);
            return ConstPointer::type_from(call->anchor(), strip_qualifiers(A));
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
            return ARGTYPE1(it->type);
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
                SCOPES_LOCATION_ERROR(String::from("unsupported dimensionality"));
                break;
            }
            if (it->arrayed) {
                comps++;
            }
            if (comps == 1) {
                return ARGTYPE1(TYPE_I32);
            } else {
                return ARGTYPE1(vector_type(TYPE_I32, comps).assert_ok());
            }
        } break;
        case FN_ImageQueryLod: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ST);
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = SCOPES_GET_RESULT(storage_type(sit->type));
            }
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
            return ARGTYPE1(vector_type(TYPE_F32, 2).assert_ok());
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
            return ARGTYPE1(TYPE_I32);
        } break;
        case FN_ImageRead: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ST);
            SCOPES_CHECK_RESULT(verify_kind<TK_Image>(ST));
            auto it = cast<ImageType>(ST);
            return ARGTYPE1(it->type);
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
                    SCOPES_LOCATION_ERROR(String::from("unsupported execution mode"));
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
            SCOPES_CHECK_RESULT(verify_bool_vector(T1));
            SCOPES_CHECK_RESULT(verify(T2, T3));
            if (T1->kind() == TK_Vector) {
                auto ST2 = SCOPES_GET_RESULT(storage_type(T2));
                SCOPES_CHECK_RESULT(verify_vector_sizes(T1, ST2));
            }
            return ARGTYPE1(T2);
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

                auto uq = try_unique(SrcT);
                if (uq) {
                    ctx.move(uq->id);
                }

                bool target_is_plain = is_plain(DestT);

                DestT = strip_qualifiers(DestT);
                auto vq = try_qualifier<ViewQualifier>(SrcT);
                if (vq) {
                    DestT = view_type(DestT, vq->ids);
                } else if (!target_is_plain) {
                    DestT = unique_type(DestT, ctx.unique_id());
                }

                auto rq = try_qualifier<ReferQualifier>(SrcT);
                if (rq) {
                    DestT = qualify(DestT, { rq });
                    _DestT = ConstPointer::type_from(_DestT->anchor(), DestT);
                }
                if (isa<Pure>(_SrcT) && target_is_plain) {
                    return PureCast::from(call->anchor(), DestT, cast<Pure>(_SrcT));
                } else {
                    return ARGTYPE1(DestT);
                }
            }
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT((verify_kind<TK_Pointer>(SCOPES_GET_RESULT(storage_type(DestT)))));
            return ARGTYPE1(DestT);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return ARGTYPE1(DestT);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return ARGTYPE1(DestT);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if (cast<RealType>(T)->width < cast<RealType>(DestT)->width) {
                SCOPES_EXPECT_ERROR(error_invalid_operands(T, DestT));
            }
            return ARGTYPE1(DestT);
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if (cast<RealType>(T)->width > cast<RealType>(DestT)->width) {
                SCOPES_EXPECT_ERROR(error_invalid_operands(T, DestT));
            }
            return ARGTYPE1(DestT);
        } break;
        case FN_FPToUI:
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_real(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            if ((T != TYPE_F32) && (T != TYPE_F64)) {
                SCOPES_EXPECT_ERROR(error_invalid_operands(T, DestT));
            }
            return ARGTYPE1(DestT);
        } break;
        case FN_UIToFP:
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_real(SCOPES_GET_RESULT(storage_type(DestT))));
            if ((DestT != TYPE_F32) && (DestT != TYPE_F64)) {
                SCOPES_CHECK_RESULT(error_invalid_operands(T, DestT));
            }
            return ARGTYPE1(DestT);
        } break;
        case FN_ZExt:
        case FN_SExt: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(T);
            READ_TYPE_CONST(DestT);
            SCOPES_CHECK_RESULT(verify_integer(T));
            SCOPES_CHECK_RESULT(verify_integer(SCOPES_GET_RESULT(storage_type(DestT))));
            return ARGTYPE1(DestT);
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
                return Call::from(call->anchor(), qualify(RT, { rq }), g_getelementref, { _T, _idx });
            } else {
                return ARGTYPE1(RT);
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
            return ARGTYPE1(_T->get_type());
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
            return ARGTYPE1(SCOPES_GET_RESULT(vector_type(vi->element_type, outcount)));
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(T);
            SCOPES_CHECK_RESULT(verify_real_vector(T));
            if (T->kind() == TK_Vector) {
                return ARGTYPE1(cast<VectorType>(T)->element_type);
            } else {
                return ARGTYPE1(_T->get_type());
            }
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            READ_TYPEOF(A);
            SCOPES_CHECK_RESULT(verify_real_ops(A));
            return ARGTYPE1(A);
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(A);
            READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_real_vector(A, 3));
            SCOPES_CHECK_RESULT(verify(typeof_A, B));
            return ARGTYPE1(typeof_A);
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            READ_TYPEOF(A);
            READ_TYPEOF(B);
            SCOPES_CHECK_RESULT(verify_real_ops(A, B));
            const Type *T = SCOPES_GET_RESULT(storage_type(A));
            if (T->kind() == TK_Vector) {
                return ARGTYPE1(cast<VectorType>(T)->element_type);
            } else {
                return ARGTYPE1(A);
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
                SCOPES_CHECK_RESULT(sanitize_tuple_index(call->anchor(), T, ti, iidx, _idx));
                RT = SCOPES_GET_RESULT(ti->type_at_index(iidx));
            } break;
            case TK_Union: {
                auto iidx = SCOPES_GET_RESULT(extract_integer_constant(_idx));
                auto ui = cast<UnionType>(T);
                SCOPES_CHECK_RESULT(sanitize_tuple_index(call->anchor(), T, ui, iidx, _idx));
                RT = SCOPES_GET_RESULT(ui->type_at_index(iidx));
                _idx = ConstInt::from(_idx->anchor(), TYPE_I32, 0);
                auto TT = tuple_type({RT}).assert_ok();
                RT = type_key(RT)._1;
                auto rq = try_qualifier<ReferQualifier>(typeof_T);
                if (rq) {
                    TT = qualify(TT, {rq});
                    auto newcall2 = Call::from(call->anchor(), TT, g_bitcast,
                        { _T, ConstPointer::type_from(call->anchor(), TT) });
                    ctx.append(newcall2);
                    auto newcall3 = Call::from(call->anchor(), qualify(RT, { rq }),
                        g_getelementref, { newcall2, _idx });
                    ctx.append(newcall3);
                    return newcall3;
                } else {
                    auto newcall2 = Call::from(call->anchor(), TT, g_bitcast,
                        { _T, ConstPointer::type_from(call->anchor(), TT) });
                    ctx.append(newcall2);
                    _T = newcall2;
                    return ARGTYPE1(RT);
                }
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            }
            RT = type_key(RT)._1;
            auto rq = try_qualifier<ReferQualifier>(typeof_T);
            if (rq) {
                auto newcall2 = Call::from(call->anchor(), qualify(RT, { rq }), g_getelementref, { _T, _idx });
                ctx.append(newcall2);
                return newcall2;
            } else {
                return ARGTYPE1(RT);
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
            return ARGTYPE1(AT);
        } break;
        case FN_GetElementRef:
        case FN_GetElementPtr: {
            CHECKARGS(2, -1);
            const Type *T;
            bool is_ref = (b.value() == FN_GetElementRef);
            TypedValue *dep = nullptr;
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
                    SCOPES_CHECK_RESULT(sanitize_tuple_index(call->anchor(), ST, ti, arg, _arg));
                    T = SCOPES_GET_RESULT(ti->type_at_index(arg));
                } break;
                default: {
                    StyledString ss;
                    ss.out << "can not get element pointer from type " << T;
                    SCOPES_LOCATION_ERROR(ss.str());
                } break;
                }
            }
            T = pointer_type(T, pi->flags, pi->storage_class);
            if (is_ref) {
                T = SCOPES_GET_RESULT(ptr_to_ref(T));
            }
            return ARGTYPE1(T);
        } break;
        case FN_Deref: {
            CHECKARGS(1, 1);
            READ_TYPEOF(T);
            (void)T;
            return _T;
        } break;
        case FN_Assign: {
            CHECKARGS(2, 2);
            READ_STORAGETYPEOF(ElemT);
            READ_NODEREF_STORAGETYPEOF(DestT);
            auto rq = SCOPES_GET_RESULT(verify_refer(typeof_DestT));
            if (rq) {
                SCOPES_CHECK_RESULT(verify_writable(rq, typeof_DestT));
            }
            //strip_qualifiers(ElemT);
            //strip_qualifiers(DestT);
            SCOPES_CHECK_RESULT(
                verify(ElemT, DestT));
            return ARGTYPE0();
        } break;
        case FN_PtrToRef: {
            CHECKARGS(1, 1);
            READ_TYPEOF(T);
            auto NT = SCOPES_GET_RESULT(ptr_to_ref(T));
            if (isa<Pure>(_T)) {
                return PureCast::from(call->anchor(), NT, cast<Pure>(_T));
            } else {
                return ARGTYPE1(NT);
            }
        } break;
        case FN_RefToPtr: {
            CHECKARGS(1, 1);
            READ_NODEREF_TYPEOF(T);
            auto NT = SCOPES_GET_RESULT(ref_to_ptr(T));
            if (isa<Pure>(_T)) {
                return PureCast::from(call->anchor(), NT, cast<Pure>(_T));
            } else {
                return ARGTYPE1(NT);
            }
        } break;
        case FN_VolatileLoad:
        case FN_Load: {
            CHECKARGS(1, 1);
            READ_STORAGETYPEOF(T);
            SCOPES_CHECK_RESULT(verify_kind<TK_Pointer>(T));
            SCOPES_CHECK_RESULT(verify_readable(T));
            return ARGTYPE1(cast<PointerType>(T)->element_type);
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
                SCOPES_LOCATION_ERROR(String::from(
                    "pointer is not a heap pointer"));
            }
            return ARGTYPE0();
        } break;
        case FN_StaticAlloc: {
            CHECKARGS(1, 1);
            READ_TYPE_CONST(T);
            void *dst = tracked_malloc(SCOPES_GET_RESULT(size_of(T)));
            return ConstPointer::from(call->anchor(), static_pointer_type(T), dst);
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
            return ARGTYPE1(SCOPES_GET_RESULT(bool_op_return_type(A)));
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
            return ARGTYPE1(SCOPES_GET_RESULT(bool_op_return_type(A)));
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
        case OP_ ## NAME: \
        case OP_ ## NAME ## NUW: \
        case OP_ ## NAME ## NSW: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            return ARGTYPE1(A); \
        } break;
#define IARITH_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A, B)); \
            return ARGTYPE1(A); \
        } break;
#define FARITH_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(2, 2); \
            READ_TYPEOF(A); READ_TYPEOF(B); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B)); \
            return ARGTYPE1(A); \
        } break;
#define FTRI_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(3, 3); \
            READ_TYPEOF(A); READ_TYPEOF(B); READ_TYPEOF(C); \
            SCOPES_CHECK_RESULT(verify_real_ops(A, B, C)); \
            return ARGTYPE1(A); \
        } break;
#define IUN_OP(NAME, PFX) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_integer_ops(A)); \
            return ARGTYPE1(A); \
        } break;
#define FUN_OP(NAME) \
        case OP_ ## NAME: { \
            CHECKARGS(1, 1); \
            READ_TYPEOF(A); \
            SCOPES_CHECK_RESULT(verify_real_ops(A)); \
            return ARGTYPE1(A); \
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

        return ArgumentList::from(call->anchor(), {});
    }
    if (!is_function_pointer(T)) {
        SCOPES_ANCHOR(call->anchor());
        SCOPES_EXPECT_ERROR(error_invalid_call_type(callee));
    }
    const FunctionType *aft = extract_function_type(T);
    const FunctionType *ft = aft->strip_annotations();
    int numargs = (int)ft->argument_types.size();
    if ((!ft->vararg() && (values.size() != numargs))
        || (ft->vararg() && (values.size() < numargs))) {
        SCOPES_ANCHOR(call->anchor());
        SCOPES_EXPECT_ERROR(error_argument_count_mismatch(numargs, values.size(), ft));
    }
    // verify_function_argument_signature
    for (int i = 0; i < numargs; ++i) {
        const Type *Ta = strip_qualifiers(values[i]->get_type(),
            (1 << QK_Key));
        const Type *Tb = ft->argument_types[i];
        if (is_reference(Ta) && !is_reference(Tb)) {
            SCOPES_CHECK_RESULT(build_deref(ctx, call->anchor(), values[i]));
            Ta = values[i]->get_type();
        }
        if (is_view(Tb)) {
            if (!is_view(Ta)) {
                build_view(ctx, call->anchor(), values[i]);
                Ta = values[i]->get_type();
            }
            Tb = strip_view(Tb);
            Ta = strip_view(Ta);
        } else if (is_unique(Tb) && !is_view(Ta)) {
            assert(is_unique(Ta));
            Tb = strip_unique(Tb);
            Ta = strip_unique(Ta);
        }
        if (SCOPES_GET_RESULT(types_compatible(Tb, Ta))) {
            continue;
        }
        SCOPES_ANCHOR(values[i]->anchor());
        SCOPES_EXPECT_ERROR(error_argument_type_mismatch(Tb, Ta));
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
            ctx.move(argu->id);
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
    Call *newcall = Call::from(call->anchor(), rt, callee, values);
    if (ft->has_exception()) {
        // todo: remap exception type
        newcall->except_body.set_parent(ctx.block);
        auto exceptctx = ctx.with_block(newcall->except_body);
        const Type *et = remap_unique_return_arguments(exceptctx, idmap, ft->except_type);
        auto exc = Exception::from(call->anchor(), et);

        map_arguments_to_block(ctx.with_block(newcall->except_body), exc);
        newcall->except = exc;

        SCOPES_CHECK_RESULT(make_raise(exceptctx, call->anchor(), exc));
    }
    return newcall;
}

static SCOPES_RESULT(TypedValue *) prove_CallTemplate(const ASTContext &ctx, CallTemplate *call) {
    SCOPES_RESULT_TYPE(TypedValue *);
    auto result = prove_call_interior(ctx, call);
    if (result.ok()) {
        return result;
    } else {
        auto err = result.assert_error();
        err->append_error_trace(call);
        SCOPES_RETURN_ERROR(err);
    }
}

static Label *make_merge_label(const ASTContext &ctx, const Anchor *anchor) {
    Label *merge_label = Label::from(anchor, LK_BranchMerge);
    merge_label->body.set_parent(ctx.block);
    return merge_label;
}

static SCOPES_RESULT(TypedValue *) finalize_merge_label(const ASTContext &ctx, Label *merge_label, const char *by) {
    SCOPES_RESULT_TYPE(TypedValue *);

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
    merge_back_valid(ctx, valid);
    ctx.append(merge_label);

    return merge_label;
}

static SCOPES_RESULT(TypedValue *) prove_SwitchTemplate(const ASTContext &ctx, SwitchTemplate *node) {
    SCOPES_RESULT_TYPE(TypedValue *);

    auto newexpr = SCOPES_GET_RESULT(prove(ctx, node->expr));
    newexpr = ExtractArgument::from(newexpr->anchor(), newexpr, 0);
    SCOPES_CHECK_RESULT(build_deref(ctx, newexpr->anchor(), newexpr));

    const Type *casetype = newexpr->get_type();

    auto _switch = Switch::from(node->anchor(), newexpr);

    Label *merge_label = make_merge_label(ctx, node->anchor());

    ASTContext subctx = ctx.with_block(merge_label->body);
    subctx.append(_switch);

    Switch::Case *defaultcase = nullptr;
    for (auto &&_case : node->cases) {
        SCOPES_ANCHOR(_case.anchor);
        Switch::Case *newcase = nullptr;
        if (_case.kind == CK_Default) {
            if (defaultcase) {
                SCOPES_EXPECT_ERROR(error_duplicate_default_case());
            }
            newcase = &_switch->append_default(_case.anchor);
            defaultcase = newcase;
        } else {
            auto newlit = SCOPES_GET_RESULT(prove(ctx, _case.literal));
            if (!isa<ConstInt>(newlit)) {
                SCOPES_EXPECT_ERROR(error_invalid_case_literal_type(newlit));
            }
            casetype = SCOPES_GET_RESULT(
                merge_value_type("switch case literal", casetype, newlit->get_type()));
            newcase = &_switch->append_pass(_case.anchor, cast<ConstInt>(newlit));
        }
        assert(_case.value);
        ASTContext newctx;
        auto newvalue = SCOPES_GET_RESULT(prove_block(subctx, newcase->body, _case.value, newctx));
        if (_case.kind == CK_Pass) {
            SCOPES_CHECK_RESULT(validate_pass_block(subctx, newcase->body));
        } else {
            if (is_returning(newvalue->get_type())) {
                make_merge(newctx, newvalue->anchor(), merge_label, newvalue);
            }
        }
    }

    if (!defaultcase) {
        SCOPES_EXPECT_ERROR(error_missing_default_case());
    }

    if (merge_label->merges.empty()) {
        // none of the paths are returning
        // cases do not need a merge label
        assert(ctx.block);
        ctx.merge_block(merge_label->body);
        return _switch;
    }

    return finalize_merge_label(ctx, merge_label, "switch case");
}

static SCOPES_RESULT(TypedValue *) prove_If(const ASTContext &ctx, If *_if) {
    SCOPES_RESULT_TYPE(TypedValue *);
    assert(!_if->clauses.empty());
    int numclauses = _if->clauses.size();
    assert(numclauses >= 1);
    CondBr *first_condbr = nullptr;
    CondBr *last_condbr = nullptr;
    ASTContext subctx(ctx);
    Label *merge_label = make_merge_label(ctx, _if->anchor());
    for (int i = 0; i < numclauses; ++i) {
        auto &&clause = _if->clauses[i];
        assert(clause.anchor);
        SCOPES_ANCHOR(clause.anchor);
        if (clause.is_then()) {
            TypedValue *newcond = SCOPES_GET_RESULT(prove(subctx, clause.cond));
            newcond = ExtractArgument::from(newcond->anchor(), newcond, 0);
            SCOPES_CHECK_RESULT(build_deref(ctx, newcond->anchor(), newcond));
            if (newcond->get_type() != TYPE_Bool) {
                SCOPES_ANCHOR(clause.anchor);
                SCOPES_EXPECT_ERROR(error_invalid_condition_type(newcond));
            }
            CondBr *condbr = CondBr::from(clause.anchor, newcond);
            if (!first_condbr) {
                first_condbr = condbr;
            }
            condbr->then_body.set_parent(&merge_label->body);
            condbr->else_body.set_parent(&merge_label->body);
            auto thenctx = subctx.with_block(condbr->then_body);
            auto thenresult = SCOPES_GET_RESULT(prove(thenctx, clause.value));
            if (is_returning(thenresult->get_type())) {
                make_merge(thenctx, thenresult->anchor(), merge_label, thenresult);
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
                make_merge(elsectx, clause.anchor, merge_label, elseresult);
            }
            last_condbr = nullptr;
        }
    }
    if (last_condbr) {
        // last else value missing
        ASTContext elsectx = ctx.with_block(last_condbr->else_body);
        make_merge(elsectx, _if->anchor(), merge_label,
            ArgumentList::from(_if->anchor(), {}));
    }

    if (merge_label->merges.empty()) {
        // none of the paths are returning
        // conditions do not need a merge label
        assert(ctx.block);
        ctx.merge_block(merge_label->body);
        return first_condbr;
    }

    return finalize_merge_label(ctx, merge_label, "branch");
}

static SCOPES_RESULT(TypedValue *) prove_Template(const ASTContext &ctx, Template *_template) {
    Function *frame = ctx.frame;
    assert(frame);
    return ConstPointer::closure_from(_template->anchor(), Closure::from(_template, frame));
}

static SCOPES_RESULT(TypedValue *) prove_Quote(const ASTContext &ctx, Quote *node) {
    //StyledStream ss;
    //ss << "before quote" << std::endl;
    //stream_ast(ss, node, StreamASTFormat());
    return quote(ctx, node->value);
    //ss << "after quote" << std::endl;
    //stream_ast(ss, value, StreamASTFormat());
}

static SCOPES_RESULT(TypedValue *) prove_Unquote(const ASTContext &ctx, Unquote *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    SCOPES_ANCHOR(node->anchor());
    SCOPES_LOCATION_ERROR(String::from("unexpected unquote"));
}

SCOPES_RESULT(TypedValue *) prove(const ASTContext &ctx, Value *node) {
    SCOPES_RESULT_TYPE(TypedValue *);
    assert(node);
    TypedValue *result = SCOPES_GET_RESULT(ctx.frame->resolve(node, ctx.function));
    if (!result) {
        if (isa<TypedValue>(node)) {
            result = cast<TypedValue>(node);
        } else {
            // we shouldn't set an anchor here because sometimes the parent context
            // is more indicative than the node position
            //SCOPES_CHECK_RESULT(verify_stack());
            switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
            case NAME: result = SCOPES_GET_RESULT(prove_ ## CLASS(ctx, cast<CLASS>(node))); break;
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
        TypedValue *newval = nullptr;
        if (oldsym->is_variadic()) {
            if ((i + 1) < count) {
                SCOPES_ANCHOR(oldsym->anchor());
                SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
            }
            if ((i + 1) == (int)tmpargs.size()) {
                newval = tmpargs[i];
            } else {
                TypedValues args;
                for (int j = i; j < tmpargs.size(); ++j) {
                    args.push_back(tmpargs[j]);
                }
                newval = ArgumentList::from(oldsym->anchor(), args);
            }
        } else if (i < tmpargs.size()) {
            newval = tmpargs[i];
        } else {
            newval = ConstAggregate::none_from(oldsym->anchor());
        }
        ctx.frame->bind(oldsym, newval);
    }
    return {};
}

static SCOPES_RESULT(TypedValue *) prove_inline_body(const ASTContext &ctx,
    const Closure *cl, const TypedValues &nodes) {
    SCOPES_RESULT_TYPE(TypedValue *);
    auto frame = cl->frame;
    auto func = cl->func;
    Timer sum_prove_time(TIMER_Specialize);
    assert(func);
    //int count = (int)func->params.size();
    Function *fn = Function::from(func->anchor(), func->name, {});
    fn->original = func;
    fn->frame = frame;
    Label *label = Label::from(func->anchor(), LK_Inline, func->name);
    fn->label = label;
    fn->boundary = ctx.function;

    // inlines may escape caller loops
    ASTContext subctx = ctx.with_frame(fn);
    SCOPES_CHECK_RESULT(prove_inline_arguments(subctx, func->params, nodes));
    SCOPES_ANCHOR(fn->anchor());
    TypedValue *result_value = nullptr;
    ASTContext bodyctx;
    auto result = prove_block(subctx, label->body, func->value, bodyctx);
    if (result.ok()) {
        result_value = result.assert_ok();
    } else {
        auto err = result.assert_error();
        err->append_error_trace(fn);
        SCOPES_RETURN_ERROR(err);
    }
    if (label->merges.empty()) {
        // label does not need a merge label
        assert(ctx.block);
        result_value = SCOPES_GET_RESULT(
            move_single_merge_value(bodyctx, ctx.block->depth, result_value, "inline return"));
        ctx.merge_block(label->body);
        return result_value;
    } else {
        if (is_returning(result_value->get_type())) {
            make_merge(bodyctx, result_value->anchor(), label, result_value);
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
        merge_back_valid(ctx, valid);
        ctx.append(label);
        return label;
    }
}

SCOPES_RESULT(TypedValue *) prove_inline(const ASTContext &ctx,
    const Closure *cl, const TypedValues &nodes) {
    SCOPES_RESULT_TYPE(TypedValue *);
    auto func = cl->func;
    if (func->recursion >= SCOPES_MAX_RECURSIONS) {
        SCOPES_EXPECT_ERROR(error_recursion_overflow());
    }
    func->recursion++;
    auto result = prove_inline_body(ctx, cl, nodes);
    func->recursion--;
    return result;
}

static SCOPES_RESULT(Function *) prove_body(Function *frame, Template *func, Types types) {
    SCOPES_RESULT_TYPE(Function *);
    Timer sum_prove_time(TIMER_Specialize);
    assert(func);
    canonicalize_argument_types(types);
    Function key(func->anchor(), func->name, {});
    key.original = func;
    key.frame = frame;
    key.instance_args = types;
    auto it = functions.find(&key);
    if (it != functions.end())
        return *it;
    int count = (int)func->params.size();
    Function *fn = Function::from(func->anchor(), func->name, {});
    fn->original = func;
    fn->frame = frame;
    fn->instance_args = types;
    fn->boundary = fn;
    for (int i = 0; i < count; ++i) {
        auto oldparam = func->params[i];
        if (oldparam->is_variadic()) {
            if ((i + 1) < count) {
                SCOPES_ANCHOR(oldparam->anchor());
                SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
            }
            if ((i + 1) == (int)types.size()) {
                auto newparam = Parameter::from(oldparam->anchor(), oldparam->name, types[i]);
                fn->append_param(newparam);
                fn->bind(oldparam, newparam);
            } else {
                Types vtypes;
                TypedValues args;
                for (int j = i; j < types.size(); ++j) {
                    vtypes.push_back(types[j]);
                    auto newparam = Parameter::from(oldparam->anchor(), oldparam->name, types[j]);
                    fn->append_param(newparam);
                    args.push_back(newparam);
                }
                fn->bind(oldparam, ArgumentList::from(oldparam->anchor(), args));
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
            auto newparam = Parameter::from(oldparam->anchor(), oldparam->name, T);
            fn->append_param(newparam);
            fn->bind(oldparam, newparam);
        }
    }
    fn->build_valids();
    functions.insert(fn);

    ASTContext fnctx = ASTContext::from_function(fn);
    SCOPES_ANCHOR(fn->anchor());
    ASTContext bodyctx = fnctx.with_block(fn->body);
    fn->body.valid = fn->valid;
    auto result = prove(bodyctx, func->value);
    if (result.ok()) {
        auto expr = result.assert_ok();
        SCOPES_CHECK_RESULT(make_return(bodyctx, expr->anchor(), expr));
    } else {
        auto err = result.assert_error();
        err->append_error_trace(fn);
        SCOPES_RETURN_ERROR(err);
    }

    SCOPES_CHECK_RESULT(ensure_function_type(fn));
    SCOPES_CHECK_RESULT(finalize_returns_raises(bodyctx));
    //SCOPES_CHECK_RESULT(track(fnctx));
    fn->complete = true;
    return fn;
}

SCOPES_RESULT(Function *) prove(Function *frame, Template *func, const Types &types) {
    SCOPES_RESULT_TYPE(Function *);
    if (func->recursion >= SCOPES_MAX_RECURSIONS) {
        SCOPES_EXPECT_ERROR(error_recursion_overflow());
    }
    func->recursion++;
    auto result = prove_body(frame, func, types);
    func->recursion--;
    return result;
}

//------------------------------------------------------------------------------

} // namespace scopes

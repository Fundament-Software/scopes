/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "ast_specializer.hpp"
#include "ast.hpp"

#include "dyn_cast.inc"

namespace scopes {

struct ASTContext {
    ASTFunction *frame;
    Loop *loop;

    ASTContext(ASTFunction *_frame, Loop *_loop = nullptr) :
        frame(_frame), loop(_loop) {
        assert(frame);
    }
};

// returns
static SCOPES_RESULT(ASTNode *) specialize(const ASTContext &ctx, ASTNode *node);

static SCOPES_RESULT(Block *) specialize_Block(const ASTContext &ctx, Block *block) {
    SCOPES_RESULT_TYPE(Block *);
    Block *newblock = Block::from(block->anchor());
    for (auto &&src : block->body) {
        newblock->append(SCOPES_GET_RESULT(specialize(ctx, src)));
    }
    return newblock;
}

static SCOPES_RESULT(ASTArgumentList *) specialize_ASTArgumentList(const ASTContext &ctx, ASTArgumentList *nlist) {
    SCOPES_RESULT_TYPE(ASTArgumentList *);
    ASTArgumentList *newnlist = ASTArgumentList::from(nlist->anchor());
    for (auto &&value : nlist->values) {
        newnlist->append(SCOPES_GET_RESULT(specialize(ctx, value)));
    }
    newnlist->flatten();
    return newnlist;
}

static SCOPES_RESULT(ASTNode *) specialize_Let(const ASTContext &ctx, Let *let) {
    SCOPES_RESULT_TYPE(ASTNode *);
    Let *newlet = Let::from(let->anchor());
    // todo
    return newlet;
}

static SCOPES_RESULT(Loop *) specialize_Loop(const ASTContext &ctx, Loop *loop) {
    SCOPES_RESULT_TYPE(Loop *);
    Loop *newloop = Loop::from(loop->anchor());
    // todo
    return newloop;
}

static SCOPES_RESULT(Const *) specialize_Const(const ASTContext &ctx, Const *vconst) {
    SCOPES_RESULT_TYPE(Const *);
    return vconst;
}

static SCOPES_RESULT(Break *) specialize_Break(const ASTContext &ctx, Break *_break) {
    SCOPES_RESULT_TYPE(Break *);
    ASTNode *value = SCOPES_GET_RESULT(specialize(ctx, _break->value));
    return Break::from(_break->anchor(), value);
}

static SCOPES_RESULT(Repeat *) specialize_Repeat(const ASTContext &ctx, Repeat *_repeat) {
    SCOPES_RESULT_TYPE(Repeat *);
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, _repeat->args));
    return Repeat::from(_repeat->anchor(), args);
}

static SCOPES_RESULT(ASTReturn *) specialize_ASTReturn(const ASTContext &ctx, ASTReturn *_return) {
    SCOPES_RESULT_TYPE(ASTReturn *);
    ASTNode *value = SCOPES_GET_RESULT(specialize(ctx, _return->value));
    return ASTReturn::from(_return->anchor(), value);
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

static SCOPES_RESULT(ASTNode *) specialize_Call(const ASTContext &ctx, Call *call) {
    SCOPES_RESULT_TYPE(ASTNode *);
    ASTNode *callee = SCOPES_GET_RESULT(specialize(ctx, call->callee));
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, call->args));
    return Call::from(call->anchor(), callee, args);
}

static SCOPES_RESULT(ASTNode *) specialize_ASTSymbol(const ASTContext &ctx, ASTSymbol *sym) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(false); // todo
    return nullptr;
    //return SCOPES_GET_RESULT(ctx.func->resolve_symbol(sym));
}

static SCOPES_RESULT(If *) specialize_If(const ASTContext &ctx, If *_if) {
    SCOPES_RESULT_TYPE(If *);
    If *newif = If::from(_if->anchor());
    for (auto &&clause : _if->clauses) {
        if (clause.cond) {
            newif->append(
                SCOPES_GET_RESULT(specialize(ctx, clause.cond)),
                SCOPES_GET_RESULT(specialize_Block(ctx, clause.body)));
        } else {
            newif->append(SCOPES_GET_RESULT(specialize_Block(ctx, clause.body)));
        }
    }
    return newif;
}

// this must never happen
static SCOPES_RESULT(ASTNode *) specialize_Template(const ASTContext &ctx, Template *_template) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(false); // todo
    return nullptr;
}

static SCOPES_RESULT(ASTFunction *) specialize_ASTFunction(const ASTContext &ctx, ASTFunction *fn) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    return fn;
}

SCOPES_RESULT(ASTNode *) specialize(const ASTContext &ctx, ASTNode *node) {
    SCOPES_RESULT_TYPE(ASTNode *);
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
#if 0
    ASTSymbols params;
    params.reserve(types.size());
    int count = (int)func->params.size();
    for (int i = 0; i < count; ++i) {
        auto oldparam = func->params[i];
        if (oldparam->is_variadic()) {
            assert((i + 1) == count);

        } else {
            const Type *T = nullptr;
            if (oldparam->type == TYPE_Unknown) {
                oldparam->is_variadic()
            }
            ASTSymbol::from(oldparam->anchor(), oldparam->name, T);
        }
    }
    ASTFunction *fn = ASTFunction::from(func->anchor(), func->name, params, func->body);

    specialize_ASTFunction(ASTContext(frame), fn);
    return sfunc;
#endif
    return nullptr;
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

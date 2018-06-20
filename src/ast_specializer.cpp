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
    ASTFunction *func;
    Loop *loop;

    ASTContext(ASTFunction *_func, Loop *_loop = nullptr) :
        func(_func), loop(_loop) {
        assert(func);
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
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, _break->args));
    return Break::from(_break->anchor(), args);
}

static SCOPES_RESULT(Repeat *) specialize_Repeat(const ASTContext &ctx, Repeat *_repeat) {
    SCOPES_RESULT_TYPE(Repeat *);
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, _repeat->args));
    return Repeat::from(_repeat->anchor(), args);
}

static SCOPES_RESULT(Return *) specialize_Return(const ASTContext &ctx, Return *_return) {
    SCOPES_RESULT_TYPE(Return *);
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, _return->args));
    return Return::from(_return->anchor(), args);
}

static SCOPES_RESULT(ASTNode *) specialize_SyntaxExtend(const ASTContext &ctx, SyntaxExtend *sx) {
    SCOPES_RESULT_TYPE(ASTNode *);
    assert(false);
    return nullptr;
}

static SCOPES_RESULT(ASTNode *) specialize_Call(const ASTContext &ctx, Call *call) {
    SCOPES_RESULT_TYPE(ASTNode *);
    ASTNode *callee = SCOPES_GET_RESULT(specialize(ctx, call->callee));
    ASTArgumentList *args = SCOPES_GET_RESULT(specialize_ASTArgumentList(ctx, call->args));
    return Call::from(call->anchor(), callee, args);
}

static SCOPES_RESULT(ASTNode *) specialize_ASTSymbol(const ASTContext &ctx, ASTSymbol *sym) {
    SCOPES_RESULT_TYPE(ASTNode *);
    return SCOPES_GET_RESULT(ctx.func->resolve_symbol(sym));
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

// the function is already instantiated
static SCOPES_RESULT(ASTFunction *) specialize_ASTFunction(const ASTContext &ctx, ASTFunction *fn) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    fn->body = SCOPES_GET_RESULT(specialize_Block(ASTContext(fn), fn->body));
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

} // namespace scopes

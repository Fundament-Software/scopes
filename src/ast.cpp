
/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "ast.hpp"
#include "type.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------

ASTKind ASTNode::kind() const { return _kind; }

ASTNode::ASTNode(ASTKind kind, const Anchor *_anchor)
    : _kind(kind),anchor(_anchor) {
    assert(anchor);
}

const Anchor *ASTNode::get_anchor() const {
    return anchor;
}

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const ASTNode *T) { \
        return T->kind() == NAME; \
    }
SCOPES_AST_KIND()
#undef T

//------------------------------------------------------------------------------

ASTFunction::ASTFunction(const Anchor *anchor, Symbol _name, const ASTSymbols &_params, Block *_block)
    : ASTNode(ASTK_Function, anchor),
        name(_name), params(_params), block(_block),
        _inline(false), docstring(nullptr) {
}

bool ASTFunction::is_forward_decl() const {
    return !block;
}

void ASTFunction::set_inline() {
    _inline = true;
}

bool ASTFunction::is_inline() const {
    return _inline;
}

void ASTFunction::append_param(ASTSymbol *sym) {
    params.push_back(sym);
}

Block *ASTFunction::ensure_body() {
    if (!block) {
        block = Block::from(anchor);
    }
    return block;
}

ASTFunction *ASTFunction::from(
    const Anchor *anchor, Symbol name,
    const ASTSymbols &params, Block *block) {
    return new ASTFunction(anchor, name, params, block);
}

//------------------------------------------------------------------------------

Block::Block(const Anchor *anchor, const ASTNodes &_body)
    : ASTNode(ASTK_Block, anchor), body(_body) {
}

Block *Block::from(const Anchor *anchor, const ASTNodes &nodes) {
    return new Block(anchor, nodes);
}

void Block::append(ASTNode *node) {
    body.push_back(node);
}

//------------------------------------------------------------------------------

If::If(const Anchor *anchor, const CondExprs &_exprs)
    : ASTNode(ASTK_If, anchor), exprs(_exprs) {
}

If *If::from(const Anchor *anchor, const CondExprs &exprs) {
    return new If(anchor, exprs);
}

ASTNode *If::get_else_expr() const {
    if (!exprs.empty()) {
        const CondExpr &last = exprs.back();
        if (!last.cond)
            return last.expr;
    }
    return nullptr;
}

void If::append_expr(ASTNode *cond, ASTNode *expr) {
    assert(!get_else_expr());
    exprs.push_back({cond, expr});
}

void If::append_expr(ASTNode *expr) {
    assert(!get_else_expr());
    exprs.push_back({nullptr, expr});
}

//------------------------------------------------------------------------------

ASTSymbol::ASTSymbol(const Anchor *anchor, Symbol _name, const Type *_type, bool _variadic)
    : ASTNode(ASTK_Symbol, anchor), name(_name), type(_type), variadic(_variadic) {
    if (!type)
        type = TYPE_Unknown;
}

ASTSymbol *ASTSymbol::from(const Anchor *anchor, Symbol name, const Type *type) {
    return new ASTSymbol(anchor, name, type, false);
}

ASTSymbol *ASTSymbol::variadic_from(const Anchor *anchor, Symbol name, const Type *type) {
    return new ASTSymbol(anchor, name, type, true);
}

bool ASTSymbol::is_variadic() const {
    return variadic;
}

//------------------------------------------------------------------------------

Call::Call(const Anchor *anchor, ASTNode *_callee, const ASTArguments &_args)
    : ASTNode(ASTK_Call, anchor), callee(_callee), args(_args), flags(0) {
}

void Call::append(const ASTArgument &arg) {
    assert(arg.expr);
    args.push_back(arg);
}

void Call::append(Symbol key, ASTNode *node) {
    append({key, node});
}

void Call::append(ASTNode *node) {
    append({SYM_Unnamed, node});
}

Call *Call::from(const Anchor *anchor, ASTNode *callee, const ASTArguments &args) {
    return new Call(anchor, callee, args);
}

//------------------------------------------------------------------------------

Let::Let(const Anchor *anchor, const ASTSymbols &_symbols, const ASTNodes &_exprs)
    : ASTNode(ASTK_Let, anchor), symbols(_symbols), exprs(_exprs) {
}

Let *Let::from(const Anchor *anchor, const ASTSymbols &symbols, const ASTNodes &exprs) {
    return new Let(anchor, symbols, exprs);
}

bool Let::is_variadic() const {
    if (symbols.empty()) return false;
    return symbols.back()->is_variadic();
}

void Let::append_symbol(ASTSymbol *sym) {
    symbols.push_back(sym);
}

void Let::append_expr(ASTNode *expr) {
    exprs.push_back(expr);
}

//------------------------------------------------------------------------------

Const::Const(const Anchor *anchor, Any _value) :
    ASTNode(ASTK_Const, anchor), value(_value) {
}

Const *Const::from(const Anchor *anchor, Any value) {
    return new Const(anchor, value);
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


} // namespace scopes

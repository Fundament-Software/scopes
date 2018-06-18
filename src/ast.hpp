/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_AST_HPP
#define SCOPES_AST_HPP

#include "symbol.hpp"
#include "any.hpp"

#include <vector>

namespace scopes {

struct Anchor;
struct List;
struct Scope;

#define SCOPES_AST_KIND() \
    T(ASTK_Function, "ast-kind-function", ASTFunction) \
    T(ASTK_Block, "ast-kind-block", Block) \
    T(ASTK_If, "ast-kind-if", If) \
    T(ASTK_Symbol, "ast-kind-symbol", ASTSymbol) \
    T(ASTK_Call, "ast-kind-call", Call) \
    T(ASTK_Let, "ast-kind-let", Let) \
    T(ASTK_Const, "ast-kind-const", Const) \
    T(ASTK_Break, "ast-kind-break", Break) \
    T(ASTK_Return, "ast-kind-break", Return)

enum ASTKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    SCOPES_AST_KIND()
#undef T
};

// forward declarations
#define T(NAME, BNAME, CLASS) struct CLASS;
    SCOPES_AST_KIND()
#undef T

struct ASTNode;

typedef std::vector<ASTSymbol *> ASTSymbols;
typedef std::vector<ASTNode *> ASTNodes;
typedef std::vector<Block *> Blocks;

//------------------------------------------------------------------------------

struct ASTNode {
    ASTKind kind() const;

    ASTNode(ASTKind _kind, const Anchor *_anchor);
    ASTNode(const ASTNode &other) = delete;

    const Anchor *get_anchor() const;

private:
    const ASTKind _kind;

protected:
    const Anchor *anchor;
};

//------------------------------------------------------------------------------

struct ASTFunction : ASTNode {
    static bool classof(const ASTNode *T);

    ASTFunction(const Anchor *anchor, Symbol name, const ASTSymbols &params, Block *block);

    bool is_forward_decl() const;
    void set_inline();
    bool is_inline() const;
    void append_param(ASTSymbol *sym);
    Block *ensure_body();

    static ASTFunction *from(
        const Anchor *anchor, Symbol name,
        const ASTSymbols &params = {}, Block *block = nullptr);

    Symbol name;
    ASTSymbols params;
    Block *block;
    bool _inline;
    const String *docstring;
};

//------------------------------------------------------------------------------

struct Block : ASTNode {
    static bool classof(const ASTNode *T);

    Block(const Anchor *anchor, const ASTNodes &nodes);
    void append(ASTNode *node);

    static Block *from(const Anchor *anchor, const ASTNodes &nodes = {});

    ASTNodes body;
};

//------------------------------------------------------------------------------

struct CondExpr {
    ASTNode *cond;
    ASTNode *expr;
};

typedef std::vector<CondExpr> CondExprs;

struct If : ASTNode {
    static bool classof(const ASTNode *T);

    If(const Anchor *anchor, const CondExprs &exprs);
    static If *from(const Anchor *anchor, const CondExprs &exprs = {});

    ASTNode *get_else_expr() const;
    void append_expr(ASTNode *cond, ASTNode *expr);
    void append_expr(ASTNode *expr);

    CondExprs exprs;
};

//------------------------------------------------------------------------------

struct ASTSymbol : ASTNode {
    static bool classof(const ASTNode *T);

    ASTSymbol(const Anchor *anchor, Symbol name, const Type *type, bool variadic);
    static ASTSymbol *from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);
    static ASTSymbol *variadic_from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);

    bool is_variadic() const;

    Symbol name;
    const Type *type;
    bool variadic;
};

//------------------------------------------------------------------------------

struct ASTArgument {
    Symbol key;
    ASTNode *expr;
};

typedef std::vector<ASTArgument> ASTArguments;

enum CallFlags {
    CF_RawCall = (1 << 0),
    CF_TryCall = (1 << 1),
};

struct Call : ASTNode {
    static bool classof(const ASTNode *T);

    Call(const Anchor *anchor, ASTNode *callee, const ASTArguments &args);
    static Call *from(const Anchor *anchor, ASTNode *callee, const ASTArguments &args = {});

    void append(const ASTArgument &arg);
    void append(Symbol key, ASTNode *node);
    void append(ASTNode *node);

    ASTNode *callee;
    ASTArguments args;
    uint32_t flags;
};

//------------------------------------------------------------------------------

struct Let : ASTNode {

    static bool classof(const ASTNode *T);

    Let(const Anchor *anchor, const ASTSymbols &symbols, const ASTNodes &exprs);
    static Let *from(const Anchor *anchor, const ASTSymbols &symbols = {}, const ASTNodes &exprs = {});

    bool is_variadic() const;

    void append_symbol(ASTSymbol *sym);
    void append_expr(ASTNode *expr);

    ASTSymbols symbols;
    ASTNodes exprs;
};

//------------------------------------------------------------------------------

struct Const : ASTNode {
    static bool classof(const ASTNode *T);

    Const(const Anchor *anchor, Any value);
    static Const *from(const Anchor *anchor, Any value);

    Any value;
};

//------------------------------------------------------------------------------

struct Break : ASTNode {
    static bool classof(const ASTNode *T);

    Block *target;
    ASTArguments args;
};

//------------------------------------------------------------------------------

struct Return : ASTNode {
    static bool classof(const ASTNode *T);

    ASTArguments args;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_AST_HPP

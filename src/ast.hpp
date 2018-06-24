/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_AST_HPP
#define SCOPES_AST_HPP

#include "symbol.hpp"
#include "any.hpp"
#include "result.hpp"
#include "type.hpp"

#include <vector>
#include <unordered_map>

namespace scopes {

struct Anchor;
struct List;
struct Scope;
struct ReturnType;

#define SCOPES_AST_KIND() \
    T(ASTK_Template, "ast-kind-template", Template) \
    T(ASTK_Function, "ast-kind-function", ASTFunction) \
    T(ASTK_Block, "ast-kind-block", Block) \
    T(ASTK_If, "ast-kind-if", If) \
    T(ASTK_Symbol, "ast-kind-symbol", ASTSymbol) \
    T(ASTK_Keyed, "ast-kind-keyed", Keyed) \
    T(ASTK_ArgumentList, "ast-kind-argumentlist", ASTArgumentList) \
    T(ASTK_ExtractArgument, "ast-kind-extractargument", ASTExtractArgument) \
    T(ASTK_Call, "ast-kind-call", Call) \
    T(ASTK_Let, "ast-kind-let", Let) \
    T(ASTK_Loop, "ast-kind-loop", Loop) \
    T(ASTK_Const, "ast-kind-const", Const) \
    T(ASTK_Break, "ast-kind-break", Break) \
    T(ASTK_Repeat, "ast-kind-repeat", Repeat) \
    T(ASTK_Return, "ast-kind-return", ASTReturn) \
    T(ASTK_SyntaxExtend, "ast-kind-syntax-extend", SyntaxExtend)

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
struct ASTValue;

typedef std::vector<ASTSymbol *> ASTSymbols;
typedef std::vector<ASTNode *> ASTNodes;
typedef std::vector<ASTValue *> ASTValues;
typedef std::vector<Block *> Blocks;

//------------------------------------------------------------------------------
/*
    each ast node can have several ways to continue flow, which are not
    mutually exclusive, and each flow method can have its own type:

    1. continue: the continue type is forwarded to whatever
        node needs to process it; if there's no continue type, the expression
        always returns

        for loops, the break statement defines the continue type.

    2. return: the return type leaves the function and types the function's
        continue type; if there's no return type, the expression always
        continues


*/

//------------------------------------------------------------------------------

struct ASTNode {
    ASTKind kind() const;

    ASTNode(ASTKind _kind, const Anchor *_anchor);
    ASTNode(const ASTNode &other) = delete;

    const Anchor *anchor() const;

    bool is_typed() const;
    void set_type(const Type *type);
    const Type *get_type() const;
private:
    const ASTKind _kind;
    const Type *_type;

protected:
    const Anchor *_anchor;
};

//------------------------------------------------------------------------------

struct ASTValue : ASTNode {
    static bool classof(const ASTNode *T);

    ASTValue(ASTKind _kind, const Anchor *anchor);
};

//------------------------------------------------------------------------------

struct Keyed : ASTNode {
    static bool classof(const ASTNode *T);

    Keyed(const Anchor *anchor, Symbol key, ASTNode *node);

    static Keyed *from(const Anchor *anchor, Symbol key, ASTNode *node);

    Symbol key;
    ASTNode *value;
};

//------------------------------------------------------------------------------

struct ASTArgumentList : ASTNode {
    static bool classof(const ASTNode *T);

    ASTArgumentList(const Anchor *anchor, const ASTNodes &values);

    void append(Symbol key, ASTNode *node);
    void append(ASTNode *node);

    static ASTArgumentList *from(const Anchor *anchor, const ASTNodes &values = {});

    ASTNodes values;
};

//------------------------------------------------------------------------------

struct ASTExtractArgument : ASTNode {
    static bool classof(const ASTNode *T);

    ASTExtractArgument(const Anchor *anchor, ASTNode *value, int index);

    static ASTExtractArgument *from(const Anchor *anchor, ASTNode *value, int index);

    ASTNode *value;
    int index;
};

//------------------------------------------------------------------------------

struct Template : ASTValue {
    static bool classof(const ASTNode *T);

    Template(const Anchor *anchor, Symbol name, const ASTSymbols &params, ASTNode *value);

    bool is_forward_decl() const;
    void set_inline();
    bool is_inline() const;
    void append_param(ASTSymbol *sym);

    static Template *from(
        const Anchor *anchor, Symbol name,
        const ASTSymbols &params = {}, ASTNode *value = nullptr);

    Symbol name;
    ASTSymbols params;
    ASTNode *value;
    bool _inline;
    const String *docstring;
    Template *scope;
};

//------------------------------------------------------------------------------

struct ASTFunction : ASTValue {
    static bool classof(const ASTNode *T);

    ASTFunction(const Anchor *anchor, Symbol name, const ASTSymbols &params, ASTNode *value);

    static ASTFunction *from(
        const Anchor *anchor, Symbol name,
        const ASTSymbols &params, ASTNode *value);

    void append_param(ASTSymbol *sym);

    Symbol name;
    ASTSymbols params;
    ASTNode *value;
    const String *docstring;
    const Type *return_type;
    ASTFunction *frame;
    Template *original;

    ArgTypes instance_args;
    ASTFunction *find_frame(Template *scope);
    void bind(ASTNode *oldnode, ASTNode *newnode);
    ASTNode *resolve(ASTNode *node);
    std::unordered_map<ASTNode *, ASTNode *> map;
};

//------------------------------------------------------------------------------

struct Block : ASTNode {
    static bool classof(const ASTNode *T);

    Block(const Anchor *anchor, const ASTNodes &nodes, ASTNode *value);
    void append(ASTNode *node);

    static Block *from(const Anchor *anchor, const ASTNodes &nodes = {}, ASTNode *value = nullptr);

    void strip_constants();
    ASTNode *canonicalize();

    ASTNodes body;
    ASTNode *value;
};

//------------------------------------------------------------------------------

struct Clause {
    const Anchor *anchor;
    ASTNode *cond;
    ASTNode *value;

    Clause() : anchor(nullptr), cond(nullptr), value(nullptr) {}
    Clause(const Anchor *_anchor, ASTNode *_cond, ASTNode *_value)
        : anchor(_anchor), cond(_cond), value(_value) {}
    Clause(const Anchor *_anchor, ASTNode *_value)
        : anchor(_anchor), cond(nullptr), value(_value) {}
};

typedef std::vector<Clause> Clauses;

struct If : ASTNode {
    static bool classof(const ASTNode *T);

    If(const Anchor *anchor, const Clauses &clauses);

    static If *from(const Anchor *anchor, const Clauses &clauses = {});

    ASTNode *get_else_clause() const;
    void append(const Anchor *anchor, ASTNode *cond, ASTNode *value);
    void append(const Anchor *anchor, ASTNode *value);
    ASTNode *canonicalize();

    Clauses clauses;
    Clause else_clause;
};

//------------------------------------------------------------------------------

struct ASTSymbol : ASTValue {
    static bool classof(const ASTNode *T);

    ASTSymbol(const Anchor *anchor, Symbol name, const Type *type, bool variadic);
    static ASTSymbol *from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);
    static ASTSymbol *variadic_from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);

    bool is_variadic() const;

    Symbol name;
    bool variadic;
};

//------------------------------------------------------------------------------

enum CallFlags {
    CF_RawCall = (1 << 0),
    CF_TryCall = (1 << 1),
};

struct Call : ASTNode {
    static bool classof(const ASTNode *T);

    Call(const Anchor *anchor, ASTNode *callee, const ASTNodes &args);
    static Call *from(const Anchor *anchor, ASTNode *callee, const ASTNodes &args = {});
    bool is_rawcall() const;
    void set_rawcall();
    bool is_trycall() const;
    void set_trycall();

    ASTNode *callee;
    ASTNodes args;
    uint32_t flags;
};

//------------------------------------------------------------------------------

struct Let : ASTNode {
    static bool classof(const ASTNode *T);

    Let(const Anchor *anchor, const ASTSymbols &params, const ASTNodes &args);

    static Let *from(const Anchor *anchor, const ASTSymbols &params = {}, const ASTNodes &args = {});

    ASTSymbols params;
    ASTNodes args;
};

//------------------------------------------------------------------------------

struct Loop : ASTNode {
    static bool classof(const ASTNode *T);

    Loop(const Anchor *anchor, const ASTSymbols &params, const ASTNodes &args, ASTNode *value);

    static Loop *from(const Anchor *anchor, const ASTSymbols &params = {}, const ASTNodes &args = {}, ASTNode *value = nullptr);

    ASTSymbols params;
    ASTNodes args;
    ASTNode *value;
    const Type *return_type;
};

//------------------------------------------------------------------------------

struct Const : ASTValue {
    static bool classof(const ASTNode *T);

    Const(const Anchor *anchor, Any value);

    static Const *from(const Anchor *anchor, Any value);

    Any value;
};

//------------------------------------------------------------------------------

struct Break : ASTNode {
    static bool classof(const ASTNode *T);

    Break(const Anchor *anchor, ASTNode *value);

    static Break *from(const Anchor *anchor, ASTNode *value);

    ASTNode *value;
};

//------------------------------------------------------------------------------

struct Repeat : ASTNode {
    static bool classof(const ASTNode *T);

    Repeat(const Anchor *anchor, const ASTNodes &args);

    static Repeat *from(const Anchor *anchor, const ASTNodes &args = {});

    ASTNodes args;
};

//------------------------------------------------------------------------------

struct ASTReturn : ASTNode {
    static bool classof(const ASTNode *T);

    ASTReturn(const Anchor *anchor, ASTNode *value);

    static ASTReturn *from(const Anchor *anchor, ASTNode *value);

    ASTNode *value;
};

//------------------------------------------------------------------------------

struct SyntaxExtend : ASTNode {
    static bool classof(const ASTNode *T);

    SyntaxExtend(const Anchor *anchor, Template *func, const List *next, Scope *env);

    static SyntaxExtend *from(const Anchor *anchor, Template *func, const List *next, Scope *env);

    Template *func;
    const List *next;
    Scope *env;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_AST_HPP

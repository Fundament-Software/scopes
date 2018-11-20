/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VALUE_HPP
#define SCOPES_VALUE_HPP

#include "symbol.hpp"
#include "result.hpp"
#include "type.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace scopes {

struct Anchor;
struct List;
struct Scope;

#define SCOPES_TEMPLATE_VALUE_KIND() \
    T(VK_Template, "value-kind-template", Template) \
    T(VK_LabelTemplate, "value-kind-label-template", LabelTemplate) \
    T(VK_Loop, "value-kind-loop", Loop) \
    T(VK_Keyed, "value-kind-keyed", Keyed) \
    T(VK_Expression, "value-kind-expression", Expression) \
    T(VK_Quote, "value-kind-quote", Quote) \
    T(VK_Unquote, "value-kind-unquote", Unquote) \
    T(VK_CompileStage, "value-kind-compile-stage", CompileStage) \
    T(VK_If, "value-kind-if", If) \
    T(VK_MergeTemplate, "value-kind-merge-template", MergeTemplate) \
    T(VK_Break, "value-kind-break", Break) \


#define SCOPES_PURE_VALUE_KIND() \
    T(VK_Function, "value-kind-function", Function) \
    T(VK_Extern, "value-kind-extern", Extern) \
    /* constants (Const::classof) */ \
    T(VK_ConstInt, "value-kind-const-int", ConstInt) \
    T(VK_ConstReal, "value-kind-const-real", ConstReal) \
    T(VK_ConstAggregate, "value-kind-const-aggregate", ConstAggregate) \
    T(VK_ConstPointer, "value-kind-const-pointer", ConstPointer) \


#define SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Merge, "value-kind-merge", Merge) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \


#define SCOPES_INSTRUCTION_VALUE_KIND() \
    T(VK_Label, "value-kind-label", Label) \
    T(VK_LoopLabel, "value-kind-looplabel", LoopLabel) \
    T(VK_CondBr, "value-kind-condbr", CondBr) \
    T(VK_Switch, "value-kind-switch", Switch) \
    T(VK_Call, "value-kind-call", Call) \
    T(VK_ArgumentList, "value-kind-argumentlist", ArgumentList) \
    T(VK_ExtractArgument, "value-kind-extractargument", ExtractArgument) \


#define SCOPES_VALUE_KIND() \
    T(VK_Parameter, "value-kind-parameter", Parameter) \
    T(VK_Exception, "value-kind-exception", Exception) \
    /* template-only */ \
    SCOPES_TEMPLATE_VALUE_KIND() \
    /* instructions (Instruction::classof) */ \
    SCOPES_INSTRUCTION_VALUE_KIND() \
    SCOPES_TERMINATOR_VALUE_KIND() \
    /* pure (Pure::classof), which includes constants */ \
    SCOPES_PURE_VALUE_KIND() \


enum ValueKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    SCOPES_VALUE_KIND()
#undef T
};

// forward declarations
#define T(NAME, BNAME, CLASS) struct CLASS;
    SCOPES_VALUE_KIND()
#undef T

struct Value;
struct Block;
struct Instruction;
struct Const;
struct Raise;

typedef std::vector<Parameter *> Parameters;
typedef std::vector<Value *> Values;
typedef std::unordered_set<Value *> ValueSet;
typedef std::vector<ValueSet> ValueSetArray;
typedef std::vector<Instruction *> Instructions;
typedef std::vector<Const *> Constants;
typedef std::vector<Block *> Blocks;

const char *get_value_kind_name(ValueKind kind);
const char *get_value_class_name(ValueKind kind);

//------------------------------------------------------------------------------

struct ValueIndex {
    struct Hash {
        std::size_t operator()(const ValueIndex & s) const;
    };
    typedef std::unordered_set<ValueIndex, ValueIndex::Hash> Set;

    ValueIndex(Value *_value, int _index = 0);
    bool operator ==(const ValueIndex &other) const;

    const Type *get_type() const;
    const Set *deps() const;
    bool has_deps() const;

    Value *value;
    int index;
};

typedef ValueIndex::Set ValueIndexSet;
typedef std::vector<ValueIndex> ValueIndices;

//------------------------------------------------------------------------------

enum DependsKind {
    DK_Undefined = 0,
    DK_Unique = 1 << 0,
    DK_Viewed = 1 << 1,
    DK_Conflicted = DK_Unique | DK_Viewed,
};

struct Depends {
    std::vector<ValueIndexSet> args;
    std::vector<char> kinds;

    void ensure_arg(int index);
    void view(Value *value);
    void view(int index, ValueIndex value);
    void unique(Value *value);
    void unique(int index);
    bool empty() const;
    bool empty(int index) const;
};

//------------------------------------------------------------------------------

struct Value {
    ValueKind kind() const;

    Value(ValueKind _kind, const Anchor *_anchor);
    Value(const Value &other) = delete;

    const Anchor *anchor() const;

    bool is_typed() const;
    void set_type(const Type *type);
    const Type *get_type() const;
    void change_type(const Type *type);
    bool is_pure() const;
    bool is_accessible() const;
    int get_depth() const;
    void annotate(const String *msg);

    Depends deps;
    Strings annotations;
private:
    const ValueKind _kind;
    const Type *_type;

protected:
    const Anchor *_anchor;
};

//------------------------------------------------------------------------------

struct Block {
    Block();
    bool append(Value *node);
    bool empty() const;
    void migrate_from(Block &source);
    void clear();
    void set_parent(Block *parent);

    void insert_at(int index);
    void insert_at_end();

    void annotate(const String *msg);

    int depth;
    int insert_index;
    Instructions body;
    Instruction *terminator;
    Block *parent;
    Strings annotations;
};

//------------------------------------------------------------------------------

struct Instruction : Value {
    static bool classof(const Value *T);

    Instruction(ValueKind _kind, const Anchor *_anchor);

    Symbol name;
    Block *block;
};

//------------------------------------------------------------------------------

struct Keyed : Value {
    static bool classof(const Value *T);

    Keyed(const Anchor *anchor, Symbol key, Value *node);

    static Keyed *from(const Anchor *anchor, Symbol key, Value *node);

    Symbol key;
    Value *value;
};

//------------------------------------------------------------------------------

struct ArgumentList : Instruction {
    static bool classof(const Value *T);

    ArgumentList(const Anchor *anchor, const Values &values);

    void append(Symbol key, Value *node);
    void append(Value *node);

    bool is_constant() const;

    static ArgumentList *from(const Anchor *anchor, const Values &values = {});

    Values values;
};

//------------------------------------------------------------------------------

struct ExtractArgument : Instruction {
    static bool classof(const Value *T);

    ExtractArgument(const Anchor *anchor, Value *value, int index, bool vararg);

    static ExtractArgument *from(const Anchor *anchor, Value *value, int index,
        bool vararg = false);

    int index;
    Value *value;
    bool vararg;
};

//------------------------------------------------------------------------------

struct Pure : Value {
    static bool classof(const Value *T);

    Pure(ValueKind _kind, const Anchor *anchor);
};

//------------------------------------------------------------------------------

struct Template : Value {
    static bool classof(const Value *T);

    Template(const Anchor *anchor, Symbol name, const Parameters &params, Value *value);

    bool is_forward_decl() const;
    void set_inline();
    bool is_inline() const;
    void append_param(Parameter *sym);

    static Template *from(
        const Anchor *anchor, Symbol name,
        const Parameters &params = {}, Value *value = nullptr);

    Symbol name;
    Parameters params;
    Value *value;
    bool _inline;
    const String *docstring;
};

//------------------------------------------------------------------------------

struct Expression : Value {
    static bool classof(const Value *T);

    Expression(const Anchor *anchor, const Values &nodes, Value *value);
    void append(Value *node);

    static Expression *from(const Anchor *anchor, const Values &nodes = {}, Value *value = nullptr);
    static Expression *unscoped_from(const Anchor *anchor, const Values &nodes = {}, Value *value = nullptr);

    Values body;
    Value *value;
    bool scoped;
};

//------------------------------------------------------------------------------

struct CondBr : Instruction {
    static bool classof(const Value *T);

    CondBr(const Anchor *anchor, Value *cond);

    static CondBr *from(const Anchor *anchor, Value *cond = nullptr);

    Value *cond;
    Block then_body;
    Block else_body;
};

//------------------------------------------------------------------------------

struct If : Value {
    struct Clause {
        const Anchor *anchor;
        Value *cond;
        Value *value;

        Clause() : anchor(nullptr), cond(nullptr), value(nullptr) {}

        bool is_then() const;
    };

    typedef std::vector<Clause> Clauses;

    static bool classof(const Value *T);

    If(const Anchor *anchor, const Clauses &clauses);

    static If *from(const Anchor *anchor, const Clauses &clauses = {});

    void append_then(const Anchor *anchor, Value *cond, Value *value);
    void append_else(const Anchor *anchor, Value *value);

    Clauses clauses;
};

//------------------------------------------------------------------------------

enum CaseKind {
    CK_Case = 0,
    CK_Pass,
    CK_Default
};

struct Switch : Instruction {
    struct Case {
        CaseKind kind;
        const Anchor *anchor;
        Value *literal;
        Block body;
        Value *value;

        Case() : kind(CK_Case), anchor(nullptr), literal(nullptr), value(nullptr) {}
    };

    typedef std::vector<Case> Cases;

    static bool classof(const Value *T);

    Switch(const Anchor *anchor, Value *expr, const Cases &cases);

    static Switch *from(const Anchor *anchor, Value *expr = nullptr, const Cases &cases = {});

    void append_case(const Anchor *anchor, Value *literal, Value *value);
    void append_pass(const Anchor *anchor, Value *literal, Value *value);
    void append_default(const Anchor *anchor, Value *value);

    Value *expr;
    Cases cases;
};

//------------------------------------------------------------------------------

struct Parameter : Value {
    static bool classof(const Value *T);

    Parameter(const Anchor *anchor, Symbol name, const Type *type, bool variadic);
    static Parameter *from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);
    static Parameter *variadic_from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);

    bool is_variadic() const;
    void set_owner(Value *_owner, int _index);

    Symbol name;
    bool variadic;
    Value *owner;
    Block *block;
    int index;
};

//------------------------------------------------------------------------------

struct Exception : Value {
    static bool classof(const Value *T);

    Exception(const Anchor *anchor);
    static Exception *from(const Anchor *anchor);
};

//------------------------------------------------------------------------------

#define SCOPES_LABEL_KIND() \
    /* an user-created label */ \
    T(LK_User, "user") \
    /* the return label of an inline function */ \
    T(LK_Inline, "inline") \
    /* the try block of a try/except construct */ \
    T(LK_Try, "try") \
    /* the except block of a try/except construct */ \
    T(LK_Except, "except") \
    /* a break label of a loop */ \
    T(LK_Break, "break") \
    /* a merge label of a branch */ \
    T(LK_BranchMerge, "branch-merge") \

enum LabelKind {
#define T(NAME, BNAME) \
    NAME,
SCOPES_LABEL_KIND()
#undef T
};

struct Label : Instruction {
    static bool classof(const Value *T);

    Label(const Anchor *anchor, LabelKind kind, Symbol name);

    static Label *from(const Anchor *anchor,
        LabelKind kind,
        Symbol name = SYM_Unnamed);

    Symbol name;
    Block body;
    std::vector<Merge *> merges;
    LabelKind label_kind;
};

//------------------------------------------------------------------------------

struct LabelTemplate : Value {
    static bool classof(const Value *T);

    LabelTemplate(const Anchor *anchor, LabelKind kind, Symbol name, Value *value);

    static LabelTemplate *from(const Anchor *anchor,
        LabelKind kind,
        Symbol name = SYM_Unnamed,
        Value *value = nullptr);
    static LabelTemplate *try_from(const Anchor *anchor,
        Value *value = nullptr);
    static LabelTemplate *except_from(const Anchor *anchor,
        Value *value = nullptr);

    Symbol name;
    Value *value;
    LabelKind label_kind;
};

//------------------------------------------------------------------------------

enum CallFlags {
    CF_RawCall = (1 << 0),
};

struct Call : Instruction {
    static bool classof(const Value *T);

    Call(const Anchor *anchor, Value *callee, const Values &args);
    static Call *from(const Anchor *anchor, Value *callee, const Values &args = {});
    bool is_rawcall() const;
    void set_rawcall();

    Value *callee;
    Values args;
    uint32_t flags;
    Block except_body;
    Exception *except;
};

//------------------------------------------------------------------------------

struct LoopLabel : Instruction {
    static bool classof(const Value *T);

    LoopLabel(const Anchor *anchor, Value *init);

    static LoopLabel *from(const Anchor *anchor, Value *init = nullptr);

    Value *init;
    Block body;
    std::vector<Repeat *> repeats;
};

//------------------------------------------------------------------------------

struct Loop : Value {
    static bool classof(const Value *T);

    Loop(const Anchor *anchor, Value *init, Value *value);

    static Loop *from(const Anchor *anchor, Value *init = nullptr, Value *value = nullptr);

    Value *init;
    Value *value;
};

//------------------------------------------------------------------------------

struct Const : Pure {
    static bool classof(const Value *T);

    Const(ValueKind _kind, const Anchor *anchor, const Type *type);
};

//------------------------------------------------------------------------------

struct Function : Pure {
    static bool classof(const Value *T);

    Function(const Anchor *anchor, Symbol name, const Parameters &params);

    static Function *from(
        const Anchor *anchor, Symbol name,
        const Parameters &params);

    void append_param(Parameter *sym);

    Symbol name;
    Parameters params;
    Block body;
    const String *docstring;
    Function *frame;
    Function *boundary;
    Template *original;
    Label *label;
    bool complete;

    Types instance_args;
    void bind(Value *oldnode, Value *newnode);
    Value *unsafe_resolve(Value *node) const;
    Value *resolve_local(Value *node) const;
    SCOPES_RESULT(Value *) resolve(Value *node, Function *boundary) const;
    std::unordered_map<Value *, Value *> map;
    std::vector<Return *> returns;
    std::vector<Raise *> raises;
};

//------------------------------------------------------------------------------

struct ConstInt : Const {
    static bool classof(const Value *T);

    ConstInt(const Anchor *anchor, const Type *type, uint64_t value);

    static ConstInt *from(const Anchor *anchor, const Type *type, uint64_t value);
    static ConstInt *symbol_from(const Anchor *anchor, Symbol value);
    static ConstInt *builtin_from(const Anchor *anchor, Builtin value);

    uint64_t value;
};

//------------------------------------------------------------------------------

struct ConstReal : Const {
    static bool classof(const Value *T);

    ConstReal(const Anchor *anchor, const Type *type, double value);

    static ConstReal *from(const Anchor *anchor, const Type *type, double value);

    double value;
};

//------------------------------------------------------------------------------

struct ConstAggregate : Const {
    static bool classof(const Value *T);

    ConstAggregate(const Anchor *anchor, const Type *type, const Constants &fields);

    static ConstAggregate *from(const Anchor *anchor, const Type *type, const Constants &fields);
    static ConstAggregate *none_from(const Anchor *anchor);

    Constants values;
};

//------------------------------------------------------------------------------

struct ConstPointer : Const {
    static bool classof(const Value *T);

    ConstPointer(const Anchor *anchor, const Type *type, const void *pointer);

    static ConstPointer *from(const Anchor *anchor, const Type *type, const void *pointer);
    static ConstPointer *type_from(const Anchor *anchor, const Type *type);
    static ConstPointer *closure_from(const Anchor *anchor, const Closure *closure);
    static ConstPointer *string_from(const Anchor *anchor, const String *str);
    static ConstPointer *ast_from(const Anchor *anchor, Value *node);
    static ConstPointer *list_from(const Anchor *anchor, const List *list);
    static ConstPointer *scope_from(const Anchor *anchor, Scope *scope);
    static ConstPointer *anchor_from(const Anchor *anchor);

    const void *value;
};

//------------------------------------------------------------------------------

enum ExternFlags {
    // if storage class is 'Uniform, the value is a SSBO
    EF_BufferBlock = (1 << 0),
    EF_NonWritable = (1 << 1),
    EF_NonReadable = (1 << 2),
    EF_Volatile = (1 << 3),
    EF_Coherent = (1 << 4),
    EF_Restrict = (1 << 5),
    // if storage class is 'Uniform, the value is a UBO
    EF_Block = (1 << 6),
};

struct Extern : Pure {
    static bool classof(const Value *T);

    Extern(const Anchor *anchor, const Type *type, Symbol name,
        size_t flags, Symbol storage_class, int location, int binding);

    static Extern *from(const Anchor *anchor, const Type *type, Symbol name,
        size_t flags = 0,
        Symbol storage_class = SYM_Unnamed,
        int location = -1, int binding = -1);

    Symbol name;
    size_t flags;
    Symbol storage_class;
    int location;
    int binding;
};

//------------------------------------------------------------------------------

struct Break : Value {
    static bool classof(const Value *T);

    Break(const Anchor *anchor, Value *value);

    static Break *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Repeat : Instruction {
    static bool classof(const Value *T);

    Repeat(const Anchor *anchor, Value *value);

    static Repeat *from(const Anchor *anchor, Value *value);

    Value *value;
    LoopLabel *loop;
};

//------------------------------------------------------------------------------

struct Return : Instruction {
    static bool classof(const Value *T);

    Return(const Anchor *anchor, Value *value);

    static Return *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Merge : Instruction {
    static bool classof(const Value *T);

    Merge(const Anchor *anchor, Label *label, Value *value);

    static Merge *from(const Anchor *anchor, Label *label, Value *value);

    Label *label;
    Value *value;
};

//------------------------------------------------------------------------------

struct MergeTemplate : Value {
    static bool classof(const Value *T);

    MergeTemplate(const Anchor *anchor, LabelTemplate *label, Value *value);

    static MergeTemplate *from(const Anchor *anchor, LabelTemplate *label, Value *value);

    LabelTemplate *label;
    Value *value;
};

//------------------------------------------------------------------------------

struct Raise : Instruction {
    static bool classof(const Value *T);

    Raise(const Anchor *anchor, Value *value);

    static Raise *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Quote : Value {
    static bool classof(const Value *T);

    Quote(const Anchor *anchor, Value *value);

    static Quote *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Unquote : Value {
    static bool classof(const Value *T);

    Unquote(const Anchor *anchor, Value *value);

    static Unquote *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct CompileStage : Value {
    static bool classof(const Value *T);

    CompileStage(const Anchor *anchor, const List *next, Scope *env);

    static CompileStage *from(const Anchor *anchor, const List *next, Scope *env);

    const List *next;
    Scope *env;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_VALUE_HPP

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

#define SCOPES_UNTYPED_VALUE_KIND() \
    T(VK_Template, "value-kind-template", Template) \
    T(VK_LabelTemplate, "value-kind-label-template", LabelTemplate) \
    T(VK_Loop, "value-kind-loop", Loop) \
    T(VK_LoopArguments, "value-kind-loop-arguments", LoopArguments) \
    T(VK_KeyedTemplate, "value-kind-keyed-template", KeyedTemplate) \
    T(VK_Expression, "value-kind-expression", Expression) \
    T(VK_Quote, "value-kind-quote", Quote) \
    T(VK_Unquote, "value-kind-unquote", Unquote) \
    T(VK_CompileStage, "value-kind-compile-stage", CompileStage) \
    T(VK_If, "value-kind-if", If) \
    T(VK_SwitchTemplate, "value-kind-switch-template", SwitchTemplate) \
    T(VK_MergeTemplate, "value-kind-merge-template", MergeTemplate) \
    T(VK_CallTemplate, "value-kind-call-template", CallTemplate) \
    T(VK_RepeatTemplate, "value-kind-repeat-template", RepeatTemplate) \
    T(VK_ReturnTemplate, "value-kind-return-template", ReturnTemplate) \
    T(VK_RaiseTemplate, "value-kind-raise-template", RaiseTemplate) \
    T(VK_Break, "value-kind-break", Break) \
    T(VK_ArgumentListTemplate, "value-kind-argument-list-template", ArgumentListTemplate) \
    T(VK_ExtractArgumentTemplate, "value-kind-extract-argument-template", ExtractArgumentTemplate) \
    T(VK_ParameterTemplate, "value-kind-parameter-template", ParameterTemplate) \


#define SCOPES_CONST_VALUE_KIND() \
    T(VK_ConstInt, "value-kind-const-int", ConstInt) \
    T(VK_ConstReal, "value-kind-const-real", ConstReal) \
    T(VK_ConstAggregate, "value-kind-const-aggregate", ConstAggregate) \
    T(VK_ConstPointer, "value-kind-const-pointer", ConstPointer) \


#define SCOPES_PURE_VALUE_KIND() \
    T(VK_Function, "value-kind-function", Function) \
    T(VK_Extern, "value-kind-extern", Extern) \
    /* constants (Const::classof) */ \
    SCOPES_CONST_VALUE_KIND() \


#define SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Merge, "value-kind-merge", Merge) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \


#define SCOPES_INSTRUCTION_VALUE_KIND() \
    SCOPES_TERMINATOR_VALUE_KIND() \
    T(VK_Label, "value-kind-label", Label) \
    T(VK_LoopLabel, "value-kind-loop-label", LoopLabel) \
    T(VK_CondBr, "value-kind-condbr", CondBr) \
    T(VK_Switch, "value-kind-switch", Switch) \
    T(VK_Call, "value-kind-call", Call) \


#define SCOPES_TYPED_VALUE_KIND() \
    T(VK_Keyed, "value-kind-keyed", Keyed) \
    T(VK_Parameter, "value-kind-parameter", Parameter) \
    T(VK_Exception, "value-kind-exception", Exception) \
    T(VK_ArgumentList, "value-kind-argument-list", ArgumentList) \
    T(VK_ExtractArgument, "value-kind-extract-argument", ExtractArgument) \
    T(VK_LoopLabelArguments, "value-kind-loop-label-arguments", LoopLabelArguments) \
    /* instructions (Instruction::classof) */ \
    SCOPES_INSTRUCTION_VALUE_KIND() \
    /* pure (Pure::classof), which includes constants */ \
    SCOPES_PURE_VALUE_KIND() \


#define SCOPES_VALUE_KIND() \
    SCOPES_UNTYPED_VALUE_KIND() \
    SCOPES_TYPED_VALUE_KIND() \


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
struct TypedValue;
struct Block;
struct Instruction;
struct Const;
struct Raise;
struct Parameter;
struct ParameterTemplate;

typedef std::vector<Parameter *> Parameters;
typedef std::vector<ParameterTemplate *> ParameterTemplates;
typedef std::vector<Value *> Values;
typedef std::vector<TypedValue *> TypedValues;
typedef std::unordered_set<Value *> ValueSet;
typedef std::unordered_set<TypedValue *> TypedValueSet;
typedef std::vector<ValueSet> ValueSetArray;
typedef std::vector<TypedValueSet> TypedValueSetArray;
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

    ValueIndex(TypedValue *_value, int _index = 0);
    bool operator ==(const ValueIndex &other) const;

    const Type *get_type() const;
    const Set *deps() const;
    bool has_deps() const;

    TypedValue *value;
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
    void view(TypedValue *value);
    void view(int index, ValueIndex value);
    void unique(TypedValue *value);
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

    bool is_accessible() const;
    int get_depth() const;

private:
    const ValueKind _kind;

protected:
    const Anchor *_anchor;
};

//------------------------------------------------------------------------------

struct UntypedValue : Value {
    static bool classof(const Value *T);

    UntypedValue(ValueKind _kind, const Anchor *_anchor);
};

//------------------------------------------------------------------------------

struct TypedValue : Value {
    static bool classof(const Value *T);

    TypedValue(ValueKind _kind, const Anchor *_anchor, const Type *type);

    //bool is_typed() const;
    const Type *get_type() const;
protected:
    const Type *_type;
};

//------------------------------------------------------------------------------

struct Block {
    Block();
    void append(TypedValue *node);
    bool empty() const;
    void migrate_from(Block &source);
    void clear();
    void set_parent(Block *parent);

    void insert_at(int index);
    void insert_at_end();

    int depth;
    int insert_index;
    Instructions body;
    Instruction *terminator;
    Block *parent;
};

//------------------------------------------------------------------------------

struct Instruction : TypedValue {
    static bool classof(const Value *T);

    Instruction(ValueKind _kind, const Anchor *_anchor, const Type *type);

    Symbol name;
    Block *block;

    Depends deps;
};

//------------------------------------------------------------------------------

struct Terminator : Instruction {
    static bool classof(const Value *T);

    Terminator(ValueKind _kind, const Anchor *_anchor);
};

//------------------------------------------------------------------------------

struct KeyedTemplate : UntypedValue {
    static bool classof(const Value *T);

    KeyedTemplate(const Anchor *anchor, Symbol key, Value *node);

    static Value *from(const Anchor *anchor, Symbol key, Value *node);

    Symbol key;
    Value *value;
};

//------------------------------------------------------------------------------

struct Keyed : TypedValue {
    static bool classof(const Value *T);

    Keyed(const Anchor *anchor, const Type *type, Symbol key, TypedValue *node);

    static TypedValue *from(const Anchor *anchor, Symbol key, TypedValue *node);

    Symbol key;
    TypedValue *value;
};

//------------------------------------------------------------------------------

struct ArgumentListTemplate : UntypedValue {
    static bool classof(const Value *T);

    ArgumentListTemplate(const Anchor *anchor, const Values &values);

    void append(Symbol key, Value *node);
    void append(Value *node);

    bool is_constant() const;

    static Value *empty_from(const Anchor *anchor);
    static Value *from(const Anchor *anchor, const Values &values = {});

    Values values;
};

//------------------------------------------------------------------------------

struct ArgumentList : TypedValue {
    static bool classof(const Value *T);

    ArgumentList(const Anchor *anchor, const TypedValues &values);

    bool is_constant() const;

    static TypedValue *from(const Anchor *anchor, const TypedValues &values);

    TypedValues values;
};

//------------------------------------------------------------------------------

struct ExtractArgumentTemplate : UntypedValue {
    static bool classof(const Value *T);

    ExtractArgumentTemplate(const Anchor *anchor, Value *value, int index, bool vararg);

    static Value *from(const Anchor *anchor, Value *value, int index,
        bool vararg = false);

    int index;
    Value *value;
    bool vararg;
};

//------------------------------------------------------------------------------

struct ExtractArgument : TypedValue {
    static bool classof(const Value *T);

    ExtractArgument(const Anchor *anchor, const Type *type, TypedValue *value, int index);

    static TypedValue *variadic_from(const Anchor *anchor, TypedValue *value, int index);
    static TypedValue *from(const Anchor *anchor, TypedValue *value, int index);

    int index;
    TypedValue *value;
};

//------------------------------------------------------------------------------

struct Pure : TypedValue {
    static bool classof(const Value *T);

    Pure(ValueKind _kind, const Anchor *anchor, const Type *type);
};

//------------------------------------------------------------------------------

struct Template : UntypedValue {
    static bool classof(const Value *T);

    Template(const Anchor *anchor, Symbol name, const ParameterTemplates &params, Value *value);

    bool is_forward_decl() const;
    void set_inline();
    bool is_inline() const;
    void append_param(ParameterTemplate *sym);

    static Template *from(
        const Anchor *anchor, Symbol name,
        const ParameterTemplates &params = {}, Value *value = nullptr);

    Symbol name;
    ParameterTemplates params;
    Value *value;
    bool _inline;
    const String *docstring;
};

//------------------------------------------------------------------------------

struct Expression : UntypedValue {
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

    CondBr(const Anchor *anchor, TypedValue *cond);

    static CondBr *from(const Anchor *anchor, TypedValue *cond);

    TypedValue *cond;
    Block then_body;
    Block else_body;
};

//------------------------------------------------------------------------------

struct If : UntypedValue {
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

struct SwitchTemplate : UntypedValue {
    struct Case {
        CaseKind kind;
        const Anchor *anchor;
        Value *literal;
        Value *value;

        Case() : kind(CK_Case), anchor(nullptr), literal(nullptr), value(nullptr) {}
    };

    typedef std::vector<Case> Cases;

    static bool classof(const Value *T);

    SwitchTemplate(const Anchor *anchor, Value *expr, const Cases &cases);

    static SwitchTemplate *from(const Anchor *anchor, Value *expr = nullptr, const Cases &cases = {});

    void append_case(const Anchor *anchor, Value *literal, Value *value);
    void append_pass(const Anchor *anchor, Value *literal, Value *value);
    void append_default(const Anchor *anchor, Value *value);

    Value *expr;
    Cases cases;
};

//------------------------------------------------------------------------------

struct Switch : Instruction {
    struct Case {
        CaseKind kind;
        const Anchor *anchor;
        ConstInt *literal;
        Block body;

        Case() : kind(CK_Case), anchor(nullptr), literal(nullptr) {}
    };

    typedef std::vector<Case> Cases;

    static bool classof(const Value *T);

    Switch(const Anchor *anchor, TypedValue *expr, const Cases &cases);

    static Switch *from(const Anchor *anchor, TypedValue *expr = nullptr, const Cases &cases = {});

    Case &append_pass(const Anchor *anchor, ConstInt *literal);
    Case &append_default(const Anchor *anchor);

    TypedValue *expr;
    Cases cases;
};

//------------------------------------------------------------------------------

struct ParameterTemplate : UntypedValue {
    static bool classof(const Value *T);

    ParameterTemplate(const Anchor *anchor, Symbol name, bool variadic);
    static ParameterTemplate *from(const Anchor *anchor, Symbol name = SYM_Unnamed);
    static ParameterTemplate *variadic_from(const Anchor *anchor, Symbol name = SYM_Unnamed);

    bool is_variadic() const;
    void set_owner(Template *_owner, int _index);

    Symbol name;
    bool variadic;
    Template *owner;
    int index;
};

//------------------------------------------------------------------------------

struct Parameter : TypedValue {
    static bool classof(const Value *T);

    Parameter(const Anchor *anchor, Symbol name, const Type *type);
    static Parameter *from(const Anchor *anchor, Symbol name, const Type *type);

    void set_owner(Function *_owner, int _index);

    Symbol name;
    Function *owner;
    Block *block;
    int index;
};

//------------------------------------------------------------------------------

struct LoopArguments : UntypedValue {
    static bool classof(const Value *T);

    LoopArguments(const Anchor *anchor, Loop *loop);
    static LoopArguments *from(const Anchor *anchor, Loop *loop);

    Loop *loop;
};

//------------------------------------------------------------------------------

struct LoopLabelArguments : TypedValue {
    static bool classof(const Value *T);

    LoopLabelArguments(const Anchor *anchor, const Type *type, LoopLabel *loop);
    static LoopLabelArguments *from(const Anchor *anchor, const Type *type, LoopLabel *loop);

    LoopLabel *loop;
};

//------------------------------------------------------------------------------

struct Exception : TypedValue {
    static bool classof(const Value *T);

    Exception(const Anchor *anchor, const Type *type);
    static Exception *from(const Anchor *anchor, const Type *type);
};

//------------------------------------------------------------------------------

#define SCOPES_LABEL_KIND() \
    /* an user-created label */ \
    T(LK_User, "merge") \
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
    void change_type(const Type *type);

    Symbol name;
    Block body;
    std::vector<Merge *> merges;
    LabelKind label_kind;
};

const char *get_label_kind_name(LabelKind kind);

//------------------------------------------------------------------------------

struct LabelTemplate : UntypedValue {
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

struct CallTemplate : UntypedValue {
    static bool classof(const Value *T);

    CallTemplate(const Anchor *anchor, Value *callee, const Values &args);
    static CallTemplate *from(const Anchor *anchor, Value *callee, const Values &args = {});
    bool is_rawcall() const;
    void set_rawcall();

    Value *callee;
    Values args;
    uint32_t flags;
};

//------------------------------------------------------------------------------

struct Call : Instruction {
    static bool classof(const Value *T);

    Call(const Anchor *anchor, const Type *type, TypedValue *callee, const TypedValues &args);
    static Call *from(const Anchor *anchor, const Type *type, TypedValue *callee, const TypedValues &args = {});

    TypedValue *callee;
    TypedValues args;
    Block except_body;
    Exception *except;
};

//------------------------------------------------------------------------------

struct LoopLabel : Instruction {
    static bool classof(const Value *T);

    LoopLabel(const Anchor *anchor, TypedValue *init);

    static LoopLabel *from(const Anchor *anchor, TypedValue *init);

    TypedValue *init;
    Block body;
    std::vector<Repeat *> repeats;
    LoopLabelArguments *args;
};

//------------------------------------------------------------------------------

struct Loop : UntypedValue {
    static bool classof(const Value *T);

    Loop(const Anchor *anchor, Value *init, Value *value);

    static Loop *from(const Anchor *anchor, Value *init = nullptr, Value *value = nullptr);

    Value *init;
    Value *value;
    LoopArguments *args;
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
    void change_type(const Type *type);
    void set_type(const Type *type);
    bool is_typed() const;

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
    void bind(Value *oldnode, TypedValue *newnode);
    TypedValue *unsafe_resolve(Value *node) const;
    TypedValue *resolve_local(Value *node) const;
    SCOPES_RESULT(TypedValue *) resolve(Value *node, Function *boundary) const;
    std::unordered_map<Value *, TypedValue *> map;
    std::vector<Return *> returns;
    std::vector<Raise *> raises;

    Depends deps;
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

struct Break : UntypedValue {
    static bool classof(const Value *T);

    Break(const Anchor *anchor, Value *value);

    static Break *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Merge : Terminator {
    static bool classof(const Value *T);

    Merge(const Anchor *anchor, Label *label, TypedValue *value);

    static Merge *from(const Anchor *anchor, Label *label, TypedValue *value);

    Label *label;
    TypedValue *value;
};

//------------------------------------------------------------------------------

struct MergeTemplate : UntypedValue {
    static bool classof(const Value *T);

    MergeTemplate(const Anchor *anchor, LabelTemplate *label, Value *value);

    static MergeTemplate *from(const Anchor *anchor, LabelTemplate *label, Value *value);

    LabelTemplate *label;
    Value *value;
};

//------------------------------------------------------------------------------

struct RepeatTemplate : UntypedValue {
    static bool classof(const Value *T);

    RepeatTemplate(const Anchor *anchor, Value *value);

    static RepeatTemplate *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Repeat : Terminator {
    static bool classof(const Value *T);

    Repeat(const Anchor *anchor, TypedValue *value, LoopLabel *loop);

    static Repeat *from(const Anchor *anchor, TypedValue *value, LoopLabel *loop);

    TypedValue *value;
    LoopLabel *loop;
};

//------------------------------------------------------------------------------

struct ReturnTemplate : UntypedValue {
    static bool classof(const Value *T);

    ReturnTemplate(const Anchor *anchor, Value *value);

    static ReturnTemplate *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Return : Terminator {
    static bool classof(const Value *T);

    Return(const Anchor *anchor, TypedValue *value);

    static Return *from(const Anchor *anchor, TypedValue *value);

    TypedValue *value;
};

//------------------------------------------------------------------------------

struct RaiseTemplate : UntypedValue {
    static bool classof(const Value *T);

    RaiseTemplate(const Anchor *anchor, Value *value);

    static RaiseTemplate *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Raise : Terminator {
    static bool classof(const Value *T);

    Raise(const Anchor *anchor, TypedValue *value);

    static Raise *from(const Anchor *anchor, TypedValue *value);

    TypedValue *value;
};

//------------------------------------------------------------------------------

struct Quote : UntypedValue {
    static bool classof(const Value *T);

    Quote(const Anchor *anchor, Value *value);

    static Quote *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Unquote : UntypedValue {
    static bool classof(const Value *T);

    Unquote(const Anchor *anchor, Value *value);

    static Unquote *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct CompileStage : UntypedValue {
    static bool classof(const Value *T);

    CompileStage(const Anchor *anchor, const List *next, Scope *env);

    static CompileStage *from(const Anchor *anchor, const List *next, Scope *env);

    const List *next;
    Scope *env;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_VALUE_HPP

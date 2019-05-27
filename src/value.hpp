/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VALUE_HPP
#define SCOPES_VALUE_HPP

#include "symbol.hpp"
#include "result.hpp"
#include "builtin.hpp"
#include "type.hpp"
#include "value_kind.hpp"
#include "valueref.inc"

#include "qualifier/unique_qualifiers.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace scopes {

// untyped, instructions, functions
#define SCOPES_DEFINITION_ANCHOR_API() \
    const Anchor *def_anchor() const; \
    void set_def_anchor(const Anchor *anchor); \
protected: const Anchor *_def_anchor = nullptr; \
public: \


struct Anchor;
struct List;
struct Scope;
struct Block;

typedef std::vector<ParameterRef> Parameters;
typedef std::vector<ParameterTemplateRef> ParameterTemplates;
typedef std::vector<ValueRef> Values;
typedef std::vector<TypedValueRef> TypedValues;
typedef std::vector<InstructionRef> Instructions;
typedef std::vector<ConstRef> Constants;
typedef std::vector<Const *> ConstantPtrs;
typedef std::vector<MergeRef> Merges;
typedef std::vector<RepeatRef> Repeats;
typedef std::vector<ReturnRef> Returns;
typedef std::vector<RaiseRef> Raises;
typedef std::vector<Block *> Blocks;

const char *get_value_kind_name(ValueKind kind);
const char *get_value_class_name(ValueKind kind);

//------------------------------------------------------------------------------

struct ValueIndex {
    struct Hash {
        std::size_t operator()(const ValueIndex & s) const;
    };
    typedef std::unordered_set<ValueIndex, ValueIndex::Hash> Set;

    ValueIndex(const TypedValueRef &_value, int _index = 0);
    bool operator ==(const ValueIndex &other) const;

    const Type *get_type() const;

    TypedValueRef value;
    int index;
};

typedef ValueIndex::Set ValueIndexSet;
typedef std::vector<ValueIndex> ValueIndices;

//------------------------------------------------------------------------------

struct Value {
    ValueKind kind() const;

    Value(ValueKind _kind);
    Value(const Value &other) = delete;

    bool is_accessible() const;
    int get_depth() const;

private:
    const ValueKind _kind;
};

const Anchor *get_best_anchor(const ValueRef &value);
void set_best_anchor(const ValueRef &value, const Anchor *anchor);

//------------------------------------------------------------------------------

struct UntypedValue : Value {
    static bool classof(const Value *T);

    UntypedValue(ValueKind _kind);
};

//------------------------------------------------------------------------------

struct TypedValue : Value {
    static bool classof(const Value *T);

    TypedValue(ValueKind _kind, const Type *type);

    //bool is_typed() const;
    const Type *get_type() const;

protected:
    const Type *_type;
};

//------------------------------------------------------------------------------

struct Block {
    Block();
    int append(const TypedValueRef &node);
    bool empty() const;
    void migrate_from(Block &source);
    void clear();
    void set_parent(Block *parent);

    bool is_terminated() const;

    void insert_at(int index);
    void insert_at_end();

    bool is_valid(int id) const;
    bool is_valid(const IDSet &ids, int &id) const;
    bool is_valid(const ValueIndex &value, int &id) const;
    bool is_valid(const IDSet &ids) const;
    bool is_valid(const ValueIndex &value) const;
    void move(int id);

    int depth;
    int insert_index;
    bool tag_traceback;
    Instructions body;
    InstructionRef terminator;
    // set of unique ids that are still valid in this scope
    IDSet valid;
};

//------------------------------------------------------------------------------

struct Instruction : TypedValue {
    static bool classof(const Value *T);

    Instruction(ValueKind _kind, const Type *type);

    Symbol name;
    Block *block;
};

void validate_instruction(const TypedValueRef &value);
void validate_instructions(const TypedValues &values);

//------------------------------------------------------------------------------

struct Terminator : Instruction {
    static bool classof(const Value *T);

    Terminator(ValueKind _kind, const TypedValues &_values);

    TypedValues values;
};

//------------------------------------------------------------------------------

struct KeyedTemplate : UntypedValue {
    static bool classof(const Value *T);

    KeyedTemplate(Symbol key, const ValueRef &node);

    static ValueRef from(Symbol key, ValueRef node);

    Symbol key;
    ValueRef value;
};

//------------------------------------------------------------------------------

struct Keyed : TypedValue {
    static bool classof(const Value *T);

    Keyed(const Type *type, Symbol key, const TypedValueRef &node);

    static TypedValueRef from(Symbol key, TypedValueRef node);

    Symbol key;
    TypedValueRef value;
};

//------------------------------------------------------------------------------

struct ArgumentListTemplate : UntypedValue {
    static bool classof(const Value *T);

    ArgumentListTemplate(const Values &values);

    //bool is_constant() const;
    bool empty() const;

    //static ValueRef empty_from();
    static ValueRef from(const Values &values = {});

    const Values &values() const;

protected:
    Values _values;
};

//------------------------------------------------------------------------------

const Type *arguments_type_from_typed_values(const TypedValues &_values);

struct ArgumentList : TypedValue {
    static bool classof(const Value *T);

    ArgumentList(const TypedValues &values);

    bool is_constant() const;

    static TypedValueRef from(const TypedValues &values);

    TypedValues values;
};

//------------------------------------------------------------------------------

struct ExtractArgumentTemplate : UntypedValue {
    static bool classof(const Value *T);

    ExtractArgumentTemplate(const ValueRef &value, int index, bool vararg);

    static ValueRef from(const ValueRef &value, int index, bool vararg = false);

    int index;
    ValueRef value;
    bool vararg;
};

//------------------------------------------------------------------------------

struct ExtractArgument : TypedValue {
    static bool classof(const Value *T);

    ExtractArgument(const Type *type, const TypedValueRef &value, int index);

    static TypedValueRef variadic_from(const TypedValueRef &value, int index);
    static TypedValueRef from(const TypedValueRef &value, int index);

    int index;
    TypedValueRef value;
};

//------------------------------------------------------------------------------

struct Pure : TypedValue {
    static bool classof(const Value *T);

    bool key_equal(const Pure *other) const;
    std::size_t hash() const;

    Pure(ValueKind _kind, const Type *type);
};

//------------------------------------------------------------------------------

struct Template : UntypedValue {
    static bool classof(const Value *T);

    Template(Symbol name, const ParameterTemplates &params, const ValueRef &value);

    bool is_forward_decl() const;
    void set_inline();
    bool is_inline() const;
    void append_param(const ParameterTemplateRef &sym);
    bool is_hidden() const;

    static TemplateRef from(Symbol name, const ParameterTemplates &params = {},
        const ValueRef &value = ValueRef());

    Symbol name;
    ParameterTemplates params;
    ValueRef value;
    bool _is_inline;
    const String *docstring;
    int recursion;
};

//------------------------------------------------------------------------------

struct Expression : UntypedValue {
    static bool classof(const Value *T);

    Expression(const Values &nodes, const ValueRef &value);
    void append(const ValueRef &node);

    static ExpressionRef scoped_from(const Values &nodes = {}, const ValueRef &value = ValueRef());
    static ExpressionRef unscoped_from(const Values &nodes = {}, const ValueRef &value = ValueRef());

    Values body;
    ValueRef value;
    bool scoped;
};

//------------------------------------------------------------------------------

struct CondBr : Instruction {
    static bool classof(const Value *T);

    CondBr(const TypedValueRef &cond);

    static CondBrRef from(const TypedValueRef &cond);

    TypedValueRef cond;
    Block then_body;
    Block else_body;
};

//------------------------------------------------------------------------------

struct If : UntypedValue {
    struct Clause {
        ValueRef cond;
        ValueRef value;

        Clause() {}

        bool is_then() const;
    };

    typedef std::vector<Clause> Clauses;

    static bool classof(const Value *T);

    If(const Clauses &clauses);

    static IfRef from(const Clauses &clauses = {});

    void append_then(const ValueRef &cond, const ValueRef &value);
    void append_else(const ValueRef &value);

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
        ValueRef literal;
        ValueRef value;

        Case() : kind(CK_Case) {}
    };

    typedef std::vector<Case> Cases;

    static bool classof(const Value *T);

    SwitchTemplate(const ValueRef &expr, const Cases &cases);

    static SwitchTemplateRef from(const ValueRef &expr = ValueRef(), const Cases &cases = {});

    void append_case(const ValueRef &literal, const ValueRef &value);
    void append_pass(const ValueRef &literal, const ValueRef &value);
    void append_default(const ValueRef &value);

    ValueRef expr;
    Cases cases;
};

//------------------------------------------------------------------------------

struct Switch : Instruction {
    struct Case {
        CaseKind kind;
        ConstIntRef literal;
        Block body;

        Case() : kind(CK_Case) {}
    };

    typedef std::vector<Case *> Cases;

    static bool classof(const Value *T);

    Switch(const TypedValueRef &expr, const Cases &cases);

    static SwitchRef from(const TypedValueRef &expr = TypedValueRef(), const Cases &cases = {});

    Case &append_pass(const ConstIntRef &literal);
    Case &append_default();

    TypedValueRef expr;
    Cases cases;
};

//------------------------------------------------------------------------------

struct ParameterTemplate : UntypedValue {
    static bool classof(const Value *T);

    ParameterTemplate(Symbol name, bool variadic);
    static ParameterTemplateRef from(Symbol name = SYM_Unnamed);
    static ParameterTemplateRef variadic_from(Symbol name = SYM_Unnamed);

    bool is_variadic() const;
    void set_owner(const TemplateRef &_owner, int _index);

    Symbol name;
    bool variadic;
    TemplateRef owner;
    int index;
};

//------------------------------------------------------------------------------

struct Parameter : TypedValue {
    static bool classof(const Value *T);

    Parameter(Symbol name, const Type *type);
    static ParameterRef from(Symbol name, const Type *type);

    void set_owner(const FunctionRef &_owner, int _index);

    void retype(const Type *T);

    Symbol name;
    FunctionRef owner;
    Block *block;
    int index;
};

//------------------------------------------------------------------------------

struct LoopArguments : UntypedValue {
    static bool classof(const Value *T);

    LoopArguments(const LoopRef &loop);
    static LoopArgumentsRef from(const LoopRef &loop);

    LoopRef loop;
};

//------------------------------------------------------------------------------

struct LoopLabelArguments : TypedValue {
    static bool classof(const Value *T);

    LoopLabelArguments(const Type *type);
    static LoopLabelArgumentsRef from(const Type *type);

    LoopLabelRef loop;
};

//------------------------------------------------------------------------------

struct Exception : TypedValue {
    static bool classof(const Value *T);

    Exception(const Type *type);
    static ExceptionRef from(const Type *type);
};

//------------------------------------------------------------------------------

#define SCOPES_LABEL_KIND() \
    /* an user-created label */ \
    T(LK_User, "label") \
    /* the return label of an inline function */ \
    T(LK_Inline, "inline") \
    /* the try block of a try/except construct */ \
    T(LK_Try, "try") \
    /* the except block of a try/except construct */ \
    T(LK_Except, "except") \
    /* a break label of a loop */ \
    T(LK_Break, "break") \
    /* a merge label of a branch */ \
    T(LK_BranchMerge, "branch") \

enum LabelKind {
#define T(NAME, BNAME) \
    NAME,
SCOPES_LABEL_KIND()
#undef T
};

struct Label : Instruction {
    static bool classof(const Value *T);

    Label(LabelKind kind, Symbol name);

    static LabelRef from(LabelKind kind, Symbol name = SYM_Unnamed);
    void change_type(const Type *type);

    Symbol name;
    Block body;
    Merges merges;
    LabelKind label_kind;
};

const char *get_label_kind_name(LabelKind kind);

//------------------------------------------------------------------------------

struct LabelTemplate : UntypedValue {
    static bool classof(const Value *T);

    LabelTemplate(LabelKind kind, Symbol name, const ValueRef &value);

    static LabelTemplateRef from(LabelKind kind, Symbol name = SYM_Unnamed,
        const ValueRef &value = ValueRef());
    static LabelTemplateRef try_from(const ValueRef &value = ValueRef());
    static LabelTemplateRef except_from(const ValueRef &value = ValueRef());

    Symbol name;
    ValueRef value;
    LabelKind label_kind;
};

//------------------------------------------------------------------------------

enum CallFlags {
    CF_RawCall = (1 << 0),
};

struct CallTemplate : UntypedValue {
    static bool classof(const Value *T);

    CallTemplate(const ValueRef &callee, const Values &args);
    static CallTemplateRef from(const ValueRef &callee, const Values &args = {});
    bool is_rawcall() const;
    void set_rawcall();

    ValueRef callee;
    Values args;
    uint32_t flags;
};

//------------------------------------------------------------------------------

struct Call : Instruction {
    static bool classof(const Value *T);

    Call(const Type *type, const TypedValueRef &callee, const TypedValues &args);
    static CallRef from(const Type *type, const TypedValueRef &callee, const TypedValues &args = {});

    TypedValueRef callee;
    TypedValues args;
    Block except_body;
    ExceptionRef except;
};

//------------------------------------------------------------------------------

struct LoopLabel : Instruction {
    static bool classof(const Value *T);

    LoopLabel(const TypedValues &init, const LoopLabelArgumentsRef &args);

    static LoopLabelRef from(const TypedValues &init,
        const LoopLabelArgumentsRef &args);

    TypedValues init;
    Block body;
    Repeats repeats;
    LoopLabelArgumentsRef args;
};

//------------------------------------------------------------------------------

struct Loop : UntypedValue {
    static bool classof(const Value *T);

    Loop(const ValueRef &init, const ValueRef &value);

    static LoopRef from(const ValueRef &init = ValueRef(),
        const ValueRef &value = ValueRef());

    ValueRef init;
    ValueRef value;
    LoopArgumentsRef args;
};

//------------------------------------------------------------------------------

struct Const : Pure {
    static bool classof(const Value *T);

    Const(ValueKind _kind, const Type *type);
};

//------------------------------------------------------------------------------

struct Function : Pure {
    struct UniqueInfo {
        ValueIndex value;
        // at which block depth is the unique defined?
        int get_depth() const;

        UniqueInfo(const ValueIndex& value);
    };
    // map of known uniques within the function (any state)
    typedef std::unordered_map<int, UniqueInfo> UniqueMap;

    static bool classof(const Value *T);

    Function(Symbol name, const Parameters &params);

    static FunctionRef from(Symbol name,
        const Parameters &params);

    void append_param(const ParameterRef &sym);
    void change_type(const Type *type);
    void set_type(const Type *type);
    bool is_typed() const;

    bool key_equal(const Function *other) const;
    std::size_t hash() const;

    int unique_id();
    void bind_unique(const UniqueInfo &info);
    void try_bind_unique(const TypedValueRef &value);
    const UniqueInfo &get_unique_info(int id) const;
    void build_valids();

    Symbol name;
    Parameters params;
    Block body;
    const String *docstring;
    FunctionRef frame;
    FunctionRef boundary;
    TemplateRef original;
    LabelRef label;
    bool complete;
    int nextid;
    const Type *returning_hint;
    const Type *raising_hint;
    const Anchor *returning_anchor;
    const Anchor *raising_anchor;

    Types instance_args;
    void bind(const ValueRef &oldnode, const TypedValueRef &newnode);
    TypedValueRef unsafe_resolve(const ValueRef &node) const;
    TypedValueRef resolve_local(const ValueRef &node) const;
    SCOPES_RESULT(TypedValueRef) resolve(const ValueRef &node,
        const FunctionRef &boundary) const;
    std::unordered_map<Value *, TypedValueRef> map;
    Returns returns;
    Raises raises;

    UniqueMap uniques;
    IDSet original_valid;
    IDSet valid;
    // expressions that moved a unique
    std::unordered_map<int, ValueRef> movers;

    const Anchor *get_best_mover_anchor(int id);
    void hint_mover(int id, const ValueRef &where);
};

//------------------------------------------------------------------------------

struct ConstInt : Const {
    static bool classof(const Value *T);

    ConstInt(const Type *type, uint64_t value);

    bool key_equal(const ConstInt *other) const;
    std::size_t hash() const;

    static ConstIntRef from(const Type *type, uint64_t value);
    static ConstIntRef symbol_from(Symbol value);
    static ConstIntRef builtin_from(Builtin value);

    uint64_t value;
};

//------------------------------------------------------------------------------

struct ConstReal : Const {
    static bool classof(const Value *T);

    ConstReal(const Type *type, double value);

    bool key_equal(const ConstReal *other) const;
    std::size_t hash() const;

    static ConstRealRef from(const Type *type, double value);

    double value;
};

//------------------------------------------------------------------------------

struct ConstAggregate : Const {
    static bool classof(const Value *T);

    ConstAggregate(const Type *type, const ConstantPtrs &fields);

    bool key_equal(const ConstAggregate *other) const;
    std::size_t hash() const;

    static ConstAggregateRef from(const Type *type, const ConstantPtrs &fields);
    static ConstAggregateRef none_from();
    static ConstAggregateRef ast_from(const ValueRef &node);

    ConstantPtrs values;
    std::size_t _hash;
};

ConstRef get_field(const ConstAggregateRef &value, int i);

//------------------------------------------------------------------------------

struct ConstPointer : Const {
    static bool classof(const Value *T);

    ConstPointer(const Type *type, const void *pointer);

    bool key_equal(const ConstPointer *other) const;
    std::size_t hash() const;

    static ConstPointerRef from(const Type *type, const void *pointer);
    static ConstPointerRef type_from(const Type *type);
    static ConstPointerRef closure_from(const Closure *closure);
    static ConstPointerRef string_from(const String *str);
    static ConstPointerRef list_from(const List *list);
    static ConstPointerRef scope_from(Scope *scope);
    static ConstPointerRef anchor_from(const Anchor *anchor);

    const void *value;
};

//------------------------------------------------------------------------------

#define SCOPES_GLOBAL_FLAGS() \
    /* if storage class is 'Uniform, the value is a SSBO */ \
    T(GF_BufferBlock, (1 << 0), "global-flag-buffer-block") \
    T(GF_NonWritable, (1 << 1), "global-flag-non-writable") \
    T(GF_NonReadable, (1 << 2), "global-flag-non-readable") \
    T(GF_Volatile, (1 << 3), "global-flag-volatile") \
    T(GF_Coherent, (1 << 4), "global-flag-coherent") \
    T(GF_Restrict, (1 << 5), "global-flag-restrict") \
    /* if storage class is 'Uniform, the value is a UBO */ \
    T(GF_Block, (1 << 6), "global-flag-block") \


enum GlobalFlags {
#define T(NAME, VALUE, SNAME) \
    NAME = VALUE,
SCOPES_GLOBAL_FLAGS()
#undef T
};

struct Global : Pure {
    static bool classof(const Value *T);

    Global(const Type *type, Symbol name,
        size_t flags, Symbol storage_class, int location, int binding);

    bool key_equal(const Global *other) const;
    std::size_t hash() const;

    static GlobalRef from(const Type *type, Symbol name,
        size_t flags = 0,
        Symbol storage_class = SYM_Unnamed,
        int location = -1, int binding = -1);

    const Type *element_type;
    PureRef initializer;
    FunctionRef constructor;
    Symbol name;
    size_t flags;
    Symbol storage_class;
    int location;
    int binding;
};

//------------------------------------------------------------------------------

struct PureCast : Pure {
    static bool classof(const Value *T);

    bool key_equal(const PureCast *other) const;
    std::size_t hash() const;

    PureCast(const Type *type, const PureRef &value);
    static PureRef from(const Type *type, PureRef value);

    PureRef value;
};

//------------------------------------------------------------------------------

struct Break : UntypedValue {
    static bool classof(const Value *T);

    Break(const ValueRef &value);

    static BreakRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct Merge : Terminator {
    static bool classof(const Value *T);

    Merge(const LabelRef &label, const TypedValues &values);

    static MergeRef from(const LabelRef &label, const TypedValues &values);

    LabelRef label;
};

//------------------------------------------------------------------------------

struct MergeTemplate : UntypedValue {
    static bool classof(const Value *T);

    MergeTemplate(const LabelTemplateRef &label, const ValueRef &value);

    static MergeTemplateRef from(const LabelTemplateRef &label, const ValueRef &value);

    LabelTemplateRef label;
    ValueRef value;
};

//------------------------------------------------------------------------------

struct RepeatTemplate : UntypedValue {
    static bool classof(const Value *T);

    RepeatTemplate(const ValueRef &value);

    static RepeatTemplateRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct Repeat : Terminator {
    static bool classof(const Value *T);

    Repeat(const LoopLabelRef &loop, const TypedValues &values);

    static RepeatRef from(const LoopLabelRef &loop, const TypedValues &values);

    LoopLabelRef loop;
};

//------------------------------------------------------------------------------

struct ReturnTemplate : UntypedValue {
    static bool classof(const Value *T);

    ReturnTemplate(const ValueRef &value);

    static ReturnTemplateRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct Return : Terminator {
    static bool classof(const Value *T);

    Return(const TypedValues &values);

    static ReturnRef from(const TypedValues &values);
};

//------------------------------------------------------------------------------

struct RaiseTemplate : UntypedValue {
    static bool classof(const Value *T);

    RaiseTemplate(const ValueRef &value);

    static RaiseTemplateRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct Raise : Terminator {
    static bool classof(const Value *T);

    Raise(const TypedValues &values);

    static RaiseRef from(const TypedValues &values);
};

//------------------------------------------------------------------------------

struct Quote : UntypedValue {
    static bool classof(const Value *T);

    Quote(const ValueRef &value);

    static QuoteRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct Unquote : UntypedValue {
    static bool classof(const Value *T);

    Unquote(const ValueRef &value);

    static UnquoteRef from(const ValueRef &value);

    ValueRef value;
};

//------------------------------------------------------------------------------

struct CompileStage : UntypedValue {
    static bool classof(const Value *T);

    CompileStage(const Anchor *anchor, const List *next, Scope *env);

    static CompileStageRef from(const Anchor *anchor, const List *next, Scope *env);

    const Anchor *anchor;
    const List *next;
    Scope *env;
};

//------------------------------------------------------------------------------

struct Closure {
    Closure(const TemplateRef &_func, const FunctionRef &_frame);

    bool key_equal(const Closure *other) const;
    std::size_t hash() const;

    TemplateRef func;
    FunctionRef frame;

    static Closure *from(const TemplateRef &func, const FunctionRef &frame);

    StyledStream &stream(StyledStream &ost) const;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_VALUE_HPP

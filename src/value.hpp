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

namespace scopes {

struct Anchor;
struct List;
struct Scope;

#define SCOPES_VALUE_KIND() \
    T(VK_Function, "value-kind-function", Function) \
    T(VK_Parameter, "value-kind-parameter", Parameter) \
    /* template-only */ \
    T(VK_Template, "value-kind-template", Template) \
    T(VK_Keyed, "value-kind-keyed", Keyed) \
    T(VK_Expression, "value-kind-expression", Expression) \
    T(VK_SyntaxExtend, "value-kind-syntax-extend", SyntaxExtend) \
    /* instructions (Instruction::classof) */ \
    T(VK_If, "value-kind-if", If) \
    T(VK_Try, "value-kind-try", Try) \
    T(VK_Call, "value-kind-call", Call) \
    T(VK_Loop, "value-kind-loop", Loop) \
    T(VK_Break, "value-kind-break", Break) \
    T(VK_Repeat, "value-kind-repeat", Repeat) \
    T(VK_Return, "value-kind-return", Return) \
    T(VK_Raise, "value-kind-raise", Raise) \
    T(VK_ArgumentList, "value-kind-argumentlist", ArgumentList) \
    T(VK_ExtractArgument, "value-kind-extractargument", ExtractArgument) \
    /* constants (Const::classof) */ \
    T(VK_Extern, "value-kind-extern", Extern) \
    T(VK_ConstInt, "value-kind-const-int", ConstInt) \
    T(VK_ConstReal, "value-kind-const-real", ConstReal) \
    T(VK_ConstTuple, "value-kind-const-tuple", ConstTuple) \
    T(VK_ConstArray, "value-kind-const-array", ConstArray) \
    T(VK_ConstVector, "value-kind-const-vector", ConstVector) \
    T(VK_ConstPointer, "value-kind-const-pointer", ConstPointer) \


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

typedef std::vector<Parameter *> Parameters;
typedef std::vector<Value *> Values;
typedef std::vector<Instruction *> Instructions;
typedef std::vector<Const *> Constants;
typedef std::vector<Block *> Blocks;

const char *get_value_kind_name(ValueKind kind);
const char *get_value_class_name(ValueKind kind);

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
private:
    const ValueKind _kind;
    const Type *_type;

protected:
    const Anchor *_anchor;
};

//------------------------------------------------------------------------------

struct Block {
    void append(Value *node);
    bool empty() const;

    Values body;
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

struct ArgumentList : Value {
    static bool classof(const Value *T);

    ArgumentList(const Anchor *anchor, const Values &values);

    void append(Symbol key, Value *node);
    void append(Value *node);

    static ArgumentList *from(const Anchor *anchor, const Values &values = {});

    Values values;
};

//------------------------------------------------------------------------------

struct ExtractArgument : Instruction {
    static bool classof(const Value *T);

    ExtractArgument(const Anchor *anchor, Value *value, int index);

    static ExtractArgument *from(const Anchor *anchor, Value *value, int index);

    int index;
    Value *value;
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
    Template *scope;
};

//------------------------------------------------------------------------------

struct Function : Value {
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
    const Type *return_type;
    const Type *except_type;
    Function *frame;
    Template *original;
    bool complete;

    ArgTypes instance_args;
    Function *find_frame(Template *scope);
    void bind(Value *oldnode, Value *newnode);
    Value *resolve(Value *node) const;
    Value *resolve_local(Value *node) const;
    std::unordered_map<Value *, Value *> map;
};

//------------------------------------------------------------------------------

struct Expression : Value {
    static bool classof(const Value *T);

    Expression(const Anchor *anchor, const Values &nodes, Value *value);
    void append(Value *node);

    static Expression *from(const Anchor *anchor, const Values &nodes = {}, Value *value = nullptr);

    Values body;
    Value *value;
    bool scoped;
};

//------------------------------------------------------------------------------

struct Clause {
    const Anchor *anchor;
    Value *cond;
    Block body;
    Value *value;

    Clause() : anchor(nullptr), cond(nullptr), value(nullptr) {}
    Clause(const Anchor *_anchor, Value *_cond, Value *_value)
        : anchor(_anchor), cond(_cond), value(_value) {}
    Clause(const Anchor *_anchor, Value *_value)
        : anchor(_anchor), cond(nullptr), value(_value) {}
};

typedef std::vector<Clause> Clauses;

struct If : Instruction {
    static bool classof(const Value *T);

    If(const Anchor *anchor, const Clauses &clauses);

    static If *from(const Anchor *anchor, const Clauses &clauses = {});

    Value *get_else_clause() const;
    void append(const Anchor *anchor, Value *cond, Value *value);
    void append(const Anchor *anchor, Value *value);
    Value *canonicalize();

    Clauses clauses;
    Clause else_clause;
};

//------------------------------------------------------------------------------

struct Try : Instruction {
    static bool classof(const Value *T);

    Try(const Anchor *anchor, Value *try_body, Parameter *except_param, Value *except_body);

    static Try *from(const Anchor *anchor,
        Value *try_body = nullptr,
        Parameter *except_param = nullptr,
        Value *except_body = nullptr);

    Block try_body;
    Value *try_value;
    Parameter *except_param;
    Block except_body;
    Value *except_value;
    const Type *raise_type;
};

//------------------------------------------------------------------------------

struct Parameter : Value {
    static bool classof(const Value *T);

    Parameter(const Anchor *anchor, Symbol name, const Type *type, bool variadic);
    static Parameter *from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);
    static Parameter *variadic_from(const Anchor *anchor, Symbol name = SYM_Unnamed, const Type *type = nullptr);

    bool is_variadic() const;

    Symbol name;
    bool variadic;
};

//------------------------------------------------------------------------------

enum CallFlags {
    CF_RawCall = (1 << 0),
    CF_TryCall = (1 << 1),
};

struct Call : Instruction {
    static bool classof(const Value *T);

    Call(const Anchor *anchor, Value *callee, const Values &args);
    static Call *from(const Anchor *anchor, Value *callee, const Values &args = {});
    bool is_rawcall() const;
    void set_rawcall();
    bool is_trycall() const;
    void set_trycall();

    Value *callee;
    Values args;
    uint32_t flags;
};

//------------------------------------------------------------------------------

struct Loop : Instruction {
    static bool classof(const Value *T);

    Loop(const Anchor *anchor, const Parameters &params, const Values &args, Value *value);

    static Loop *from(const Anchor *anchor, const Parameters &params = {}, const Values &args = {}, Value *value = nullptr);

    Parameters params;
    Values args;
    Block body;
    Value *value;
    const Type *return_type;
};

//------------------------------------------------------------------------------

struct Const : Value {
    static bool classof(const Value *T);

    Const(ValueKind _kind, const Anchor *anchor, const Type *type);
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

struct ConstTuple : Const {
    static bool classof(const Value *T);

    ConstTuple(const Anchor *anchor, const Type *type, const Constants &fields);

    static ConstTuple *from(const Anchor *anchor, const Type *type, const Constants &fields);
    static ConstTuple *none_from(const Anchor *anchor);

    Constants values;
};

//------------------------------------------------------------------------------

struct ConstArray : Const {
    static bool classof(const Value *T);

    ConstArray(const Anchor *anchor, const Type *type, const Constants &fields);

    static ConstArray *from(const Anchor *anchor, const Type *type, const Constants &fields);

    Constants values;
};

//------------------------------------------------------------------------------

struct ConstVector : Const {
    static bool classof(const Value *T);

    ConstVector(const Anchor *anchor, const Type *type, const Constants &fields);

    static ConstVector *from(const Anchor *anchor, const Type *type, const Constants &fields);

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

struct Extern : Const {
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

struct Break : Instruction {
    static bool classof(const Value *T);

    Break(const Anchor *anchor, Value *value);

    static Break *from(const Anchor *anchor, Value *value);

    Value *value;
};

//------------------------------------------------------------------------------

struct Repeat : Instruction {
    static bool classof(const Value *T);

    Repeat(const Anchor *anchor, const Values &args);

    static Repeat *from(const Anchor *anchor, const Values &args = {});

    Values args;
};

//------------------------------------------------------------------------------

struct Return : Instruction {
    static bool classof(const Value *T);

    Return(const Anchor *anchor, Value *value);

    static Return *from(const Anchor *anchor, Value *value);

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

struct SyntaxExtend : Value {
    static bool classof(const Value *T);

    SyntaxExtend(const Anchor *anchor, Template *func, const List *next, Scope *env);

    static SyntaxExtend *from(const Anchor *anchor, Template *func, const List *next, Scope *env);

    Template *func;
    const List *next;
    Scope *env;
};

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_VALUE_HPP

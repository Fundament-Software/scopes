
/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "value.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "types.hpp"
#include "stream_ast.hpp"
#include "dyn_cast.inc"

#include <assert.h>

namespace scopes {

const char *get_value_kind_name(ValueKind kind) {
    switch(kind) {
#define T(NAME, BNAME, CLASS) \
    case NAME: return BNAME;
SCOPES_VALUE_KIND()
#undef T
    default: return "???";
    }
}

const char *get_value_class_name(ValueKind kind) {
    switch(kind) {
#define T(NAME, BNAME, CLASS) \
    case NAME: return #CLASS;
SCOPES_VALUE_KIND()
#undef T
    default: return "???";
    }
}

//------------------------------------------------------------------------------

Keyed::Keyed(const Anchor *anchor, Symbol _key, Value *node)
    : Value(VK_Keyed, anchor), key(_key), value(node)
{
}

Keyed *Keyed::from(const Anchor *anchor, Symbol key, Value *node) {
    assert(node);
    if (isa<Keyed>(node)) {
        node = cast<Keyed>(node)->value;
    }
    return new Keyed(anchor, key, node);
}

//------------------------------------------------------------------------------

ArgumentList::ArgumentList(const Anchor *anchor, const Values &_values)
    : Value(VK_ArgumentList, anchor), values(_values) {
}

void ArgumentList::append(Value *node) {
    values.push_back(node);
}

void ArgumentList::append(Symbol key, Value *node) {
    assert(false); // todo: store key
    values.push_back(node);
}

ArgumentList *ArgumentList::from(const Anchor *anchor, const Values &values) {
    return new ArgumentList(anchor, values);
}

//------------------------------------------------------------------------------

ExtractArgument::ExtractArgument(const Anchor *anchor, Value *_value, int _index, bool _vararg)
    : Instruction(VK_ExtractArgument, anchor), index(_index), value(_value), vararg(_vararg) {
    assert(index >= 0);
}

ExtractArgument *ExtractArgument::from(const Anchor *anchor, Value *value, int index, bool vararg) {
    return new ExtractArgument(anchor, value, index, vararg);
}

//------------------------------------------------------------------------------

Template::Template(const Anchor *anchor, Symbol _name, const Parameters &_params, Value *_value)
    : Pure(VK_Template, anchor),
        name(_name), params(_params), value(_value),
        _inline(false), docstring(nullptr), scope(nullptr) {
}

bool Template::is_forward_decl() const {
    return !value;
}

void Template::set_inline() {
    _inline = true;
}

bool Template::is_inline() const {
    return _inline;
}

void Template::append_param(Parameter *sym) {
    params.push_back(sym);
}

Template *Template::from(
    const Anchor *anchor, Symbol name,
    const Parameters &params, Value *value) {
    return new Template(anchor, name, params, value);
}

//------------------------------------------------------------------------------

Function::Function(const Anchor *anchor, Symbol _name, const Parameters &_params)
    : Pure(VK_Function, anchor),
        name(_name), params(_params),
        docstring(nullptr), return_type(nullptr), except_type(nullptr),
        frame(nullptr), original(nullptr), label(nullptr), complete(false) {
    set_type(TYPE_Unknown);
}

void Function::append_param(Parameter *sym) {
    // verify that the symbol is typed
    assert(sym->is_typed());
    params.push_back(sym);
}

Value *Function::resolve_local(Value *node) const {
    auto it = map.find(node);
    if (it == map.end())
        return nullptr;
    return it->second;
}

Value *Function::resolve(Value *node) const {
    auto fn = this;
    while (fn) {
        auto val = fn->resolve_local(node);
        if (val) return val;
        fn = fn->frame;
    }
    return nullptr;
}

Function *Function::find_frame(Template *scope) {
    Function *frame = this;
    while (frame) {
        if (scope == frame->original)
            return frame;
        frame = frame->frame;
    }
    return nullptr;
}

void Function::bind(Value *oldnode, Value *newnode) {
    map.insert({oldnode, newnode});
}

Function *Function::from(
    const Anchor *anchor, Symbol name,
    const Parameters &params) {
    return new Function(anchor, name, params);
}

//------------------------------------------------------------------------------

Extern::Extern(const Anchor *anchor, const Type *type, Symbol _name, size_t _flags, Symbol _storage_class, int _location, int _binding)
    : Pure(VK_Extern, anchor), name(_name), flags(_flags), storage_class(_storage_class), location(_location), binding(_binding) {
    if ((storage_class == SYM_SPIRV_StorageClassUniform)
        && !(flags & EF_BufferBlock)) {
        flags |= EF_Block;
    }
    size_t ptrflags = required_flags_for_storage_class(storage_class);
    if (flags & EF_NonWritable)
        ptrflags |= PTF_NonWritable;
    else if (flags & EF_NonReadable)
        ptrflags |= PTF_NonReadable;
    set_type(pointer_type(type, ptrflags, storage_class));
}

Extern *Extern::from(const Anchor *anchor, const Type *type, Symbol name, size_t flags, Symbol storage_class, int location, int binding) {
    return new Extern(anchor, type, name, flags, storage_class, location, binding);
}

//------------------------------------------------------------------------------

/*
Value *Block::canonicalize() {
    strip_constants();
    // can strip block if no side effects
    if (body.empty())
        return ArgumentList::from(anchor());
    else if (body.size() == 1)
        return body[0];
    else
        return this;
}

void Block::strip_constants() {
    int i = (int)body.size();
    while (i > 0) {
        i--;
        auto arg = body[i];
        if (arg->is_pure()) {
            body.erase(body.begin() + i);
        }
    }
}
*/

void Block::clear() {
    body.clear();
}

void Block::migrate_from(Block &source) {
    for (auto arg : source.body) {
        arg->block = nullptr;
        append(arg);
    }
    source.clear();
}

bool Block::empty() const {
    return body.empty();
}

void Block::append(Value *node) {
    if (node->is_pure())
        return;
    if (isa<Instruction>(node)) {
        auto instr = cast<Instruction>(node);
        if (instr->block)
            return;
        instr->block = this;
        body.push_back(instr);
    }
}

//------------------------------------------------------------------------------

Expression::Expression(const Anchor *anchor, const Values &_body, Value *_value)
    : Value(VK_Expression, anchor), body(_body), value(_value), scoped(true) {
}

void Expression::append(Value *node) {
    assert(node);
    if (value && !value->is_pure()) {
        body.push_back(value);
    }
    value = node;
}

Expression *Expression::from(const Anchor *anchor, const Values &nodes, Value *value) {
    return new Expression(anchor, nodes, value);
}

//------------------------------------------------------------------------------

If::If(const Anchor *anchor, const Clauses &_clauses)
    : Instruction(VK_If, anchor), clauses(_clauses) {
}

If *If::from(const Anchor *anchor, const Clauses &_clauses) {
    return new If(anchor, _clauses);
}

void If::append(const Anchor *anchor, Value *cond, Value *value) {
    assert(anchor);
    assert(cond);
    assert(value);
    Clause clause;
    clause.anchor = anchor;
    clause.cond = cond;
    clause.value = value;
    clauses.push_back(clause);
}

void If::append(const Anchor *anchor, Value *value) {
    assert(anchor);
    assert(value);
    assert(!else_clause.value);
    else_clause.anchor = anchor;
    else_clause.value = value;
}

Value *If::canonicalize() {
    if (!else_clause.value) {
        else_clause.anchor = anchor();
        else_clause.value = ArgumentList::from(anchor());
    }
    return this;
}

//------------------------------------------------------------------------------

Try::Try(const Anchor *anchor, Value *_try_value, Parameter *_except_param, Value *_except_value)
    : Instruction(VK_Try, anchor), try_value(_try_value),
        except_param(_except_param), except_value(_except_value),
        raise_type(nullptr)
{}

Try *Try::from(const Anchor *anchor, Value *try_value, Parameter *except_param,
    Value *except_value) {
    return new Try(anchor, try_value, except_param, except_value);
}

//------------------------------------------------------------------------------

Parameter::Parameter(const Anchor *anchor, Symbol _name, const Type *_type, bool _variadic)
    : Value(VK_Parameter, anchor), name(_name), variadic(_variadic) {
    if (_type) set_type(_type);
}

Parameter *Parameter::from(const Anchor *anchor, Symbol name, const Type *type) {
    return new Parameter(anchor, name, type, false);
}

Parameter *Parameter::variadic_from(const Anchor *anchor, Symbol name, const Type *type) {
    return new Parameter(anchor, name, type, true);
}

bool Parameter::is_variadic() const {
    return variadic;
}

//------------------------------------------------------------------------------

Call::Call(const Anchor *anchor, Value *_callee, const Values &_args)
    : Instruction(VK_Call, anchor), callee(_callee), args(_args), flags(0) {
}

bool Call::is_rawcall() const {
    return flags & CF_RawCall;
}

void Call::set_rawcall() {
    flags |= CF_RawCall;
}

bool Call::is_trycall() const {
    return flags & CF_TryCall;
}

void Call::set_trycall() {
    flags |= CF_TryCall;
}

Call *Call::from(const Anchor *anchor, Value *callee, const Values &args) {
    return new Call(anchor, callee, args);
}

//------------------------------------------------------------------------------

Loop::Loop(const Anchor *anchor, const Parameters &_params, const Values &_args, Value *_value)
    : Instruction(VK_Loop, anchor), params(_params), args(_args), value(_value), return_type(nullptr) {
}

Loop *Loop::from(const Anchor *anchor, const Parameters &params, const Values &args, Value *value) {
    return new Loop(anchor, params, args, value);
}

//------------------------------------------------------------------------------

Label::Label(const Anchor *anchor, Value *_value)
    : Instruction(VK_Label, anchor), value(_value), return_type(nullptr) {}

Label *Label::from(const Anchor *anchor, Value *value) {
    return new Label(anchor, value);
}

//------------------------------------------------------------------------------

bool Pure::classof(const Value *T) {
    auto k = T->kind();
    return (k == VK_Function) || (k == VK_Extern) || (k == VK_Template) || Const::classof(T);
}

Pure::Pure(ValueKind _kind, const Anchor *anchor)
    : Value(_kind, anchor) {
}

//------------------------------------------------------------------------------

bool Const::classof(const Value *T) {
    auto k = T->kind();
    return (k >= VK_ConstInt) && (k <= VK_ConstPointer);
}

Const::Const(ValueKind _kind, const Anchor *anchor, const Type *type)
    : Pure(_kind, anchor) {
    set_type(type);
}

//------------------------------------------------------------------------------

ConstInt::ConstInt(const Anchor *anchor, const Type *type, uint64_t _value)
    : Const(VK_ConstInt, anchor, type), value(_value) {
}

ConstInt *ConstInt::from(const Anchor *anchor, const Type *type, uint64_t value) {
    return new ConstInt(anchor, type, value);
}

ConstInt *ConstInt::symbol_from(const Anchor *anchor, Symbol value) {
    return new ConstInt(anchor, TYPE_Symbol, value.value());
}

ConstInt *ConstInt::builtin_from(const Anchor *anchor, Builtin value) {
    return new ConstInt(anchor, TYPE_Builtin, value.value());
}

//------------------------------------------------------------------------------

ConstReal::ConstReal(const Anchor *anchor, const Type *type, double _value)
    : Const(VK_ConstReal, anchor, type), value(_value) {}

ConstReal *ConstReal::from(const Anchor *anchor, const Type *type, double value) {
    return new ConstReal(anchor, type, value);
}

//------------------------------------------------------------------------------

ConstAggregate::ConstAggregate(const Anchor *anchor, const Type *type, const Constants &_fields)
    : Const(VK_ConstAggregate, anchor, type), values(_fields) {
}

ConstAggregate *ConstAggregate::from(const Anchor *anchor, const Type *type, const Constants &fields) {
    return new ConstAggregate(anchor, type, fields);
}

ConstAggregate *ConstAggregate::none_from(const Anchor *anchor) {
    return from(anchor, TYPE_Nothing, {});
}

//------------------------------------------------------------------------------

ConstPointer::ConstPointer(const Anchor *anchor, const Type *type, const void *_pointer)
    : Const(VK_ConstPointer, anchor, type), value(_pointer) {}

ConstPointer *ConstPointer::from(const Anchor *anchor, const Type *type, const void *pointer) {
    return new ConstPointer(anchor, type, pointer);
}

ConstPointer *ConstPointer::type_from(const Anchor *anchor, const Type *type) {
    return from(anchor, TYPE_Type, type);
}

ConstPointer *ConstPointer::closure_from(const Anchor *anchor, const Closure *closure) {
    return from(anchor, TYPE_Closure, closure);
}

ConstPointer *ConstPointer::string_from(const Anchor *anchor, const String *str) {
    return from(anchor, TYPE_String, str);
}

ConstPointer *ConstPointer::ast_from(const Anchor *anchor, Value *node) {
    return from(anchor, TYPE_Value, node);
}

ConstPointer *ConstPointer::list_from(const Anchor *anchor, const List *list) {
    return from(anchor, TYPE_List, list);
}

ConstPointer *ConstPointer::scope_from(const Anchor *anchor, Scope *scope) {
    return from(anchor, TYPE_Scope, scope);
}

ConstPointer *ConstPointer::anchor_from(const Anchor *anchor) {
    return from(anchor, TYPE_Anchor, anchor);
}

//------------------------------------------------------------------------------

Break::Break(const Anchor *anchor, Value *_value)
    : Instruction(VK_Break, anchor), value(_value) {
}

Break *Break::from(const Anchor *anchor, Value *value) {
    return new Break(anchor, value);
}

//------------------------------------------------------------------------------

Repeat::Repeat(const Anchor *anchor, const Values &_args)
    : Instruction(VK_Repeat, anchor), args(_args) {}

Repeat *Repeat::from(const Anchor *anchor, const Values &args) {
    return new Repeat(anchor, args);
}

//------------------------------------------------------------------------------

Return::Return(const Anchor *anchor, Value *_value)
    : Instruction(VK_Return, anchor), value(_value) {}

Return *Return::from(const Anchor *anchor, Value *value) {
    return new Return(anchor, value);
}

//------------------------------------------------------------------------------

Merge::Merge(const Anchor *anchor, Label *_label, Value *_value)
    : Instruction(VK_Merge, anchor), label(_label), value(_value) {}

Merge *Merge::from(const Anchor *anchor, Label *label, Value *value) {
    return new Merge(anchor, label, value);
}

//------------------------------------------------------------------------------

Raise::Raise(const Anchor *anchor, Value *_value)
    : Instruction(VK_Raise, anchor), value(_value) {}

Raise *Raise::from(const Anchor *anchor, Value *value) {
    return new Raise(anchor, value);
}

//------------------------------------------------------------------------------

CompileStage::CompileStage(const Anchor *anchor, const List *_next, Scope *_env)
    : Value(VK_CompileStage, anchor), next(_next), env(_env) {
}

CompileStage *CompileStage::from(const Anchor *anchor, const List *next, Scope *env) {
    return new CompileStage(anchor, next, env);
}

//------------------------------------------------------------------------------

ValueKind Value::kind() const { return _kind; }

Value::Value(ValueKind kind, const Anchor *anchor)
    : _kind(kind),_type(nullptr),_anchor(anchor) {
    assert(_anchor);
}

bool Value::is_pure() const {
    switch(kind()) {
    case VK_Parameter:
        return true;
    case VK_ArgumentList: {
        auto al = cast<ArgumentList>(this);
        for (auto val : al->values) {
            assert(val);
            if (!val->is_pure())
                return false;
        }
        return true;
    } break;
    default: break;
    }
    return isa<Pure>(this);
}

bool Value::is_typed() const {
    return _type != nullptr;
}
void Value::set_type(const Type *type) {
    assert(!is_typed());
    _type = type;
}

void Value::change_type(const Type *type) {
    assert(is_typed());
    _type = type;
}

const Type *Value::get_type() const {
    assert(_type);
    return _type;
}

const Anchor *Value::anchor() const {
    return _anchor;
}

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const Value *T) { \
        return T->kind() == NAME; \
    }
SCOPES_VALUE_KIND()
#undef T

//------------------------------------------------------------------------------

Instruction::Instruction(ValueKind _kind, const Anchor *_anchor)
    : Value(_kind, _anchor), name(SYM_Unnamed), block(nullptr) {
}

bool Instruction::classof(const Value *T) {
    auto k = T->kind();
    return (k >= VK_If) && (k <= VK_ExtractArgument);
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Value *node) {
    stream_ast(ost, node, StreamASTFormat());
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const Value *node) {
    stream_ast(ost, node, StreamASTFormat());
    return ost;
}

} // namespace scopes

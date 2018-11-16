
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
#include "qualifier.inc"
#include "hash.hpp"

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

std::size_t ValueIndex::Hash::operator()(const ValueIndex & s) const {
    return hash2(std::hash<Value *>{}(s.value), s.index);
}

ValueIndex::ValueIndex(Value *_value, int _index)
    : value(_value), index(_index) {
    if (isa<ArgumentList>(value)) {
        auto al = cast<ArgumentList>(value);
        if (index < al->values.size()) {
            value = al->values[index];
            index = 0;
        }
    }
}

bool ValueIndex::operator ==(const ValueIndex &other) const {
    return value == other.value && index == other.index;
}

const Type *ValueIndex::get_type() const {
    return get_argument(value->get_type(), index);
}

const ValueIndexSet *ValueIndex::deps() const {
    if (value->deps.empty(index))
        return nullptr;
    const ValueIndexSet &args = value->deps.args[index];
    return &args;
}

bool ValueIndex::has_deps() const {
    return !value->deps.empty(index);
}

//------------------------------------------------------------------------------

bool Depends::empty(int index) const {
    if (index >= args.size()) return true;
    return args[index].empty();
}

bool Depends::empty() const {
    for (auto &&arg : args) {
        if (!arg.empty())
            return false;
    }
    return true;
}

void Depends::ensure_arg(int index) {
    while(args.size() <= index) {
        args.push_back(ValueIndexSet());
        kinds.push_back(DK_Undefined);
    }
}

void Depends::unique(Value *value) {
    auto T = value->get_type();
    int count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        unique(i);
    }
}

void Depends::unique(int index) {
    ensure_arg(index);
    auto &&s = kinds[index];
    s |= DK_Unique;
}

void Depends::borrow(Value *value) {
    auto T = value->get_type();
    int count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        borrow(i, ValueIndex(value, i));
    }
}

void Depends::borrow(int index, ValueIndex value) {
    ensure_arg(index);
    ValueIndexSet &arg = args[index];
    auto &&s = kinds[index];
    s |= DK_Borrowed;
    arg.insert(value);
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
    : Instruction(VK_ArgumentList, anchor), values(_values) {
}

void ArgumentList::append(Value *node) {
    values.push_back(node);
}

void ArgumentList::append(Symbol key, Value *node) {
    assert(false); // todo: store key
    values.push_back(node);
}

bool ArgumentList::is_constant() const {
    for (auto val : values) {
        assert(val);
        if (!isa<Const>(val))
            return false;
    }
    return true;
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
    : Value(VK_Template, anchor),
        name(_name), params(_params), value(_value),
        _inline(false), docstring(nullptr) {
    int index = 0;
    for (auto param : params) {
        param->set_owner(this, index++);
    }
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
    sym->set_owner(this, params.size());
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
        name(_name), params(_params), value(nullptr),
        docstring(nullptr), return_type(nullptr), except_type(nullptr),
        frame(nullptr), boundary(nullptr), original(nullptr), label(nullptr),
        complete(false), next_id(1) {
    set_type(TYPE_Unknown);
    body.depth = 1;
    int index = 0;
    for (auto param : params) {
        param->set_owner(this, index++);
    }
}

uint32_t Function::new_id() {
    return next_id++;
}

uint32_t Function::get_id(Value *value) {
    auto result = value2id.find(value);
    if (result == value2id.end()) {
        auto id = new_id();
        value2id.insert({value, id});
        id2value.insert({id, value});
        return id;
    }
    return result->second;
}

Value *Function::get_value(uint32_t id) {
    auto result = id2value.find(id);
    assert(result != id2value.end());
    return result->second;
}

uint32_t Function::get_id(Symbol name) {
    auto result = name2block.find(name);
    if (result == name2block.end()) {
        auto id = new_id();
        name2block.insert({ name, id });
        return id;
    }
    return result->second;
}

void Function::append_param(Parameter *sym) {
    // verify that the symbol is typed
    assert(sym->is_typed());
    sym->set_owner(this, params.size());
    params.push_back(sym);
}

Value *Function::resolve_local(Value *node) const {
    auto it = map.find(node);
    if (it == map.end())
        return nullptr;
    return it->second;
}

Value *Function::unsafe_resolve(Value *node) const {
    auto fn = this;
    while (fn) {
        auto val = fn->resolve_local(node);
        if (val) return val;
        fn = fn->frame;
    }
    return nullptr;
}

SCOPES_RESULT(Value *) Function::resolve(Value *node, Function *_boundary) const {
    SCOPES_RESULT_TYPE(Value *)
    auto fn = this;
    while (fn) {
        auto val = fn->resolve_local(node);
        if (val) {
            if ((fn->boundary != _boundary) && !val->is_accessible()) {
                SCOPES_EXPECT_ERROR(
                    error_value_inaccessible_from_closure(val, _boundary));
            }
            return val;
        }
        fn = fn->frame;
    }
    return nullptr;
}

void Function::bind(Value *oldnode, Value *newnode) {
    auto it = map.insert({oldnode, newnode});
    if (!it.second) {
        it.first->second = newnode;
    }
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

Block::Block()
    : depth(0), insert_index(0), terminator(nullptr), blockid(0), parent(nullptr)
{}

void Block::annotate(const String *msg) {
    annotations.push_back(msg);
}

bool Block::is_scoped() const {
    return blockid != 0;
}

void Block::set_scope(uint32_t _blockid, Block *_parent) {
    assert(_blockid);
    blockid = _blockid;
    parent = _parent;
    if (_parent) {
        depth = _parent->depth + 1;
    }
}

void Block::clear() {
    body.clear();
    terminator = nullptr;
    blockid = 0;
    parent = nullptr;
}

void Block::migrate_from(Block &source) {
    for (auto arg : source.body) {
        arg->block = nullptr;
        append(arg);
    }
    if (source.terminator) {
        terminator = source.terminator;
        terminator->block = this;
    }
    source.clear();
}

bool Block::empty() const {
    return body.empty() && !terminator;
}

void Block::insert_at(int index) {
    assert((index >= 0) && (index <= body.size()));
    insert_index = index;
}

void Block::insert_at_end() {
    insert_index = body.size();
}

bool Block::append(Value *node) {
    if (node->is_pure())
        return false;
    if (isa<Instruction>(node)) {
        auto instr = cast<Instruction>(node);
        if (instr->block)
            return false;
        instr->block = this;
        if (isa<Loop>(node)) {
            auto loop = cast<Loop>(node);
            assert(loop->param);
            loop->param->block = this;
        }
        if (!is_returning(instr->get_type())) {
            assert(!terminator);
            assert(insert_index == body.size());
            terminator = instr;
        } else {
            body.insert(body.begin() + insert_index, instr);
            insert_index++;
        }
        return true;
    }
    return false;
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

Expression *Expression::unscoped_from(const Anchor *anchor, const Values &nodes, Value *value) {
    auto expr = new Expression(anchor, nodes, value);
    expr->scoped = false;
    return expr;
}


//------------------------------------------------------------------------------

bool If::Clause::is_then() const {
    return cond != nullptr;
}

If::If(const Anchor *anchor, const Clauses &_clauses)
    : Instruction(VK_If, anchor), clauses(_clauses) {
}

If *If::from(const Anchor *anchor, const Clauses &_clauses) {
    return new If(anchor, _clauses);
}

void If::append_then(const Anchor *anchor, Value *cond, Value *value) {
    assert(anchor);
    assert(cond);
    assert(value);
    Clause clause;
    clause.anchor = anchor;
    clause.cond = cond;
    clause.value = value;
    clauses.push_back(clause);
}

void If::append_else(const Anchor *anchor, Value *value) {
    assert(anchor);
    assert(value);
    Clause clause;
    clause.anchor = anchor;
    clause.value = value;
    clauses.push_back(clause);
}

//------------------------------------------------------------------------------

Switch::Switch(const Anchor *anchor, Value *_expr, const Cases &_cases)
    : Instruction(VK_Switch, anchor), expr(_expr), cases(_cases)
{}

Switch *Switch::from(const Anchor *anchor, Value *expr, const Cases &cases) {
    return new Switch(anchor, expr, cases);
}

void Switch::append_case(const Anchor *anchor, Value *literal, Value *value) {
    assert(anchor);
    assert(literal);
    assert(value);
    Case _case;
    _case.kind = CK_Case;
    _case.anchor = anchor;
    _case.literal = literal;
    _case.value = value;
    cases.push_back(_case);
}

void Switch::append_pass(const Anchor *anchor, Value *literal, Value *value) {
    assert(anchor);
    assert(literal);
    assert(value);
    Case _case;
    _case.kind = CK_Pass;
    _case.anchor = anchor;
    _case.literal = literal;
    _case.value = value;
    cases.push_back(_case);
}

void Switch::append_default(const Anchor *anchor, Value *value) {
    assert(anchor);
    assert(value);
    Case _case;
    _case.kind = CK_Default;
    _case.anchor = anchor;
    _case.value = value;
    cases.push_back(_case);
}

//------------------------------------------------------------------------------

Parameter::Parameter(const Anchor *anchor, Symbol _name, const Type *_type, bool _variadic)
    : Value(VK_Parameter, anchor), name(_name), variadic(_variadic),
        owner(nullptr), block(nullptr), index(-1) {
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

void Parameter::set_owner(Value *_owner, int _index) {
    assert(!owner);
    owner = _owner;
    index = _index;
}

//------------------------------------------------------------------------------

Call::Call(const Anchor *anchor, Value *_callee, const Values &_args)
    : Instruction(VK_Call, anchor), callee(_callee), args(_args), flags(0), except_label(nullptr) {
}

bool Call::is_rawcall() const {
    return flags & CF_RawCall;
}

void Call::set_rawcall() {
    flags |= CF_RawCall;
}

Call *Call::from(const Anchor *anchor, Value *callee, const Values &args) {
    return new Call(anchor, callee, args);
}

//------------------------------------------------------------------------------

Loop::Loop(const Anchor *anchor, Parameter *_param, Value *_init, Value *_value)
    : Instruction(VK_Loop, anchor), param(_param), init(_init), value(_value), return_type(nullptr) {
    if (param) {
        param->set_owner(this, 0);
    }
}

Loop *Loop::from(const Anchor *anchor, Parameter *param, Value *init, Value *value) {
    return new Loop(anchor, param, init, value);
}

void Loop::set_param(Parameter *_param) {
    assert(!param);
    param = _param;
    param->set_owner(this, 0);
}

//------------------------------------------------------------------------------

Label::Label(const Anchor *anchor, LabelKind _kind, Symbol _name, Value *_value)
    : Instruction(VK_Label, anchor), name(_name), value(_value),
        return_type(nullptr), label_kind(_kind) {}

Label *Label::from(const Anchor *anchor, LabelKind kind, Symbol name, Value *value) {
    return new Label(anchor, kind, name, value);
}

Label *Label::try_from(const Anchor *anchor,
    Value *value) {
    return new Label(anchor, LK_Try, KW_Try, value);
}
Label *Label::except_from(const Anchor *anchor,
    Value *value) {
    return new Label(anchor, LK_Except, KW_Except, value);
}

bool Label::is_try() const {
    return label_kind == LK_Try;
}

bool Label::is_except() const {
    return label_kind == LK_Except;
}

//------------------------------------------------------------------------------

bool Pure::classof(const Value *T) {
    auto k = T->kind();
    return (k == VK_Function) || (k == VK_Extern) || Const::classof(T);
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

Repeat::Repeat(const Anchor *anchor, Value *_value)
    : Instruction(VK_Repeat, anchor), value(_value) {}

Repeat *Repeat::from(const Anchor *anchor, Value *value) {
    return new Repeat(anchor, value);
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

Quote::Quote(const Anchor *anchor, Value *_value)
    : Value(VK_Quote, anchor), value(_value) {
}

Quote *Quote::from(const Anchor *anchor, Value *value) {
    return new Quote(anchor, value);
}

//------------------------------------------------------------------------------

Unquote::Unquote(const Anchor *anchor, Value *_value)
    : Value(VK_Unquote, anchor), value(_value) {
}

Unquote *Unquote::from(const Anchor *anchor, Value *value) {
    return new Unquote(anchor, value);
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

bool Value::is_accessible() const {
    switch(kind()) {
    case VK_ArgumentList: {
        auto al = cast<ArgumentList>(this);
        for (auto val : al->values) {
            assert(val);
            if (!val->is_accessible())
                return false;
        }
        return true;
    } break;
    default: break;
    }
    return isa<Pure>(this);
}

void Value::annotate(const String *msg) {
    annotations.push_back(msg);
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

int Value::get_depth() const {
    const Value *value = this;
    if (isa<Parameter>(value)) {
        auto param = cast<Parameter>(value);
        assert(param->owner);
        if (param->block) {
            return param->block->depth;
        } if (isa<Function>(param->owner)) {
            // outside of function
            return 0;
        } else if (isa<Instruction>(param->owner)) {
            auto instr = cast<Instruction>(param->owner);
            if (!instr->block) {
                StyledStream ss;
                stream_ast(ss, instr, StreamASTFormat());
            }
            assert(instr->block);
            return instr->block->depth;
        } else {
            assert(false);
            return 0;
        }
    } else if (isa<Instruction>(value)) {
        if (value->is_pure())
            return 0;
        auto instr = cast<Instruction>(value);
        assert(instr->block);
        return instr->block->depth;
    }
    return 0;
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
    ost << const_cast<const Value *>(node);
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const Value *node) {
    ost << Style_Keyword << get_value_class_name(node->kind()) << Style_None;
    ost << "$" << (const void *)node;
    if (node->is_typed()) {
        ost << Style_Operator << ":" << Style_None << node->get_type();
    }
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const ValueIndex &arg) {
    ost << arg.value;
    if (arg.value->is_typed()) {
        if (get_argument_count(arg.value->get_type()) != 1) {
            ost << Style_Operator << "@" << Style_None << arg.index;
            ost << Style_Operator << ":" << Style_None << arg.get_type();
        }
    } else {
        ost << Style_Operator << "@" << Style_None << arg.index;
    }
    return ost;
}

StyledStream& operator<<(StyledStream& ost, ValueIndex &arg) {
    ost << const_cast<const ValueIndex &>(arg);
    return ost;
}

} // namespace scopes


/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "ast.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "types.hpp"
#include "stream_ast.hpp"
#include "dyn_cast.inc"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------

Keyed::Keyed(const Anchor *anchor, Symbol _key, ASTNode *node)
    : ASTNode(ASTK_Keyed, anchor), key(_key), value(node)
{}

Keyed *Keyed::from(const Anchor *anchor, Symbol key, ASTNode *node) {
    return new Keyed(anchor, key, node);
}

//------------------------------------------------------------------------------

ASTArgumentList::ASTArgumentList(const Anchor *anchor, const ASTNodes &_values)
    : ASTNode(ASTK_ArgumentList, anchor), values(_values) {
}

void ASTArgumentList::append(ASTNode *node) {
    values.push_back(node);
}

void ASTArgumentList::append(Symbol key, ASTNode *node) {
    assert(false); // todo: store key
    values.push_back(node);
}

ASTArgumentList *ASTArgumentList::from(const Anchor *anchor, const ASTNodes &values) {
    return new ASTArgumentList(anchor, values);
}

//------------------------------------------------------------------------------

ASTExtractArgument::ASTExtractArgument(const Anchor *anchor, ASTNode *_value, int _index)
    : ASTNode(ASTK_ExtractArgument, anchor), value(_value), index(_index) {
}

ASTExtractArgument *ASTExtractArgument::from(const Anchor *anchor, ASTNode *value, int index) {
    return new ASTExtractArgument(anchor, value, index);
}

//------------------------------------------------------------------------------

Template::Template(const Anchor *anchor, Symbol _name, const ASTSymbols &_params, ASTNode *_value)
    : ASTValue(ASTK_Template, anchor),
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

void Template::append_param(ASTSymbol *sym) {
    params.push_back(sym);
}

Template *Template::from(
    const Anchor *anchor, Symbol name,
    const ASTSymbols &params, ASTNode *value) {
    return new Template(anchor, name, params, value);
}

//------------------------------------------------------------------------------

ASTFunction::ASTFunction(const Anchor *anchor, Symbol _name, const ASTSymbols &_params, ASTNode *_value)
    : ASTValue(ASTK_Function, anchor),
        name(_name), params(_params), value(_value),
        docstring(nullptr), return_type(nullptr), frame(nullptr), original(nullptr), complete(false) {
}

void ASTFunction::append_param(ASTSymbol *sym) {
    // verify that the symbol is typed
    assert(sym->is_typed());
    params.push_back(sym);
}

ASTNode *ASTFunction::resolve(ASTNode *node) {
    auto it = map.find(node);
    if (it == map.end())
        return nullptr;
    return it->second;
}

ASTFunction *ASTFunction::find_frame(Template *scope) {
    ASTFunction *frame = this;
    while (frame) {
        if (scope == frame->original)
            return frame;
        frame = frame->frame;
    }
    return nullptr;
}

void ASTFunction::bind(ASTNode *oldnode, ASTNode *newnode) {
    map.insert({oldnode, newnode});
}

ASTFunction *ASTFunction::from(
    const Anchor *anchor, Symbol name,
    const ASTSymbols &params, ASTNode *value) {
    return new ASTFunction(anchor, name, params, value);
}

//------------------------------------------------------------------------------

ASTExtern::ASTExtern(const Anchor *anchor, const Type *type, Symbol _name, size_t _flags, Symbol _storage_class, int _location, int _binding)
    : ASTValue(ASTK_Extern, anchor), name(_name), flags(_flags), storage_class(_storage_class), location(_location), binding(_binding) {
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

ASTExtern *ASTExtern::from(const Anchor *anchor, const Type *type, Symbol name, size_t flags, Symbol storage_class, int location, int binding) {
    return new ASTExtern(anchor, type, name, flags, storage_class, location, binding);
}

//------------------------------------------------------------------------------

Block::Block(const Anchor *anchor, const ASTNodes &_body, ASTNode *_value)
    : ASTNode(ASTK_Block, anchor), body(_body), value(_value) {
}

Block *Block::from(const Anchor *anchor, const ASTNodes &nodes, ASTNode *value) {
    return new Block(anchor, nodes, value);
}

ASTNode *Block::canonicalize() {
    if (!value) {
        if (!body.empty()) {
            value = body.back();
            body.pop_back();
        } else {
            value = ASTArgumentList::from(anchor());
        }
    }
    strip_constants();
    // can strip block if no side effects
    if (body.empty())
        return value;
    else
        return this;
}

void Block::strip_constants() {
    int i = (int)body.size();
    while (i > 0) {
        i--;
        auto arg = body[i];
        if (isa<ASTValue>(arg)) {
            body.erase(body.begin() + i);
        }
    }
}

void Block::append(ASTNode *node) {
    assert(!value);
    body.push_back(node);
}

//------------------------------------------------------------------------------

If::If(const Anchor *anchor, const Clauses &_clauses)
    : ASTNode(ASTK_If, anchor), clauses(_clauses) {
}

If *If::from(const Anchor *anchor, const Clauses &_clauses) {
    return new If(anchor, _clauses);
}

void If::append(const Anchor *anchor, ASTNode *cond, ASTNode *value) {
    clauses.push_back({anchor, cond, value});
}

void If::append(const Anchor *anchor, ASTNode *value) {
    assert(!else_clause.value);
    else_clause = Clause(anchor, nullptr, value);
}

ASTNode *If::canonicalize() {
    if (!else_clause.value) {
        else_clause = Clause(anchor(), ASTArgumentList::from(anchor()));
    }
    return this;
}

//------------------------------------------------------------------------------

ASTValue::ASTValue(ASTKind _kind, const Anchor *anchor)
    : ASTNode(_kind, anchor) {
}

bool ASTValue::classof(const ASTNode *T) {
    if (Const::classof(T)) return true;
    switch(T->kind()) {
    case ASTK_Template:
    case ASTK_Function:
    case ASTK_Symbol:
    case ASTK_Extern:
        return true;
    default: return false;
    }
}

//------------------------------------------------------------------------------

ASTSymbol::ASTSymbol(const Anchor *anchor, Symbol _name, const Type *_type, bool _variadic)
    : ASTValue(ASTK_Symbol, anchor), name(_name), variadic(_variadic) {
    if (_type) set_type(_type);
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

Call::Call(const Anchor *anchor, ASTNode *_callee, const ASTNodes &_args)
    : ASTNode(ASTK_Call, anchor), callee(_callee), args(_args), flags(0) {
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

Call *Call::from(const Anchor *anchor, ASTNode *callee, const ASTNodes &args) {
    return new Call(anchor, callee, args);
}

//------------------------------------------------------------------------------

Let::Let(const Anchor *anchor, const ASTSymbols &_params, const ASTNodes &_args)
    : ASTNode(ASTK_Let, anchor), params(_params), args(_args) {
}
Let *Let::from(const Anchor *anchor, const ASTSymbols &params, const ASTNodes &args) {
    return new Let(anchor, params, args);
}

//------------------------------------------------------------------------------

Loop::Loop(const Anchor *anchor, const ASTSymbols &_params, const ASTNodes &_args, ASTNode *_value)
    : ASTNode(ASTK_Loop, anchor), params(_params), args(_args), value(_value) {
}

Loop *Loop::from(const Anchor *anchor, const ASTSymbols &params, const ASTNodes &args, ASTNode *value) {
    return new Loop(anchor, params, args, value);
}

//------------------------------------------------------------------------------

bool Const::classof(const ASTNode *T) {
    switch(T->kind()) {
    case ASTK_ConstInt:
    case ASTK_ConstReal:
    case ASTK_ConstTuple:
    case ASTK_ConstArray:
    case ASTK_ConstVector:
    case ASTK_ConstPointer:
        return true;
    default: return false;
    }
}

Const::Const(ASTKind _kind, const Anchor *anchor, const Type *type)
    : ASTValue(_kind, anchor) {
    set_type(type);
}

//------------------------------------------------------------------------------

ConstInt::ConstInt(const Anchor *anchor, const Type *type, uint64_t _value)
    : Const(ASTK_ConstInt, anchor, type), value(_value) {
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
    : Const(ASTK_ConstReal, anchor, type), value(_value) {}

ConstReal *ConstReal::from(const Anchor *anchor, const Type *type, double value) {
    return new ConstReal(anchor, type, value);
}

//------------------------------------------------------------------------------

ConstTuple::ConstTuple(const Anchor *anchor, const Type *type, const Constants &_fields)
    : Const(ASTK_ConstTuple, anchor, type), values(_fields) {
}

ConstTuple *ConstTuple::from(const Anchor *anchor, const Type *type, const Constants &fields) {
    return new ConstTuple(anchor, type, fields);
}

ConstTuple *ConstTuple::none_from(const Anchor *anchor) {
    return from(anchor, TYPE_Nothing, {});
}

//------------------------------------------------------------------------------

ConstArray::ConstArray(const Anchor *anchor, const Type *type, const Constants &_fields)
    : Const(ASTK_ConstTuple, anchor, type), values(_fields) {
}

ConstArray *ConstArray::from(const Anchor *anchor, const Type *type, const Constants &fields) {
    return new ConstArray(anchor, type, fields);
}

//------------------------------------------------------------------------------

ConstVector::ConstVector(const Anchor *anchor, const Type *type, const Constants &_fields)
    : Const(ASTK_ConstTuple, anchor, type), values(_fields) {
}

ConstVector *ConstVector::from(const Anchor *anchor, const Type *type, const Constants &fields) {
    return new ConstVector(anchor, type, fields);
}

//------------------------------------------------------------------------------

ConstPointer::ConstPointer(const Anchor *anchor, const Type *type, const void *_pointer)
    : Const(ASTK_ConstPointer, anchor, type), value(_pointer) {}

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

ConstPointer *ConstPointer::ast_from(const Anchor *anchor, ASTNode *node) {
    return from(anchor, TYPE_ASTNode, node);
}

ConstPointer *ConstPointer::list_from(const Anchor *anchor, const List *list) {
    return from(anchor, TYPE_List, list);
}

//------------------------------------------------------------------------------

Break::Break(const Anchor *anchor, ASTNode *_value)
    : ASTNode(ASTK_Break, anchor), value(_value) {
}

Break *Break::from(const Anchor *anchor, ASTNode *value) {
    return new Break(anchor, value);
}

//------------------------------------------------------------------------------

Repeat::Repeat(const Anchor *anchor, const ASTNodes &_args)
    : ASTNode(ASTK_Repeat, anchor), args(_args) {}

Repeat *Repeat::from(const Anchor *anchor, const ASTNodes &args) {
    return new Repeat(anchor, args);
}

//------------------------------------------------------------------------------

ASTReturn::ASTReturn(const Anchor *anchor, ASTNode *_value)
    : ASTNode(ASTK_Return, anchor), value(_value) {}

ASTReturn *ASTReturn::from(const Anchor *anchor, ASTNode *value) {
    return new ASTReturn(anchor, value);
}

//------------------------------------------------------------------------------

SyntaxExtend::SyntaxExtend(const Anchor *anchor, Template *_func, const List *_next, Scope *_env)
    : ASTNode(ASTK_SyntaxExtend, anchor), func(_func), next(_next), env(_env) {
}

SyntaxExtend *SyntaxExtend::from(const Anchor *anchor, Template *func, const List *next, Scope *env) {
    return new SyntaxExtend(anchor, func, next, env);
}

//------------------------------------------------------------------------------

ASTKind ASTNode::kind() const { return _kind; }

ASTNode::ASTNode(ASTKind kind, const Anchor *anchor)
    : _kind(kind),_type(nullptr),_anchor(anchor) {
    assert(_anchor);
}

bool ASTNode::is_typed() const {
    return _type != nullptr;
}
void ASTNode::set_type(const Type *type) {
    assert(!is_typed());
    _type = type;
}

void ASTNode::change_type(const Type *type) {
    assert(is_typed());
    _type = type;
}

const Type *ASTNode::get_type() const {
    assert(_type);
    return _type;
}

const Anchor *ASTNode::anchor() const {
    return _anchor;
}

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const ASTNode *T) { \
        return T->kind() == NAME; \
    }
SCOPES_AST_KIND()
#undef T

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, ASTNode *node) {
    stream_ast(ost, node, StreamASTFormat());
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const ASTNode *node) {
    stream_ast(ost, node, StreamASTFormat());
    return ost;
}

} // namespace scopes

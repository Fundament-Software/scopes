
/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "value.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "types.hpp"
#include "stream_expr.hpp"
#include "dyn_cast.inc"
#include "qualifier.inc"
#include "qualifiers.hpp"
#include "hash.hpp"
#include "anchor.hpp"
#include "prover.hpp"

#include <assert.h>
#include <unordered_set>

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

#define T(CLASS) \
    const Anchor *CLASS::def_anchor() const { \
        if (!_def_anchor) return unknown_anchor(); \
        return _def_anchor; \
    } \
    void CLASS::set_def_anchor(const Anchor *anchor) { \
        _def_anchor = anchor; \
    } \


SCOPES_DEFINED_VALUES()
#undef T

//------------------------------------------------------------------------------

std::size_t ValueIndex::Hash::operator()(const ValueIndex & s) const {
    return hash2(std::hash<Value *>{}(s.value.unref()), s.index);
}

ValueIndex::ValueIndex(const TypedValueRef &_value, int _index)
    : value(_value), index(_index) {
repeat:
    switch(value->kind()) {
    case VK_Keyed: {
        value = value.cast<Keyed>()->value;
        goto repeat;
    } break;
    case VK_ArgumentList: {
        auto al = value.cast<ArgumentList>();
        if (index < al->values.size()) {
            value = al->values[index];
            index = 0;
            goto repeat;
        } else {
            assert(false);
        }
    } break;
    case VK_ExtractArgument: {
        auto ea = value.cast<ExtractArgument>();
        if (index == 0) {
            value = ea->value;
            index = ea->index;
            goto repeat;
        } else {
            assert(false);
        }
    } break;
    default: break;
    }
}

bool ValueIndex::operator ==(const ValueIndex &other) const {
    return value.unref() == other.value.unref() && index == other.index;
}

const Type *ValueIndex::get_type() const {
    return get_argument(value->get_type(), index);
}

//------------------------------------------------------------------------------

KeyedTemplate::KeyedTemplate(Symbol _key, const ValueRef &node)
    : UntypedValue(VK_KeyedTemplate), key(_key), value(node)
{
}

ValueRef KeyedTemplate::from(Symbol key, ValueRef node) {
    assert(node);
    if (node.isa<TypedValue>()) {
        return Keyed::from(key, node.cast<TypedValue>());
    } else {
        if (node.isa<KeyedTemplate>()) {
            node = node.cast<KeyedTemplate>()->value;
        }
        return ref(node.anchor(), new KeyedTemplate(key, node));
    }
}

//------------------------------------------------------------------------------

Keyed::Keyed(const Type *type, Symbol _key, const TypedValueRef &node)
    : TypedValue(VK_Keyed, type), key(_key), value(node)
{
}

TypedValueRef Keyed::from(Symbol key, TypedValueRef node) {
    assert(node);
    if (node.isa<Keyed>()) {
        node = node.cast<Keyed>()->value;
    }
    auto T = node->get_type();
    auto NT = key_type(key, T);
    if (T == NT)
        return node;
    if (node.isa<Pure>()) {
        return PureCast::from(NT, node.cast<Pure>());
    } else {
        return ref(node.anchor(), new Keyed(NT, key, node));
    }
}

//------------------------------------------------------------------------------

// when joining two argument lists, all but the last argument can be flattened

ArgumentListTemplate::ArgumentListTemplate(const Values &values)
    : UntypedValue(VK_ArgumentListTemplate), _values(values) {
}

/*
bool ArgumentListTemplate::is_constant() const {
    for (auto val : _values) {
        assert(val);
        if (!val.isa<Const>())
            return false;
    }
    return true;
}
*/

bool ArgumentListTemplate::empty() const {
    return _values.empty();
}

const Values &ArgumentListTemplate::values() const {
    return _values;
}

ValueRef ArgumentListTemplate::from(const Values &values) {
    if (values.size() == 1) {
        return values[0];
    }
    bool is_typed = true;
    for (auto value : values) {
        if (value.isa<UntypedValue>()) {
            is_typed = false;
            break;
        }
    }
    if (is_typed) {
        // all values are typed - promote to ArgumentList
        TypedValues typed_values;
        typed_values.reserve(values.size());
        for (auto value : values) {
            typed_values.push_back(value.cast<TypedValue>());
        }
        return ref(unknown_anchor(), ArgumentList::from(typed_values));
    } else {
        #if 0
        assert(!values.empty());
        Values newvalues;
        newvalues.reserve(values.size());
        int count = (int)values.size() - 1;
        // truncate all but last argument
        for (int i = 0; i < count; ++i) {
            auto &&arg = values[i];
            if (arg.isa<ArgumentListTemplate>() || arg.isa<ArgumentList>()) {
                newvalues.push_back(ref(arg.anchor(), ExtractArgumentTemplate::from(arg, 0)));
            } else {
                newvalues.push_back(arg);
            }
        }
        auto &&arg = values.back();
        /*if (arg.isa<ArgumentListTemplate>()) {
            for (auto &&val : (arg.cast<ArgumentListTemplate>())->values()) {
                newvalues.push_back(val);
            }
        } else if (arg.isa<ArgumentList>()) {
            auto al = arg.cast<ArgumentList>();
            for (auto &&val : al->values) {
                newvalues.push_back(val);
            }
        } else*/ {
            newvalues.push_back(arg);
        }
        #endif
        return ref(unknown_anchor(), new ArgumentListTemplate(values));
    }
}

//------------------------------------------------------------------------------

static TypedValueRef _empty_argument_list;

const Type *arguments_type_from_typed_values(const TypedValues &_values) {
    Types types;
    for (auto value : _values) {
        types.push_back(value->get_type());
    }
    return arguments_type(types);
}

ArgumentList::ArgumentList(const TypedValues &_values)
    : TypedValue(VK_ArgumentList, arguments_type_from_typed_values(_values)) {
    values.reserve(get_argument_count(get_type()));
    int idx = 1;
    for (auto value : _values) {
        if (value.isa<ArgumentList>()) {
            auto al = value.cast<ArgumentList>();
            for (auto &&value : al->values) {
                values.push_back(value);
                if (idx != _values.size())
                    break;
            }
        } else {
            values.push_back(value);
        }
        idx++;
    }
    assert(get_argument_count(get_type()) == values.size());
}

bool ArgumentList::is_constant() const {
    for (auto val : values) {
        assert(val);
        if (!val.isa<Const>())
            return false;
    }
    return true;
}

TypedValueRef ArgumentList::from(const TypedValues &values) {
    if (values.size() == 0) {
        if (!_empty_argument_list)
            _empty_argument_list = ref(unknown_anchor(), new ArgumentList(values));
        return _empty_argument_list;
    } else if (values.size() == 1) {
        return values[0];
    }
    return ref(unknown_anchor(), new ArgumentList(values));
}

//------------------------------------------------------------------------------

ExtractArgumentTemplate::ExtractArgumentTemplate(const ValueRef &_value, int _index, bool _vararg)
    : UntypedValue(VK_ExtractArgumentTemplate), index(_index), value(_value), vararg(_vararg) {
    assert(index >= 0);
}

ValueRef ExtractArgumentTemplate::from(
    const ValueRef &_value, int index, bool vararg) {
    ValueRef value = _value;
//repeat:
    if (value.isa<TypedValue>()) {
        if (vararg) {
            return ExtractArgument::variadic_from(value.cast<TypedValue>(), index);
        } else {
            return ExtractArgument::from(value.cast<TypedValue>(), index);
        }
    }
    // doesn't work right: we might be un-singling mrv's
    /* else if (value.isa<ArgumentListTemplate>()) {
        auto al = value.cast<ArgumentListTemplate>();
        auto &&values = al->values();
        if (vararg) {
            if ((index == 0) || values.empty()) {
                return value;
            }
        } else {
            if (values.empty()) {
                return ref(value.anchor(), ConstAggregate::none_from());
            }
            if ((index + 1) < values.size()) {
                // we can extract directly
                return values[index];
            }
            auto lastkind = values.back()->kind();
            if ((lastkind == VK_ArgumentListTemplate) || (lastkind == VK_ArgumentList)) {
                // continue searching in next list
                value = values.back();
                index = index + 1 - (int)values.size();
                goto repeat;
            }
        }
    } else if (!vararg && value.isa<ExtractArgumentTemplate>()) {
        auto eat = value.cast<ExtractArgumentTemplate>();
        if (eat->vararg) {
            value = eat->value;
            index += eat->index;
            goto repeat;
        } else if (index == 0) {
            return value;
        } else {
            return ref(value.anchor(), ConstAggregate::none_from());
        }
    }
    */
    return ref(value.anchor(),
        new ExtractArgumentTemplate(value, index, vararg));
}

//------------------------------------------------------------------------------

ExtractArgument::ExtractArgument(const Type *type, const TypedValueRef &_value, int _index)
    : TypedValue(VK_ExtractArgument, type),
        index(_index), value(_value) {
}

TypedValueRef ExtractArgument::variadic_from(const TypedValueRef &value, int index) {
    if (!index) return value;
    auto T = value->get_type();
    if (!is_returning(T)) return value;
    int count = get_argument_count(T);
    TypedValues values;
    if (value.isa<ArgumentList>()) {
        auto al = value.cast<ArgumentList>();
        for (int i = index; i < count; ++i) {
            values.push_back(al->values[i]);
        }
    } else {
        for (int i = index; i < count; ++i) {
            //auto argT = get_argument(T, i);
            values.push_back(from(value, i));
        }
    }
    return ref(value.anchor(), ArgumentList::from(values));
}

TypedValueRef ExtractArgument::from(const TypedValueRef &value, int index) {
    auto T = value->get_type();
    if (!is_returning(T)) return value;
    int count = get_argument_count(T);
    if (index >= count)
        return ref(value.anchor(), ConstAggregate::none_from());
    if (value.isa<ArgumentList>()) {
        auto al = value.cast<ArgumentList>();
        assert (index < al->values.size());
        return al->values[index];
    } else if ((count == 1) && (index == 0)) {
        return value;
    } else {
        auto argT = get_argument(T, index);
        return ref(value.anchor(),
            new ExtractArgument(argT, value, index));
    }
}

//------------------------------------------------------------------------------

Template::Template(Symbol _name, const ParameterTemplates &_params, const ValueRef &_value)
    : UntypedValue(VK_Template),
        name(_name), params(_params), value(_value),
        _is_inline(false), docstring(nullptr),
        recursion(0) {
    int index = 0;
    for (auto param : params) {
        param->set_owner(ref(unknown_anchor(), this), index++);
    }
}

bool Template::is_forward_decl() const {
    return !value;
}

void Template::set_inline() {
    _is_inline = true;
}

bool Template::is_inline() const {
    return _is_inline;
}

bool Template::is_hidden() const {
    return is_inline() && name == SYM_HiddenInline;
}

void Template::append_param(const ParameterTemplateRef &sym) {
    sym->set_owner(ref(unknown_anchor(), this), params.size());
    params.push_back(sym);
}

TemplateRef Template::from(Symbol name,
    const ParameterTemplates &params, const ValueRef &value) {
    return ref(unknown_anchor(), new Template(name, params, value));
}

//------------------------------------------------------------------------------

Function::UniqueInfo::UniqueInfo(const ValueIndex& _value)
    : value(_value) {
}

int Function::UniqueInfo::get_depth() const {
    return value.value->get_depth();
}

Function::Function(Symbol _name, const Parameters &_params)
    : Pure(VK_Function, TYPE_Unknown),
        name(_name), params(_params),
        docstring(nullptr),
        frame(FunctionRef()),
        boundary(FunctionRef()),
        original(TemplateRef()),
        label(LabelRef()),
        complete(false),
        nextid(FirstUniquePrivate),
        returning_hint(TYPE_NoReturn),
        raising_hint(TYPE_NoReturn),
        returning_anchor(nullptr),
        raising_anchor(nullptr) {
    body.depth = 1;
    int index = 0;
    for (auto param : params) {
        param->set_owner(ref(unknown_anchor(), this), index++);
    }
}

const Function::UniqueInfo &Function::get_unique_info(int id) const {
    assert(id != 0);
    assert(id != GlobalUnique);
    assert(id < nextid);
    auto it = uniques.find(id);
    assert(it != uniques.end());
    return it->second;
}

int Function::unique_id() {
    return nextid++;
}

void Function::try_bind_unique(const TypedValueRef &value) {
    auto T = value->get_type();
    int count = get_argument_count(T);
    for (int i = 0; i < count; ++i) {
        auto argT = get_argument(T, i);
        if (is_unique(argT)) {
            bind_unique(UniqueInfo(ValueIndex(value, i)));
        }
    }
}

void Function::bind_unique(const UniqueInfo &info) {
    auto uq = get_unique(info.value.get_type());
    auto result = uniques.insert({uq->id, info});
    if (!result.second) {
        StyledStream ss;
        ss << "internal error: duplicate unique " << uq->id;
        ss << " (was " << result.first->second.value;
        ss << ", is " << info.value << ")" << std::endl;
    }
    assert(result.second);
}

bool Function::key_equal(const Function *other) const {
    return this == other;
}

std::size_t Function::hash() const {
    return std::hash<const Function *>{}(this);
}

bool Function::is_typed() const {
    assert(_type);
    return _type != TYPE_Unknown;
}

void Function::set_type(const Type *type) {
    assert(!is_typed());
    assert(type);
    _type = type;
}

void Function::change_type(const Type *type) {
    assert(is_typed());
    assert(type);
    _type = type;
}

const Anchor *Function::get_best_mover_anchor(int id) {
    auto it = movers.find(id);
    if (it != movers.end()) {
        return it->second.anchor();
    } else {
        auto info = get_unique_info(id);
        return info.value.value.anchor();
    }
}

void Function::hint_mover(int id, const ValueRef &where) {
    auto result = movers.insert({ id, where });
    if (!result.second)
        result.second = where;
}

void Function::build_valids() {
    assert(valid.empty());
    // add uniques
    for (auto sym : params) {
        auto T = sym->get_type();
        int count = get_argument_count(T);
        for (int i = 0; i < count; ++i) {
            auto argT = get_argument(T, i);
            auto uq = try_unique(argT);
            if (uq) {
                assert(uq->id);
                bind_unique(UniqueInfo(ValueIndex(sym, i)));
                valid.insert(uq->id);
            }
        }
    }
    // add views
    for (auto sym : params) {
        auto T = sym->get_type();
        int count = get_argument_count(T);
        for (int i = 0; i < count; ++i) {
            auto argT = get_argument(T, i);
            auto vq = try_view(argT);
            if (vq) {
                for (auto id : vq->ids) {
                    assert(id);
                    if (!valid.count(id)) {
                        auto result = uniques.insert({id,
                            UniqueInfo(ValueIndex(sym, i))});
                        assert(result.second);
                        valid.insert(id);
                    }
                }
            }
        }
    }
    original_valid = valid;
}

void Function::append_param(const ParameterRef &sym) {
    sym->set_owner(ref(unknown_anchor(), this), params.size());
    params.push_back(sym);
}

TypedValueRef Function::resolve_local(const ValueRef &node) const {
    auto it = map.find(node.unref());
    if (it == map.end())
        return TypedValueRef();
    return it->second;
}

TypedValueRef Function::unsafe_resolve(const ValueRef &node) const {
    auto fn = this;
    while (fn) {
        auto val = fn->resolve_local(node);
        if (val) return val;
        fn = fn->frame.unref();
    }
    return TypedValueRef();
}

SCOPES_RESULT(TypedValueRef) Function::resolve(const ValueRef &node, const FunctionRef &_boundary) const {
    SCOPES_RESULT_TYPE(TypedValueRef)
    auto fn = this;
    while (fn) {
        auto val = fn->resolve_local(node);
        if (val) {
            if ((fn->boundary != _boundary) && !val->is_accessible()) {
                SCOPES_TRACE_PROVE_ARG(node);
                SCOPES_ERROR(VariableOutOfScope, val->get_type(), val.anchor());
            }
            return val;
        }
        fn = fn->frame.unref();
    }
    return TypedValueRef();
}

void Function::bind(const ValueRef &oldnode, const TypedValueRef &newnode) {
    auto it = map.insert({oldnode.unref(), newnode});
    if (!it.second) {
        it.first->second = newnode;
    }
}

FunctionRef Function::from(Symbol name,
    const Parameters &params) {
    return ref(unknown_anchor(), new Function(name, params));
}

//------------------------------------------------------------------------------

static const Type *pointer_for_global_type(const Type *type, size_t flags, Symbol storage_class) {
    size_t ptrflags = 0;
    if (storage_class == SYM_SPIRV_StorageClassUniformConstant) {
        // regular uniforms are always readonly, even though the underlying sampler may have other flags
    } else {
        if (flags & GF_NonWritable)
            ptrflags |= PTF_NonWritable;
        else if (flags & GF_NonReadable)
            ptrflags |= PTF_NonReadable;
    }
    return pointer_type(type, ptrflags, storage_class);
}

Global::Global(const Type *type, Symbol _name, size_t _flags, Symbol _storage_class)
    : Pure(VK_Global, pointer_for_global_type(type, _flags, _storage_class)),
        element_type(type), name(_name), flags(_flags), storage_class(_storage_class) {
}

bool Global::key_equal(const Global *other) const {
    return this == other;
}

std::size_t Global::hash() const {
    return std::hash<const Global *>{}(this);
}

GlobalRef Global::from(const Type *type, Symbol name, size_t flags, Symbol storage_class) {
    if ((storage_class == SYM_SPIRV_StorageClassUniform)
        && !(flags & GF_BufferBlock)) {
        flags |= GF_Block;
    }
    return ref(unknown_anchor(), new Global(type, name, flags, storage_class));
}

//------------------------------------------------------------------------------

template<typename T>
struct ConstSet {
    struct Hash {
        std::size_t operator()(const T *k) const {
            return k->hash();
        }
    };

    struct Equal {
        std::size_t operator()(const T *self, const T *other) const {
            return self->key_equal(other);
        }
    };

    std::unordered_set<T *, Hash, Equal> map;

    template<typename ... Args>
    TValueRef<T> from(Args ... args) {
        T key(args ...);
        auto it = map.find(&key);
        if (it != map.end()) {
            return ref(unknown_anchor(), *it);
        }
        auto val = new T(args ...);
        map.insert(val);
        return ref(unknown_anchor(), val);
    }
};

//------------------------------------------------------------------------------

static ConstSet<GlobalString> globalstrings;

GlobalString::GlobalString(const String *str)
    : Pure(VK_GlobalString,
        refer_type(
            array_type(TYPE_I8, str->count).assert_ok(),
            PTF_NonWritable,
            SYM_SPIRV_StorageClassPrivate)),
        value(str)
{}

bool GlobalString::key_equal(const GlobalString *other) const {
    return String::KeyEqual{}(value, other->value);
}

std::size_t GlobalString::hash() const {
    return value->hash();
}

GlobalStringRef GlobalString::from(const String *str) {
    return globalstrings.from(str);
}

//------------------------------------------------------------------------------

PureCast::PureCast(const Type *type, const PureRef &_value)
    : Pure(VK_PureCast, type), value(_value) {}

bool PureCast::key_equal(const PureCast *other) const {
    return get_type() == other->get_type()
        && value == other->value;
}

std::size_t PureCast::hash() const {
    return value->hash();
}

PureRef PureCast::from(const Type *type, PureRef value) {
    if (value.isa<PureCast>()) {
        value = value.cast<PureCast>()->value;
    }
    if (value->get_type() == type)
        return value;
    if (value.isa<Undef>()) {
        return Undef::from(type);
    } else if (value.isa<Const>()
        && (storage_kind(type) == storage_kind(value->get_type()))) {
        ConstRef result;
        switch (value->kind()) {
        case VK_ConstInt: {
            auto node = value.cast<ConstInt>();
            result = ConstInt::from(type, node->words);
        } break;
        case VK_ConstReal: {
            auto node = value.cast<ConstReal>();
            result = ConstReal::from(type, node->value);
        } break;
        case VK_ConstAggregate: {
            auto node = value.cast<ConstAggregate>();
            result = ConstAggregate::from(type, node->values);
        } break;
        case VK_ConstPointer: {
            auto node = value.cast<ConstPointer>();
            result = ConstPointer::from(type, node->value);
        } break;
        default:
            assert (false && "unknown const kind");
            break;
        }
        return PureRef(value.anchor(), result);
    } else {
        return ref(value.anchor(), new PureCast(type, value));
    }
}

//------------------------------------------------------------------------------

bool Undef::key_equal(const Undef *other) const {
    return get_type() == other->get_type();
}

std::size_t Undef::hash() const {
    return std::hash<const Type *>{}(get_type());
}

Undef::Undef(const Type *_type)
    : Pure(VK_Undef, _type) {
}

UndefRef Undef::from(const Type *type) {
    return ref(unknown_anchor(), new Undef(type));
}

//------------------------------------------------------------------------------

Block::Block()
    : depth(-1), insert_index(0), tag_traceback(true), terminator(InstructionRef())
{}

bool Block::is_valid(const IDSet &ids) const {
    int _id = 0;
    return is_valid(ids, _id);
}

bool Block::is_valid(const ValueIndex &value) const {
    int _id = 0;
    return is_valid(value, _id);
}

bool Block::is_valid(const ValueIndex &value, int &_id) const {
    auto T = value.get_type();
    auto vq = try_view(T);
    if (vq) {
        return is_valid(vq->ids, _id);
    }
    auto uq = try_unique(T);
    if (uq) {
        if (!is_valid(uq->id)) {
            _id = uq->id;
            return false;
        }
    }
    return true;
}

bool Block::is_valid(const IDSet &ids, int &_id) const {
    for (auto id : ids) {
        if (!is_valid(id)) {
            _id = id;
            return false;
        }
    }
    return true;
}

bool Block::is_valid(int id) const {
    return (id == GlobalUnique)?true:valid.count(id);
}

void Block::move(int id) {
    assert(is_valid(id));
    valid.erase(id);
}

bool Block::is_terminated() const {
    return terminator.unref() != nullptr;
}

void Block::set_parent(Block *_parent) {
    assert(depth < 0);
    assert(_parent);
    depth = _parent->depth + 1;
    tag_traceback = _parent->tag_traceback;
    // copy valids from parent
    if (valid.empty()) {
        valid = _parent->valid;
    } else {
        // block has some preconfigured values
        valid = union_idset(valid, _parent->valid);
    }
}

void Block::clear() {
    body.clear();
    valid.clear();
    terminator = InstructionRef();
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
    // equivalent to removing all values no longer valid, and adding
    // all values that have since been added
    valid = source.valid;
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

void Block::append(const InstructionRef &node) {
    assert(!node->block);
    node->block = this;
    if (!is_returning(node->get_type())) {
        assert(!terminator);
        assert(insert_index == body.size());
        terminator = node;
    } else {
        auto T = node->get_type();
        auto count = get_argument_count(T);
        for (int i = 0; i < count; ++i) {
            auto uq = try_unique(get_argument(T, i));
            if (uq) {
                assert(uq->id);
                valid.insert(uq->id);
            }
        }
        body.insert(body.begin() + insert_index, node);
        insert_index++;
    }
}

Block::DataMap &Block::get_channel(Symbol name) {
    DataMap *&map = channels[name];
    if (!map) {
        map = new DataMap();
    }
    return *map;
}

//------------------------------------------------------------------------------

Expression::Expression(const Values &_body, const ValueRef &_value)
    : UntypedValue(VK_Expression), body(_body), value(_value), scoped(true) {
}

void Expression::append(const ValueRef &node) {
    assert(node);
    if (value && !value.isa<Pure>()) {
        body.push_back(value);
    }
    value = node;
}

ExpressionRef Expression::scoped_from(const Values &nodes, const ValueRef &value) {
    return ref(unknown_anchor(), new Expression(nodes, value));
}

ExpressionRef Expression::unscoped_from(const Values &nodes, const ValueRef &value) {
    auto expr = new Expression(nodes, value);
    expr->scoped = false;
    return ref(unknown_anchor(), expr);
}

//------------------------------------------------------------------------------

CondBr::CondBr(const TypedValueRef &_cond)
    : Instruction(VK_CondBr, TYPE_NoReturn), cond(_cond)
{}

CondBrRef CondBr::from(const TypedValueRef &cond) {
    validate_instruction(cond);
    return ref(unknown_anchor(), new CondBr(cond));
}

//------------------------------------------------------------------------------

bool If::Clause::is_then() const {
    return cond.unref() != nullptr;
}

If::If(const Clauses &_clauses)
    : UntypedValue(VK_If), clauses(_clauses) {
}

IfRef If::from(const Clauses &_clauses) {
    return ref(unknown_anchor(), new If(_clauses));
}

void If::append_then(const ValueRef &cond, const ValueRef &value) {
    assert(cond);
    assert(value);
    Clause clause;
    clause.cond = cond;
    clause.value = value;
    clauses.push_back(clause);
}

void If::append_else(const ValueRef &value) {
    assert(value);
    Clause clause;
    clause.value = value;
    clauses.push_back(clause);
}

//------------------------------------------------------------------------------

SwitchTemplate::SwitchTemplate(const ValueRef &_expr, const Cases &_cases)
    : UntypedValue(VK_SwitchTemplate), expr(_expr), cases(_cases)
{}

SwitchTemplateRef SwitchTemplate::from(const ValueRef &expr, const Cases &cases) {
    return ref(unknown_anchor(), new SwitchTemplate(expr, cases));
}

void SwitchTemplate::append_case(const ValueRef &literal, const ValueRef &value) {
    assert(literal);
    assert(value);
    Case _case;
    _case.anchor = value.anchor();
    _case.kind = CK_Case;
    _case.literal = literal;
    _case.value = value;
    cases.push_back(_case);
}

void SwitchTemplate::append_pass(const ValueRef &literal, const ValueRef &value) {
    assert(literal);
    assert(value);
    Case _case;
    _case.anchor = value.anchor();
    _case.kind = CK_Pass;
    _case.literal = literal;
    _case.value = value;
    cases.push_back(_case);
}

void SwitchTemplate::append_do(const ValueRef &value) {
    assert(value);
    Case _case;
    _case.anchor = value.anchor();
    _case.kind = CK_Do;
    _case.value = value;
    cases.push_back(_case);
}

void SwitchTemplate::append_default(const ValueRef &value) {
    assert(value);
    Case _case;
    _case.anchor = value.anchor();
    _case.kind = CK_Default;
    _case.value = value;
    cases.push_back(_case);
}

//------------------------------------------------------------------------------

Switch::Switch(const TypedValueRef &_expr, const Cases &_cases)
    : Instruction(VK_Switch, TYPE_NoReturn), expr(_expr), cases(_cases)
{}

SwitchRef Switch::from(const TypedValueRef &expr, const Cases &cases) {
    validate_instruction(expr);
    return ref(unknown_anchor(), new Switch(expr, cases));
}

Switch::Case &Switch::append_pass(const Anchor *anchor, const ConstIntRef &literal) {
    assert(literal);
    Case *_case = new Case();
    _case->anchor = anchor;
    _case->kind = CK_Pass;
    _case->literal = literal;
    cases.push_back(_case);
    return *cases.back();
}

Switch::Case &Switch::append_default(const Anchor *anchor) {
    Case *_case = new Case();
    _case->anchor = anchor;
    _case->kind = CK_Default;
    cases.push_back(_case);
    return *cases.back();
}

//------------------------------------------------------------------------------

ParameterTemplate::ParameterTemplate(Symbol _name, bool _variadic)
    : UntypedValue(VK_ParameterTemplate), name(_name), variadic(_variadic),
        owner(TemplateRef()), index(-1) {
}

ParameterTemplateRef ParameterTemplate::from(Symbol name) {
    return ref(unknown_anchor(), new ParameterTemplate(name, false));
}

ParameterTemplateRef ParameterTemplate::variadic_from(Symbol name) {
    return ref(unknown_anchor(), new ParameterTemplate(name, true));
}

bool ParameterTemplate::is_variadic() const {
    return variadic;
}

void ParameterTemplate::set_owner(const TemplateRef &_owner, int _index) {
    assert(!owner);
    owner = _owner;
    index = _index;
}

//------------------------------------------------------------------------------

Parameter::Parameter(Symbol _name, const Type *_type)
    : TypedValue(VK_Parameter, _type), name(_name),
        owner(FunctionRef()), block(nullptr), index(-1) {
}

ParameterRef Parameter::from(Symbol name, const Type *type) {
    return ref(unknown_anchor(), new Parameter(name, type));
}

void Parameter::set_owner(const FunctionRef &_owner, int _index) {
    assert(!owner);
    owner = _owner;
    index = _index;
}

void Parameter::retype(const Type *T) {
    _type = T;
}

//------------------------------------------------------------------------------

LoopArguments::LoopArguments(const LoopRef &_loop)
    : UntypedValue(VK_LoopArguments), loop(_loop) {}

LoopArgumentsRef LoopArguments::from(const LoopRef &loop) {
    return ref(unknown_anchor(), new LoopArguments(loop));
}

//------------------------------------------------------------------------------

LoopLabelArguments::LoopLabelArguments(const Type *type)
    : TypedValue(VK_LoopLabelArguments, type), loop(LoopLabelRef()) {}

LoopLabelArgumentsRef LoopLabelArguments::from(const Type *type) {
    return ref(unknown_anchor(), new LoopLabelArguments(type));
}

//------------------------------------------------------------------------------

Exception::Exception(const Type *type)
    : TypedValue(VK_Exception, type) {
}

ExceptionRef Exception::from(const Type *type) {
    return ref(unknown_anchor(), new Exception(type));
}

//------------------------------------------------------------------------------

CallTemplate::CallTemplate(const ValueRef &_callee, const Values &_args)
    : UntypedValue(VK_CallTemplate), callee(_callee), args(_args), flags(0) {
}

bool CallTemplate::is_rawcall() const {
    return flags & CF_RawCall;
}

void CallTemplate::set_rawcall() {
    flags |= CF_RawCall;
}

CallTemplateRef CallTemplate::from(const ValueRef &callee, const Values &args) {
    return ref(unknown_anchor(), new CallTemplate(callee, args));
}

//------------------------------------------------------------------------------

static const Type *get_cmp_type(const Type *T) {
    return bool_op_return_type(T).assert_ok();
}

ICmp::ICmp(ICmpKind _cmp_kind, const TypedValueRef &_value1, const TypedValueRef &_value2)
    : Instruction(VK_ICmp, get_cmp_type(_value1->get_type())), cmp_kind(_cmp_kind), value1(_value1), value2(_value2) {}
ICmpRef ICmp::from(ICmpKind cmp_kind, const TypedValueRef &value1, const TypedValueRef &value2) {
    return ref(unknown_anchor(), new ICmp(cmp_kind, value1, value2));
}

FCmp::FCmp(FCmpKind _cmp_kind, const TypedValueRef &_value1, const TypedValueRef &_value2)
    : Instruction(VK_FCmp, get_cmp_type(_value1->get_type())), cmp_kind(_cmp_kind), value1(_value1), value2(_value2) {}
FCmpRef FCmp::from(FCmpKind cmp_kind, const TypedValueRef &value1, const TypedValueRef &value2) {
    return ref(unknown_anchor(), new FCmp(cmp_kind, value1, value2));
}

//------------------------------------------------------------------------------

static const Type *get_unop_type(UnOpKind _op, const Type *T) {
    switch(_op) {
    case UnOpLength: {
        auto ST = storage_type(T).assert_ok();
        if (isa<VectorType>(ST)) {
            return cast<VectorType>(ST)->element_type;
        }
    } break;
    default: break;
    }
    return T;
}

UnOp::UnOp(UnOpKind _op, const TypedValueRef &_value)
    : Instruction(VK_UnOp, get_unop_type(_op, _value->get_type())), op(_op), value(_value) {}
UnOpRef UnOp::from(UnOpKind op, const TypedValueRef &value) {
    return ref(unknown_anchor(), new UnOp(op, value));
}

BinOp::BinOp(BinOpKind _op, const TypedValueRef &_value1, const TypedValueRef &_value2)
    : Instruction(VK_BinOp, _value1->get_type()), op(_op), value1(_value1), value2(_value2) {}
BinOpRef BinOp::from(BinOpKind op, const TypedValueRef &value1, const TypedValueRef &value2) {
    return ref(unknown_anchor(), new BinOp(op, value1, value2));
}

TriOp::TriOp(TriOpKind _op, const TypedValueRef &_value1, const TypedValueRef &_value2, const TypedValueRef &_value3)
    : Instruction(VK_TriOp, _value1->get_type()), op(_op), value1(_value1), value2(_value2), value3(_value3) {}
TriOpRef TriOp::from(TriOpKind op, const TypedValueRef &value1, const TypedValueRef &value2, const TypedValueRef &value3) {
    return ref(unknown_anchor(), new TriOp(op, value1, value2, value3));
}

//------------------------------------------------------------------------------

static const ImageType *sampler_image_type(const Type *T) {
    auto ST = storage_type(T).assert_ok();
    if (ST->kind() == TK_SampledImage) {
        auto sit = cast<SampledImageType>(ST);
        ST = storage_type(sit->type).assert_ok();
    }
    return cast<ImageType>(ST);
}

static const Type *sampler_comp_type(const Type *T) {
    return sampler_image_type(T)->type;
}

Sample::Sample(const TypedValueRef &_sampler, const TypedValueRef &_coords, const std::vector<Option> &_options)
    : Instruction(VK_Sample, sampler_comp_type(_sampler->get_type())), sampler(_sampler), coords(_coords), options(_options) {}
SampleRef Sample::from(const TypedValueRef &sampler, const TypedValueRef &coords, const std::vector<Option> &options) {
    return ref(unknown_anchor(), new Sample(sampler, coords, options));
}

static const Type *sampler_comp_size_type(const Type *T) {
    auto it = sampler_image_type(T);
    int comps = 0;
    switch(it->dim.value()) {
    case SYM_SPIRV_Dim1D:
    case SYM_SPIRV_DimBuffer:
        comps = 1;
        break;
    case SYM_SPIRV_Dim2D:
    case SYM_SPIRV_DimCube:
    case SYM_SPIRV_DimRect:
    case SYM_SPIRV_DimSubpassData:
        comps = 2;
        break;
    case SYM_SPIRV_Dim3D:
        comps = 3;
        break;
    default: assert(false); break;
    }
    if (it->arrayed) {
        comps++;
    }
    const Type *retT = TYPE_I32;
    if (comps != 1) {
        return vector_type(TYPE_I32, comps).assert_ok();
    } else {
        return retT;
    }
}

ImageQuerySize::ImageQuerySize(const TypedValueRef &_sampler, const TypedValueRef &_lod)
    : Instruction(VK_ImageQuerySize, sampler_comp_size_type(_sampler->get_type())), sampler(_sampler), lod(_lod) {}
bool ImageQuerySize::has_lod() const {
    return lod;
}
ImageQuerySizeRef ImageQuerySize::from(const TypedValueRef &sampler) {
    return ref(unknown_anchor(), new ImageQuerySize(sampler, TypedValueRef()));
}
ImageQuerySizeRef ImageQuerySize::from(const TypedValueRef &sampler, const TypedValueRef &lod) {
    return ref(unknown_anchor(), new ImageQuerySize(sampler, lod));
}

ImageQueryLod::ImageQueryLod(const TypedValueRef &_sampler, const TypedValueRef &_coords)
    : Instruction(VK_ImageQueryLod, vector_type(TYPE_F32, 2).assert_ok()), sampler(_sampler), coords(_coords) {}
ImageQueryLodRef ImageQueryLod::from(const TypedValueRef &sampler, const TypedValueRef &coords) {
    return ref(unknown_anchor(), new ImageQueryLod(sampler, coords));
}

ImageQueryLevels::ImageQueryLevels(const TypedValueRef &_sampler)
    : Instruction(VK_ImageQueryLevels, TYPE_I32), sampler(_sampler) {}
ImageQueryLevelsRef ImageQueryLevels::from(const TypedValueRef &sampler) {
    return ref(unknown_anchor(), new ImageQueryLevels(sampler));
}

ImageQuerySamples::ImageQuerySamples(const TypedValueRef &_sampler)
    : Instruction(VK_ImageQuerySamples, TYPE_I32), sampler(_sampler) {}
ImageQuerySamplesRef ImageQuerySamples::from(const TypedValueRef &sampler) {
    return ref(unknown_anchor(), new ImageQuerySamples(sampler));
}

ImageRead::ImageRead(const TypedValueRef &_image, const TypedValueRef &_coords)
    : Instruction(VK_ImageRead, sampler_comp_type(_image->get_type())), image(_image), coords(_coords) {}
ImageReadRef ImageRead::from(const TypedValueRef &image, const TypedValueRef &coords) {
    return ref(unknown_anchor(), new ImageRead(image, coords));
}

ImageWrite::ImageWrite(const TypedValueRef &_image, const TypedValueRef &_coords, const TypedValueRef &_texel)
    : Instruction(VK_ImageWrite, empty_arguments_type()), image(_image), coords(_coords), texel(_texel) {}
ImageWriteRef ImageWrite::from(const TypedValueRef &image, const TypedValueRef &coords, const TypedValueRef &texel) {
    return ref(unknown_anchor(), new ImageWrite(image, coords, texel));
}

ExecutionMode::ExecutionMode(Symbol _mode, int v0, int v1, int v2)
    : Instruction(VK_ExecutionMode, empty_arguments_type()), mode(_mode) {
    values[0] = v0;
    values[1] = v1;
    values[2] = v2;
}
ExecutionModeRef ExecutionMode::from(Symbol mode) {
    return ref(unknown_anchor(), new ExecutionMode(mode,-1,-1,-1));
}
ExecutionModeRef ExecutionMode::from(Symbol mode, int v0) {
    return ref(unknown_anchor(), new ExecutionMode(mode,v0,-1,-1));
}
ExecutionModeRef ExecutionMode::from(Symbol mode, int v0, int v1) {
    return ref(unknown_anchor(), new ExecutionMode(mode,v0,v1,-1));
}
ExecutionModeRef ExecutionMode::from(Symbol mode, int v0, int v1, int v2) {
    return ref(unknown_anchor(), new ExecutionMode(mode,v0,v1,v2));
}

//------------------------------------------------------------------------------

Annotate::Annotate(const TypedValues &_values)
    : Instruction(VK_Annotate, empty_arguments_type()), values(_values) {}
AnnotateRef Annotate::from(const TypedValues &values) {
    return ref(unknown_anchor(), new Annotate(values));
}

//------------------------------------------------------------------------------

Select::Select(const TypedValueRef &_cond,
    const TypedValueRef &_value1, const TypedValueRef &_value2)
    : Instruction(VK_Select, _value2->get_type()), cond(_cond), value1(_value1), value2(_value2)
{}

SelectRef Select::from(const TypedValueRef &cond,
    const TypedValueRef &value1, const TypedValueRef &value2) {
    return ref(unknown_anchor(), new Select(cond, value1, value2));
}

//------------------------------------------------------------------------------

static const Type *get_element_pointer_type(const Type *T, const TypedValues &indices) {
    T = storage_type(T).assert_ok();
    assert(isa<PointerType>(T));
    auto pi = cast<PointerType>(T);
    T = pi->element_type;
    int count = indices.size();
    for (int i = 1; i < count; ++i) {
        const Type *ST = storage_type(T).assert_ok();
        switch(ST->kind()) {
        case TK_Array: {
            auto ai = cast<ArrayType>(ST);
            T = ai->element_type;
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(ST);
            T = vi->element_type;
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(ST);
            assert(indices[i].isa<ConstInt>());
            auto idx = indices[i].cast<ConstInt>()->msw();
            T = type_key(ti->type_at_index(idx).assert_ok())._1;
        } break;
        default: {
            assert(false);
        } break;
        }
    }
    return pointer_type(T, pi->flags, pi->storage_class);
}

GetElementPtr::GetElementPtr(const TypedValueRef &_value, const TypedValues &_indices)
    : Instruction(VK_GetElementPtr, get_element_pointer_type(_value->get_type(), _indices)),
    value(_value), indices(_indices) {}

GetElementPtrRef GetElementPtr::from(const TypedValueRef &value, const TypedValues &indices) {
    return ref(unknown_anchor(), new GetElementPtr(value, indices));
}

//------------------------------------------------------------------------------

static const Type *value_type_at_index(const Type *T, int index) {
    T = storage_type(T).assert_ok();
    switch(T->kind()) {
    case TK_Pointer: return cast<PointerType>(T)->element_type;
    case TK_Array: return cast<ArrayType>(T)->element_type;
    case TK_Vector: return cast<VectorType>(T)->element_type;
    case TK_Tuple: return type_key(cast<TupleType>(T)->type_at_index(index).assert_ok())._1;
    default: {
        assert(false);
        return nullptr;
    } break;
    }
}

ExtractValue::ExtractValue(const TypedValueRef &_value, uint32_t _index)
    : Instruction(VK_ExtractValue,
        value_type_at_index(_value->get_type(), _index)),
    value(_value), index(_index) {}

ExtractValueRef ExtractValue::from(const TypedValueRef &value, uint32_t index) {
    return ref(unknown_anchor(), new ExtractValue(value, index));
}

//------------------------------------------------------------------------------

InsertValue::InsertValue(const TypedValueRef &_value, const TypedValueRef &_element, uint32_t _index)
    : Instruction(VK_InsertValue, _value->get_type()),
        value(_value), element(_element), index(_index) {}

InsertValueRef InsertValue::from(const TypedValueRef &value, const TypedValueRef &element, uint32_t index) {
    return ref(unknown_anchor(), new InsertValue(value, element, index));
}

//------------------------------------------------------------------------------

ExtractElement::ExtractElement(const TypedValueRef &_value, const TypedValueRef &_index)
    : Instruction(VK_ExtractElement, value_type_at_index(_value->get_type(), 0)), value(_value), index(_index) {}
ExtractElementRef ExtractElement::from(const TypedValueRef &value, const TypedValueRef &index) {
    return ref(unknown_anchor(), new ExtractElement(value, index));
}

InsertElement::InsertElement(const TypedValueRef &_value, const TypedValueRef &_element, const TypedValueRef &_index)
    : Instruction(VK_InsertElement, _value->get_type()), value(_value), element(_element), index(_index) {}
InsertElementRef InsertElement::from(const TypedValueRef &value, const TypedValueRef &element, const TypedValueRef &index) {
    return ref(unknown_anchor(), new InsertElement(value, element, index));
}

ShuffleVector::ShuffleVector(const TypedValueRef &_v1, const TypedValueRef &_v2, const std::vector<uint32_t> &_mask)
    : Instruction(VK_ShuffleVector, vector_type(value_type_at_index(_v1->get_type(), 0), _mask.size()).assert_ok()),
    v1(_v1), v2(_v2), mask(_mask) {}
ShuffleVectorRef ShuffleVector::from(const TypedValueRef &v1, const TypedValueRef &v2, const std::vector<uint32_t> &mask) {
    return ref(unknown_anchor(), new ShuffleVector(v1, v2, mask));
}

//------------------------------------------------------------------------------

Alloca::Alloca(const Type *T, const TypedValueRef &_count)
    : Instruction(VK_Alloca, local_pointer_type(T)), type(T), count(_count) {}
AllocaRef Alloca::from(const Type *T) {
    return ref(unknown_anchor(), new Alloca(T, TypedValueRef()));
}
AllocaRef Alloca::from(const Type *T, const TypedValueRef &count) {
    return ref(unknown_anchor(), new Alloca(T, count));
}
bool Alloca::is_array() const {
    return count;
}

Malloc::Malloc(const Type *T, const TypedValueRef &_count)
    : Instruction(VK_Malloc, native_pointer_type(T)), type(T), count(_count) {}
MallocRef Malloc::from(const Type *T) {
    return ref(unknown_anchor(), new Malloc(T, TypedValueRef()));
}
MallocRef Malloc::from(const Type *T, const TypedValueRef &count) {
    return ref(unknown_anchor(), new Malloc(T, count));
}
bool Malloc::is_array() const {
    return count;
}

Free::Free(const TypedValueRef &_value)
    : Instruction(VK_Free, empty_arguments_type()), value(_value) {}
FreeRef Free::from(const TypedValueRef &value) {
    return ref(unknown_anchor(), new Free(value));
}

Load::Load(const TypedValueRef &_value, bool _is_volatile)
    : Instruction(VK_Load, value_type_at_index(_value->get_type(),0)), value(_value), is_volatile(_is_volatile) {}
LoadRef Load::from(const TypedValueRef &value, bool is_volatile) {
    return ref(unknown_anchor(), new Load(value, is_volatile));
}

Store::Store(const TypedValueRef &_value, const TypedValueRef &_target, bool _is_volatile)
    : Instruction(VK_Store, empty_arguments_type()), value(_value), target(_target), is_volatile(_is_volatile) {}
StoreRef Store::from(const TypedValueRef &value, const TypedValueRef &target, bool is_volatile) {
    return ref(unknown_anchor(), new Store(value, target, is_volatile));
}

AtomicRMW::AtomicRMW(AtomicRMWOpKind _op, const TypedValueRef &_target, const TypedValueRef &_value)
    : Instruction(VK_AtomicRMW, value_type_at_index(_target->get_type(),0)), op(_op), target(_target), value(_value) {}
AtomicRMWRef AtomicRMW::from(AtomicRMWOpKind op, const TypedValueRef &target, const TypedValueRef &value) {
    return ref(unknown_anchor(), new AtomicRMW(op, target, value));
}

CmpXchg::CmpXchg(const TypedValueRef &_target, const TypedValueRef &_cmp, const TypedValueRef &_value)
    : Instruction(VK_CmpXchg,
        arguments_type({value_type_at_index(_target->get_type(),0), TYPE_Bool})),
        target(_target), cmp(_cmp), value(_value) {}
CmpXchgRef CmpXchg::from(const TypedValueRef &target, const TypedValueRef &cmp, const TypedValueRef &value) {
    return ref(unknown_anchor(), new CmpXchg(target, cmp, value));
}

Barrier::Barrier(BarrierKind _kind)
    : Instruction(VK_Barrier, empty_arguments_type()), kind(_kind) {}
BarrierRef Barrier::from(BarrierKind kind) {
    return ref(unknown_anchor(), new Barrier(kind));
}

//------------------------------------------------------------------------------

CastRef PtrToRef::from(const TypedValueRef &value) {
    return Cast::from(CastPtrToRef, value, ptr_to_ref(value->get_type()).assert_ok());
}

CastRef RefToPtr::from(const TypedValueRef &value) {
    return Cast::from(CastRefToPtr, value, ref_to_ptr(value->get_type()).assert_ok());
}

//------------------------------------------------------------------------------

Call::Call(const Type *type, const TypedValueRef &_callee, const TypedValues &_args)
    : Instruction(VK_Call, type), callee(_callee), args(_args),
        except(ExceptionRef()) {
}

CallRef Call::from(const Type *type, const TypedValueRef &callee, const TypedValues &args) {
    validate_instruction(callee);
    validate_instructions(args);
    return ref(unknown_anchor(), new Call(type, callee, args));
}

//------------------------------------------------------------------------------

Cast::Cast(CastKind _op, const TypedValueRef &_value, const Type *_type)
    : Instruction(VK_Cast, _type), op(_op), value(_value) {}

CastRef Cast::from(CastKind op, const TypedValueRef &value, const Type *type) {
    return ref(unknown_anchor(), new Cast(op, value,  type));
}

//------------------------------------------------------------------------------

LoopLabel::LoopLabel(const TypedValues &_init, const LoopLabelArgumentsRef &_args)
    : Instruction(VK_LoopLabel, TYPE_NoReturn), init(_init), args(_args) {
    assert(args);
    args->loop = ref(unknown_anchor(), this);
}

LoopLabelRef LoopLabel::from(const TypedValues &init, const LoopLabelArgumentsRef &args) {
    return ref(unknown_anchor(), new LoopLabel(init, args));
}

//------------------------------------------------------------------------------

Loop::Loop(const ValueRef &_init, const ValueRef &_value)
    : UntypedValue(VK_Loop), init(_init), value(_value) {
    args = ref(unknown_anchor(), LoopArguments::from(ref(unknown_anchor(), this)));
}

LoopRef Loop::from(const ValueRef &init, const ValueRef &value) {
    return ref(unknown_anchor(), new Loop(init, value));
}

//------------------------------------------------------------------------------

Label::Label(LabelKind _kind, Symbol _name)
    : Instruction(VK_Label, empty_arguments_type()), name(_name), label_kind(_kind) {}

LabelRef Label::from(LabelKind kind, Symbol name) {
    return ref(unknown_anchor(), new Label(kind, name));
}

void Label::change_type(const Type *type) {
    assert(type);
    _type = type;
}

const char *get_label_kind_name(LabelKind kind) {
    switch(kind) {
    #define T(NAME, BNAME) \
        case NAME: return BNAME;
    SCOPES_LABEL_KIND()
    #undef T
    default: return "???";
    }
}

//------------------------------------------------------------------------------

LabelTemplate::LabelTemplate(LabelKind _kind, Symbol _name, const ValueRef &_value)
    : UntypedValue(VK_LabelTemplate), name(_name), value(_value), label_kind(_kind) {}

LabelTemplateRef LabelTemplate::from(LabelKind kind, Symbol name, const ValueRef &value) {
    return ref(unknown_anchor(), new LabelTemplate(kind, name, value));
}

LabelTemplateRef LabelTemplate::try_from(const ValueRef &value) {
    return ref(unknown_anchor(), new LabelTemplate(LK_Try, KW_Try, value));
}
LabelTemplateRef LabelTemplate::except_from(const ValueRef &value) {
    return ref(unknown_anchor(), new LabelTemplate(LK_Except, KW_Except, value));
}

//------------------------------------------------------------------------------

bool Pure::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME:
SCOPES_PURE_VALUE_KIND()
#undef T
        return true;
    default: return false;
    }
}

Pure::Pure(ValueKind _kind, const Type *type)
    : TypedValue(_kind, type) {
}

bool Pure::key_equal(const Pure *other) const {
    if (kind() != other->kind())
        return false;
    switch(kind()) {
    #define T(KIND, NAME, CLASS) \
        case KIND: \
            return cast<CLASS>(this)->key_equal(cast<CLASS>(other));
    SCOPES_PURE_VALUE_KIND()
    #undef T
    default: assert(false); return false;
    }
}

std::size_t Pure::hash() const {
    switch(kind()) {
    #define T(KIND, NAME, CLASS) \
        case KIND: \
            return hash2(std::hash<int>{}(kind()), cast<CLASS>(this)->hash());
            //return cast<CLASS>(this)->hash();
    SCOPES_PURE_VALUE_KIND()
    #undef T
    default: assert(false); return 0;
    }
}

//------------------------------------------------------------------------------

bool Const::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME:
SCOPES_CONST_VALUE_KIND()
#undef T
        return true;
    default: return false;
    }
}

Const::Const(ValueKind _kind, const Type *type)
    : Pure(_kind, type) {
}

//------------------------------------------------------------------------------

static ConstSet<ConstInt> constints;

ConstInt::ConstInt(const Type *type, const std::vector<uint64_t> &_value)
    : Const(VK_ConstInt, type), words(_value) {
    assert(!_value.empty());
}

bool ConstInt::key_equal(const ConstInt *other) const {
    if (get_type() != other->get_type())
        return false;
    if (words.size() != other->words.size())
        return false;
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i] != other->words[i])
            return false;
    }
    return true;
}

std::size_t ConstInt::hash() const {
    auto h = std::hash<const Type *>{}(get_type());
    for (size_t i = 0; i < words.size(); ++i) {
        h = hash2(h, std::hash<uint64_t>{}(words[i]));
    }
    return h;
}

uint64_t ConstInt::value() const {
    return words[0];
}

uint64_t ConstInt::msw() const {
    return words.back();
}

ConstIntRef ConstInt::from(const Type *type, std::vector<uint64_t> values) {
    auto T = cast<IntegerType>(storage_type(type).assert_ok());
    // truncate value
    auto w = T->width;
    auto issigned = T->issigned;
    auto numwords = (w + 63) / 64;
    if (values.size() > numwords) {
        // truncate
        values.resize(numwords);
    } else {
        // pad
        uint64_t val = (!issigned || (int64_t(values.back()) >= 0ll))?0ull:-1ull;
        while (values.size() < numwords) {
            values.push_back(val);
        }
    }
    assert(values.size() == numwords);
    if (w % 64 != 0) {
        w = w % 64;
        // mask tail
        auto &&value = values[numwords - 1];
        if (issigned) {
            int64_t intval = value;
            int shift = 64 - w;
            intval = (intval << shift) >> shift;
            value = intval;
        } else {
            value = value & ~(-1ull << w);
        }
    }
    return constints.from(type, values);
}

ConstIntRef ConstInt::from(const Type *type, uint64_t value) {
    std::vector<uint64_t> values = { value };
    return from(type, values);
}

ConstIntRef ConstInt::symbol_from(Symbol value) {
    return from(TYPE_Symbol, value.value());
}

ConstIntRef ConstInt::builtin_from(Builtin value) {
    return from(TYPE_Builtin, value.value());
}

//------------------------------------------------------------------------------

static ConstSet<ConstReal> constreals;

ConstReal::ConstReal(const Type *type, double _value)
    : Const(VK_ConstReal, type), value(_value) {}

bool ConstReal::key_equal(const ConstReal *other) const {
    return get_type() == other->get_type()
        && value == other->value;
}

std::size_t ConstReal::hash() const {
    return hash2(std::hash<const Type *>{}(get_type()), std::hash<double>{}(value));
}

ConstRealRef ConstReal::from(const Type *type, double value) {
    return constreals.from(type, value);
}

//------------------------------------------------------------------------------

static ConstSet<ConstAggregate> constaggs;

ConstAggregate::ConstAggregate(const Type *type, const ConstantPtrs &_fields)
    : Const(VK_ConstAggregate, type), values(_fields) {
    uint64_t h = std::hash<const Type *>{}(get_type());
    for (int i = 0; i < values.size(); ++i) {
        h = hash2(h, values[i]->hash());
    }
    _hash = h;
}

bool ConstAggregate::key_equal(const ConstAggregate *other) const {
    if (get_type() != other->get_type())
        return false;
    for (int i = 0; i < values.size(); ++i) {
        auto a = values[i];
        auto b = other->values[i];
        if (a != b)
            return false;
    }
    return true;
}

std::size_t ConstAggregate::hash() const {
    return _hash;
}

ConstAggregateRef ConstAggregate::from(const Type *type, const ConstantPtrs &fields) {
    return constaggs.from(type, fields);
}

ConstAggregateRef ConstAggregate::none_from() {
    return from(TYPE_Nothing, {});
}

ConstAggregateRef ConstAggregate::ast_from(const ValueRef &node) {
    auto ptr = ConstPointer::from(TYPE__Value, node.unref()).unref();
    return from(TYPE_ValueRef, { ptr, ConstPointer::anchor_from(node.anchor()).unref() });
}

ConstRef get_field(const ConstAggregateRef &value, int i) {
    return ref(value.anchor(), value->values[i]);
}

//------------------------------------------------------------------------------

static ConstSet<ConstPointer> constptrs;

ConstPointer::ConstPointer(const Type *type, const void *_pointer)
    : Const(VK_ConstPointer, type), value(_pointer) {}

bool ConstPointer::key_equal(const ConstPointer *other) const {
    if (get_type() != other->get_type())
        return false;
    return value == other->value;
}

std::size_t ConstPointer::hash() const {
    return hash2(std::hash<const Type *>{}(get_type()), std::hash<const void *>{}(value));
}

ConstPointerRef ConstPointer::from(const Type *type, const void *pointer) {
    return constptrs.from(type, pointer);
}

ConstPointerRef ConstPointer::type_from(const Type *type) {
    return from(TYPE_Type, type);
}

ConstPointerRef ConstPointer::closure_from(const Closure *closure) {
    return from(TYPE_Closure, closure);
}

ConstPointerRef ConstPointer::string_from(const String *str) {
    return from(TYPE_String, str);
}

ConstPointerRef ConstPointer::list_from(const List *list) {
    return from(TYPE_List, list);
}

ConstPointerRef ConstPointer::scope_from(const Scope *scope) {
    return from(TYPE_Scope, scope);
}

ConstPointerRef ConstPointer::anchor_from(const Anchor *anchor) {
    return from(TYPE_Anchor, anchor);
}

//------------------------------------------------------------------------------

Repeat::Repeat(const LoopLabelRef &_loop, const TypedValues &values)
    : Terminator(VK_Repeat, values), loop(_loop) {
}

RepeatRef Repeat::from(const LoopLabelRef &loop, const TypedValues &values) {
    return ref(unknown_anchor(), new Repeat(loop, values));
}

//------------------------------------------------------------------------------

Return::Return(const TypedValues &values)
    : Terminator(VK_Return, values) {
}

ReturnRef Return::from(const TypedValues &values) {
    return ref(unknown_anchor(), new Return(values));
}

//------------------------------------------------------------------------------

Merge::Merge(const LabelRef &_label, const TypedValues &values)
    : Terminator(VK_Merge, values), label(_label) {
}

MergeRef Merge::from(const LabelRef &label, const TypedValues &values) {
    return ref(unknown_anchor(), new Merge(label, values));
}

//------------------------------------------------------------------------------

MergeTemplate::MergeTemplate(const LabelTemplateRef &_label, const ValueRef &_value)
    : UntypedValue(VK_MergeTemplate), label(_label), value(_value) {}

MergeTemplateRef MergeTemplate::from(const LabelTemplateRef &label, const ValueRef &value) {
    return ref(unknown_anchor(), new MergeTemplate(label, value));
}

//------------------------------------------------------------------------------

Raise::Raise(const TypedValues &values)
    : Terminator(VK_Raise, values) {
}

RaiseRef Raise::from(const TypedValues &values) {
    return ref(unknown_anchor(), new Raise(values));
}

//------------------------------------------------------------------------------

Unreachable::Unreachable()
    : Terminator(VK_Unreachable, {}) {}
UnreachableRef Unreachable::from() {
    return ref(unknown_anchor(), new Unreachable());
}

//------------------------------------------------------------------------------

Discard::Discard()
    : Terminator(VK_Discard, {}) {}
DiscardRef Discard::from() {
    return ref(unknown_anchor(), new Discard());
}

//------------------------------------------------------------------------------

Quote::Quote(const ValueRef &_value)
    : UntypedValue(VK_Quote), value(_value) {
}

QuoteRef Quote::from(const ValueRef &value) {
    return ref(unknown_anchor(), new Quote(value));
}

//------------------------------------------------------------------------------

Unquote::Unquote(const ValueRef &_value)
    : UntypedValue(VK_Unquote), value(_value) {
}

UnquoteRef Unquote::from(const ValueRef &value) {
    return ref(unknown_anchor(), new Unquote(value));
}

//------------------------------------------------------------------------------

CompileStage::CompileStage(const Anchor *_anchor, const List *_next, const Scope *_env)
    : UntypedValue(VK_CompileStage), anchor(_anchor), next(_next), env(_env) {
}

CompileStageRef CompileStage::from(const Anchor *anchor, const List *next, const Scope *env) {
    return ref(unknown_anchor(), new CompileStage(anchor, next, env));
}

//------------------------------------------------------------------------------

ValueKind Value::kind() const { return _kind; }

Value::Value(ValueKind kind)
    : _kind(kind) {
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

int Value::get_depth() const {
    if (isa<Instruction>(this)) {
        auto instr = cast<Instruction>(this);
        const Block *block = instr->block;
        assert(block);
        return block->depth;
    } else if (isa<LoopLabelArguments>(this)) {
        auto lla = cast<LoopLabelArguments>(this);
        assert(lla->loop);
        return lla->loop->body.depth;
    } else {
        return 0;
    }
}

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const Value *T) { \
        return T->kind() == NAME; \
    }
SCOPES_VALUE_KIND()
#undef T

const Anchor *get_best_anchor(const ValueRef &value) {
    const Anchor *def_anchor = unknown_anchor();
#define T(CLASS) \
    if (value.isa<CLASS>()) { \
        def_anchor = value.cast<CLASS>()->def_anchor(); \
    } else
    SCOPES_DEFINED_VALUES()
#undef T
    {};
    if (!def_anchor->is_boring()) {
        return def_anchor;
    }
    return value.anchor();
}

void set_best_anchor(const ValueRef &value, const Anchor *anchor) {
#define T(CLASS) \
    if (value.isa<CLASS>()) { \
        auto val = value.cast<CLASS>(); \
        val->set_def_anchor(anchor); \
    } else
    SCOPES_DEFINED_VALUES()
#undef T
    {};
}

//------------------------------------------------------------------------------

UntypedValue::UntypedValue(ValueKind _kind)
    : Value(_kind)
{}

bool UntypedValue::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME:
SCOPES_UNTYPED_VALUE_KIND()
#undef T
        return true;
    default: return false;
    }
}

//------------------------------------------------------------------------------

bool TypedValue::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME: return true;
SCOPES_TYPED_VALUE_KIND()
#undef T
    default: return false;
    }
}

TypedValue::TypedValue(ValueKind _kind, const Type *type)
    : Value(_kind), _type(type) {
    assert(_type);
}

void TypedValue::hack_change_value(const Type *T) {
    assert(T);
    _type = T;
}

const Type *TypedValue::get_type() const {
    if (!_type) {
        StyledStream ss;
        ss << get_value_class_name(kind()) << std::endl;
    }
    assert(_type);
    return _type;
}

//------------------------------------------------------------------------------

Instruction::Instruction(ValueKind _kind, const Type *type)
    : TypedValue(_kind, type), name(SYM_Unnamed), block(nullptr) {
}

bool Instruction::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME:
SCOPES_INSTRUCTION_VALUE_KIND()
#undef T
        return true;
    default: return false;
    }
}

void validate_instruction(const TypedValueRef &value) {
#if 0
    assert(value);
    if (value.isa<Instruction>()) {
        auto instr = value.cast<Instruction>();
        assert(instr->block);
    }
#endif
}

void validate_instructions(const TypedValues &values) {
#if 0
    for (auto value : values) {
        validate_instruction(value);
    }
#endif
}

//------------------------------------------------------------------------------

bool Terminator::classof(const Value *T) {
    switch(T->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME:
SCOPES_TERMINATOR_VALUE_KIND()
#undef T
        return true;
    default: return false;
    }
}

Terminator::Terminator(ValueKind _kind, const TypedValues &_values)
    : Instruction(_kind, TYPE_NoReturn), values(_values)
{
    validate_instructions(_values);
}

//------------------------------------------------------------------------------

namespace ClosureSet {

struct Hash {
    std::size_t operator()(const Closure *k) const {
        return k->hash();
    }
};

struct Equal {
    std::size_t operator()(const Closure *self, const Closure *other) const {
        return self->key_equal(other);
    }
};

} // namespace ClosureSet

static std::unordered_set<Closure *, ClosureSet::Hash, ClosureSet::Equal> closures;

Closure::Closure(const TemplateRef &_func, const FunctionRef &_frame) :
    func(_func), frame(_frame) {}

bool Closure::key_equal(const Closure *other) const {
    return (func == other->func) && (frame == other->frame);
}

std::size_t Closure::hash() const {
    return hash2(
        std::hash<Template *>{}(func.unref()),
        std::hash<Function *>{}(frame.unref()));
}

Closure *Closure::from(const TemplateRef &func, const FunctionRef &frame) {
    Closure cl(func, frame);
    auto it = closures.find(&cl);
    if (it != closures.end()) {
        return *it;
    }
    Closure *result = new Closure(func, frame);
    closures.insert(result);
    return result;
}

StyledStream &Closure::stream(StyledStream &ost) const {
    ost << Style_Comment << "<" << Style_None;
    if (frame)
        ost << Style_Symbol << frame->name.name()->data << "λ" << (void *)frame.unref() << Style_None;
    ost << Style_Comment << "::" << Style_None
        << Style_Symbol << func->name.name()->data << "λ" << (void *)func.unref() << Style_None
        << Style_Comment << ">" << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ss, const Closure *closure) {
    closure->stream(ss);
    return ss;
}

StyledStream& operator<<(StyledStream& ost, Value *node) {
    ost << const_cast<const Value *>(node);
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const Value *node) {
    ost << Style_Keyword << get_value_class_name(node->kind()) << Style_None;
    ost << "$";
    stream_address(ost, node);
    if (isa<TypedValue>(node)) {
        ost << Style_Operator << ":" << Style_None << cast<TypedValue>(node)->get_type();
    }
    return ost;
}

StyledStream& operator<<(StyledStream& ost, TypedValue *node) {
    ost << (Value *)node;
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const TypedValue *node) {
    ost << (const Value *)node;
    return ost;
}

StyledStream& operator<<(StyledStream& ost, const ValueIndex &arg) {
    ost << arg.value;
    if (get_argument_count(arg.value->get_type()) != 1) {
        ost << Style_Operator << "@" << Style_None << arg.index;
        ost << Style_Operator << ":" << Style_None << arg.get_type();
    }
    return ost;
}

StyledStream& operator<<(StyledStream& ost, ValueIndex &arg) {
    ost << const_cast<const ValueIndex &>(arg);
    return ost;
}

} // namespace scopes

/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "symbol.hpp"
#include "hash.hpp"
#include "styled_stream.hpp"
#include "symbol_enum.inc"

#include <memory.h>
#include <string.h>
#include <assert.h>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

namespace scopes {

static absl::flat_hash_map<Symbol, const String *, Symbol::Hash> map_symbol_name;
static absl::flat_hash_map<const String *, Symbol> map_name_symbol;

static uint64_t num_symbols = 0;

//------------------------------------------------------------------------------
// SYMBOL TYPE
//------------------------------------------------------------------------------

std::size_t Symbol::Hash::operator()(const scopes::Symbol & s) const {
    return s.hash();
}

//------------------------------------------------------------------------------

size_t Symbol::symbol_count() {
    return num_symbols;
}

void Symbol::verify_unmapped(Symbol id, const String *name) {
    auto it = map_name_symbol.find({ name });
    if (it != map_name_symbol.end()) {
        StyledStream ss(SCOPES_CERR);
        ss << "known symbols "
            << get_known_symbol_name(id.known_value()) << " and "
            << get_known_symbol_name(it->second.known_value())
            << " mapped to same string ("
            << name
            << ")" << std::endl;
    }
}

void Symbol::map_symbol(Symbol id, const String *name) {
    map_name_symbol[name] = id;
    map_symbol_name[id] = name;
}

void Symbol::map_known_symbol(Symbol id, const String *name) {
    verify_unmapped(id, name);
    map_symbol(id, name);
}

Symbol Symbol::get_symbol(const String *name) {
    auto it = map_name_symbol.find(name);
    if (it != map_name_symbol.end()) {
        auto oldname = get_symbol_name(it->second);
        if (oldname != name) {
            StyledStream ss(SCOPES_CERR);
            ss << "internal error: symbol hash collision between "
               << name << " and " << oldname << std::endl;
        }
        return it->second;
    }
    num_symbols++;
    Symbol id = Symbol::wrap(name->hash());
    map_symbol(id, name);
    return id;
}

const String *Symbol::get_symbol_name(Symbol id) {
    auto it = map_symbol_name.find(id);
    if (it == map_symbol_name.end())
        it = map_symbol_name.find(SYM_Corrupted);
    return it->second;
}

Symbol::Symbol(uint64_t tid) :
    _value(tid) {
}

Symbol Symbol::wrap(uint64_t value) {
    return { value };
}

Symbol::Symbol() :
    _value(SYM_Unnamed) {}

Symbol::Symbol(Symbol::EnumT id) :
    _value(id) {
}

Symbol::Symbol(const String *str) :
    _value(get_symbol(str)._value) {
}

bool Symbol::is_known() const {
    switch(_value) {
#define T(sym, name) case sym: return true;
    SCOPES_SYMBOLS()
#undef T
#define T(NAME) case SYM_SPIRV_StorageClass ## NAME: return true;
    B_SPIRV_STORAGE_CLASS()
#undef T
#define T(NAME) case SYM_SPIRV_BuiltIn ## NAME: return true;
    B_SPIRV_BUILTINS()
#undef T
#define T(NAME) case SYM_SPIRV_ExecutionMode ## NAME: return true;
    B_SPIRV_EXECUTION_MODE()
#undef T
#define T(NAME) case SYM_SPIRV_Dim ## NAME: return true;
    B_SPIRV_DIM()
#undef T
#define T(NAME) case SYM_SPIRV_ImageFormat ## NAME: return true;
    B_SPIRV_IMAGE_FORMAT()
#undef T
#define T(NAME) case SYM_SPIRV_ImageOperand ## NAME: return true;
    B_SPIRV_IMAGE_OPERAND()
#undef T
    default: return false;
    }
}

Symbol::EnumT Symbol::known_value() const {
    assert(is_known());
    return (EnumT)_value;
}

// for std::map support
bool Symbol::operator < (Symbol b) const {
    return _value < b._value;
}

bool Symbol::operator ==(Symbol b) const {
    return _value == b._value;
}

bool Symbol::operator !=(Symbol b) const {
    return _value != b._value;
}

bool Symbol::operator ==(EnumT b) const {
    return _value == b;
}

bool Symbol::operator !=(EnumT b) const {
    return _value != b;
}

std::size_t Symbol::hash() const {
    return _value;
}

uint64_t Symbol::value() const {
    return _value;
}

const String *Symbol::name() const {
    return get_symbol_name(*this);
}

void Symbol::_init_symbols() {
#define T(sym, name) map_known_symbol(sym, String::from(name));
    SCOPES_SYMBOLS()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_StorageClass ## NAME, String::from(#NAME));
    B_SPIRV_STORAGE_CLASS()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_BuiltIn ## NAME, String::from("spirv." #NAME));
    B_SPIRV_BUILTINS()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_ExecutionMode ## NAME, String::from(#NAME));
    B_SPIRV_EXECUTION_MODE()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_Dim ## NAME, String::from(#NAME));
    B_SPIRV_DIM()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_ImageFormat ## NAME, String::from(#NAME));
    B_SPIRV_IMAGE_FORMAT()
#undef T
#define T(NAME) \
    map_known_symbol(SYM_SPIRV_ImageOperand ## NAME, String::from(#NAME));
    B_SPIRV_IMAGE_OPERAND()
#undef T

#if 0
    absl::flat_hash_set<Symbol, Symbol::Hash> defined;
    StyledStream ss;
#define T(NAME) \
    ss << "T(" << #NAME << ", " << Symbol(NAME).name() << ") \\" << std::endl; \
    defined.insert(Symbol(NAME));
    SCOPES_BUILTIN_SYMBOLS()
#undef T
    ss << std::endl;
#define T(NAME, STR) \
    if (!defined.count(Symbol(NAME))) { \
        ss << "T(" << #NAME << ", " << Symbol(NAME).name() << ") \\" << std::endl; \
    }
    SCOPES_SYMBOLS()
#undef T
    ss << std::endl;
#endif
}

StyledStream& Symbol::stream(StyledStream& ost) const {
    auto s = name();
    assert(s);
    ost << Style_Symbol << "'";
    s->stream(ost, SYMBOL_ESCAPE_CHARS);
    ost << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Symbol &sym) {
    return sym.stream(ost);
}

StyledStream& operator<<(StyledStream& ost, const Symbol &sym) {
    return sym.stream(ost);
}

bool ends_with_parenthesis(Symbol sym) {
    if (sym == SYM_Parenthesis)
        return true;
    const String *str = sym.name();
    if (str->count < 3)
        return false;
    const char *dot = str->data + str->count - 3;
    return !strcmp(dot, "...");
}

} // namespace scopes

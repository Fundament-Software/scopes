/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "symbol.hpp"

#include "cityhash/city.h"

#include <memory.h>
#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// SYMBOL TYPE
//------------------------------------------------------------------------------

std::size_t Symbol::Hash::operator()(const scopes::Symbol & s) const {
    return s.hash();
}

//------------------------------------------------------------------------------

std::size_t Symbol::StringKey::Hash::operator()(const StringKey &s) const {
    return CityHash64(s.str->data, s.str->count);
}

bool Symbol::StringKey::operator ==(const StringKey &rhs) const {
    if (str->count == rhs.str->count) {
        return !memcmp(str->data, rhs.str->data, str->count);
    }
    return false;
}

//------------------------------------------------------------------------------

void Symbol::verify_unmapped(Symbol id, const String *name) {
    auto it = map_name_symbol.find({ name });
    if (it != map_name_symbol.end()) {
        printf("known symbols %s and %s mapped to same string.\n",
            get_known_symbol_name(id.known_value()),
            get_known_symbol_name(it->second.known_value()));
    }
}

void Symbol::map_symbol(Symbol id, const String *name) {
    map_name_symbol[{ name }] = id;
    map_symbol_name[id] = name;
}

void Symbol::map_known_symbol(Symbol id, const String *name) {
    verify_unmapped(id, name);
    map_symbol(id, name);
}

Symbol Symbol::get_symbol(const String *name) {
    auto it = map_name_symbol.find({ name });
    if (it != map_name_symbol.end()) {
        return it->second;
    }
    Symbol id = Symbol::wrap(++next_symbol_id);
    // make copy
    map_symbol(id, String::from(name->data, name->count));
    return id;
}

const String *Symbol::get_symbol_name(Symbol id) {
    auto it = map_symbol_name.find(id);
    assert (it != map_symbol_name.end());
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
    return _value < end_value;
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
#define T0 T
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    map_known_symbol(FN_ ## UNAME ## PFIX, String::from(#LNAME #OP));
    B_MAP_SYMBOLS()
#undef T
#undef T0
#undef T1
#undef T2
#undef T2T
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
}

StyledStream& Symbol::stream(StyledStream& ost) const {
    auto s = name();
    assert(s);
    ost << Style_Symbol << "'";
    s->stream(ost, SYMBOL_ESCAPE_CHARS);
    ost << Style_None;
    return ost;
}

std::unordered_map<Symbol, const String *, Symbol::Hash> Symbol::map_symbol_name;
std::unordered_map<Symbol::StringKey, Symbol, Symbol::StringKey::Hash> Symbol::map_name_symbol;
uint64_t Symbol::next_symbol_id = SYM_Count;

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Symbol sym) {
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

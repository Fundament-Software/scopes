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

#ifndef SCOPES_SYMBOL_HPP
#define SCOPES_SYMBOL_HPP

#include "symbol_enum.hpp"
#include "string.hpp"

#include <cstddef>
#include <unordered_map>

namespace scopes {

struct StyledStream;

//------------------------------------------------------------------------------
// SYMBOL
//------------------------------------------------------------------------------

const char SYMBOL_ESCAPE_CHARS[] = " []{}()\"";

//------------------------------------------------------------------------------
// SYMBOL TYPE
//------------------------------------------------------------------------------

struct Symbol {
    typedef KnownSymbol EnumT;
    enum { end_value = SYM_Count };

    struct Hash {
        std::size_t operator()(const scopes::Symbol & s) const;
    };

protected:
    struct StringKey {
        struct Hash {
            std::size_t operator()(const StringKey &s) const;
        };

        const String *str;

        bool operator ==(const StringKey &rhs) const;
    };

    static std::unordered_map<Symbol, const String *, Hash> map_symbol_name;
    static std::unordered_map<StringKey, Symbol, StringKey::Hash> map_name_symbol;
    static uint64_t next_symbol_id;

    static void verify_unmapped(Symbol id, const String *name);

    static void map_symbol(Symbol id, const String *name);

    static void map_known_symbol(Symbol id, const String *name);

    static Symbol get_symbol(const String *name);

    static const String *get_symbol_name(Symbol id);

    uint64_t _value;

    Symbol(uint64_t tid);

public:
    static Symbol wrap(uint64_t value);

    Symbol();

    Symbol(EnumT id);

    template<unsigned N>
    Symbol(const char (&str)[N]) :
        _value(get_symbol(String::from(str))._value) {
    }

    Symbol(const String *str);

    bool is_known() const;
    EnumT known_value() const;

    // for std::map support
    bool operator < (Symbol b) const;
    bool operator ==(Symbol b) const;
    bool operator !=(Symbol b) const;
    bool operator ==(EnumT b) const;
    bool operator !=(EnumT b) const;

    std::size_t hash() const;
    uint64_t value() const;

    const String *name() const;

    static void _init_symbols();

    StyledStream& stream(StyledStream& ost) const;

};

StyledStream& operator<<(StyledStream& ost, Symbol sym);

bool ends_with_parenthesis(Symbol sym);

} // namespace scopes

#endif // SCOPES_SYMBOL_HPP

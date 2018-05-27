/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SCOPE_HPP
#define SCOPES_SCOPE_HPP

#include "any.hpp"

#include <vector>
#include <unordered_map>

namespace scopes {

//------------------------------------------------------------------------------
// SCOPE
//------------------------------------------------------------------------------

struct AnyDoc {
    Any value;
    const String *doc;
};

struct Scope {
public:
    typedef std::unordered_map<Symbol, AnyDoc, Symbol::Hash> Map;
protected:
    Scope(Scope *_parent = nullptr, Map *_map = nullptr);

public:
    Scope *parent;
    Map *map;
    bool borrowed;
    const String *doc;
    const String *next_doc;

    void set_doc(const String *str);

    size_t count() const;

    size_t totalcount() const;

    size_t levelcount() const;

    void ensure_not_borrowed();

    void bind_with_doc(Symbol name, const AnyDoc &entry);

    void bind(Symbol name, const Any &value);

    void bind(KnownSymbol name, const Any &value);

    void del(Symbol name);

    std::vector<Symbol> find_closest_match(Symbol name) const;

    std::vector<Symbol> find_elongations(Symbol name) const;

    bool lookup(Symbol name, AnyDoc &dest, size_t depth = -1) const;

    bool lookup(Symbol name, Any &dest, size_t depth = -1) const;

    bool lookup_local(Symbol name, AnyDoc &dest) const;

    bool lookup_local(Symbol name, Any &dest) const;

    StyledStream &stream(StyledStream &ss);

    static Scope *from(Scope *_parent = nullptr, Scope *_borrow = nullptr);
};

extern Scope *globals;

StyledStream& operator<<(StyledStream& ost, Scope *scope);

} // namespace scopes

#endif // SCOPES_SCOPE_HPP
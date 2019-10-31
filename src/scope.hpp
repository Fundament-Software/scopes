/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SCOPE_HPP
#define SCOPES_SCOPE_HPP

#include "symbol.hpp"
#include "valueref.inc"
#include "ordered_map.hpp"

#include <vector>

namespace scopes {

//------------------------------------------------------------------------------
// SCOPE
//------------------------------------------------------------------------------

struct Value;
struct String;

struct ScopeMapEntry {
    ValueRef value;
    const String *doc;
};

struct Scope {
public:
    typedef OrderedMap<ConstRef, ScopeMapEntry, ConstRef::Hash> Map;

protected:
    Scope(const ConstRef &name, const ValueRef &value, const String *doc, const Scope *next);
    Scope(const String *doc, const Scope *parent);

    mutable const Map *map;
    mutable size_t index;
public:
    ConstRef name;
    ValueRef value;
    const String *doc;
    const Scope *next;
    const Scope *start;

    const Scope *parent() const;
    const String *header_doc() const;
    bool is_header() const;

    size_t count() const;

    size_t totalcount() const;

    size_t levelcount() const;

    const Map &table() const;

    std::vector<Symbol> find_closest_match(Symbol name) const;
    std::vector<Symbol> find_elongations(Symbol name) const;

    bool lookup(const ConstRef &name, ValueRef &dest, const String *&doc, size_t depth = -1) const;
    bool lookup(const ConstRef &name, ValueRef &dest, size_t depth = -1) const;

    bool lookup_local(const ConstRef &name, ValueRef &dest, const String *&doc) const;

    bool lookup_local(const ConstRef &name, ValueRef &dest) const;

    StyledStream &stream(StyledStream &ss) const;

    static const Scope *reparent_from(const Scope *content, const Scope *parent);
    static const Scope *bind_from(const ConstRef &name, const ValueRef &value, const String *doc, const Scope *next);
    static const Scope *unbind_from(const ConstRef &name, const Scope *next);
    static const Scope *from(const String *doc, const Scope *parent);
};

} // namespace scopes

#endif // SCOPES_SCOPE_HPP
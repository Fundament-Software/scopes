/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "scope.hpp"

#include <algorithm>
#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------
// SCOPE
//------------------------------------------------------------------------------

Scope::Scope(Scope *_parent, Map *_map) :
    parent(_parent),
    map(_map?_map:(new Map())),
    borrowed(_map?true:false),
    doc(nullptr),
    next_doc(nullptr) {
    if (_parent)
        doc = _parent->doc;
}

void Scope::set_doc(const String *str) {
    if (!doc) {
        doc = str;
        next_doc = nullptr;
    } else {
        next_doc = str;
    }
}

size_t Scope::count() const {
#if 0
    return map->size();
#else
    size_t count = 0;
    auto &&_map = *map;
    for (auto &&k : _map) {
        if (!is_typed(k.second.value))
            continue;
        count++;
    }
    return count;
#endif
}

size_t Scope::totalcount() const {
    const Scope *self = this;
    size_t count = 0;
    while (self) {
        count += self->count();
        self = self->parent;
    }
    return count;
}

size_t Scope::levelcount() const {
    const Scope *self = this;
    size_t count = 0;
    while (self) {
        count += 1;
        self = self->parent;
    }
    return count;
}

void Scope::ensure_not_borrowed() {
    if (!borrowed) return;
    parent = Scope::from(parent, this);
    map = new Map();
    borrowed = false;
}

void Scope::bind_with_doc(Symbol name, const AnyDoc &entry) {
    ensure_not_borrowed();
    auto ret = map->insert({name, entry});
    if (!ret.second) {
        ret.first->second = entry;
    }
}

void Scope::bind(Symbol name, const Any &value) {
    AnyDoc entry = { value, next_doc };
    bind_with_doc(name, entry);
    next_doc = nullptr;
}

void Scope::bind(KnownSymbol name, const Any &value) {
    AnyDoc entry = { value, nullptr };
    bind_with_doc(Symbol(name), entry);
}

void Scope::del(Symbol name) {
    ensure_not_borrowed();
    auto it = map->find(name);
    if (it != map->end()) {
        // if in local map, we can delete it directly
        map->erase(it);
    } else {
        // otherwise check if it's contained at all
        Any dest = none;
        if (lookup(name, dest)) {
            AnyDoc entry = { untyped(), nullptr };
            // if yes, bind to unknown unknown to mark it as deleted
            bind_with_doc(name, entry);
        }
    }
}

std::vector<Symbol> Scope::find_closest_match(Symbol name) const {
    const String *s = name.name();
    std::unordered_set<Symbol, Symbol::Hash> done;
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    const Scope *self = this;
    do {
        auto &&map = *self->map;
        for (auto &&k : map) {
            Symbol sym = k.first;
            if (done.count(sym))
                continue;
            if (is_typed(k.second.value)) {
                size_t dist = distance(s, sym.name());
                if (dist == best_dist) {
                    best_syms.push_back(sym);
                } else if (dist < best_dist) {
                    best_dist = dist;
                    best_syms = { sym };
                }
            }
            done.insert(sym);
        }
        self = self->parent;
    } while (self);
    std::sort(best_syms.begin(), best_syms.end());
    return best_syms;
}

std::vector<Symbol> Scope::find_elongations(Symbol name) const {
    const String *s = name.name();

    std::unordered_set<Symbol, Symbol::Hash> done;
    std::vector<Symbol> found;
    const Scope *self = this;
    do {
        auto &&map = *self->map;
        for (auto &&k : map) {
            Symbol sym = k.first;
            if (done.count(sym))
                continue;
            if (is_typed(k.second.value)) {
                if (sym.name()->count >= s->count &&
                        *sym.name()->substr(0, s->count) == *s)
                    found.push_back(sym);
            }
            done.insert(sym);
        }
        self = self->parent;
    } while (self);
    std::sort(found.begin(), found.end(), [](Symbol a, Symbol b){
                return a.name()->count < b.name()->count; });
    return found;
}

bool Scope::lookup(Symbol name, AnyDoc &dest, size_t depth) const {
    const Scope *self = this;
    do {
        auto it = self->map->find(name);
        if (it != self->map->end()) {
            if (is_typed(it->second.value)) {
                dest = it->second;
                return true;
            } else {
                return false;
            }
        }
        if (!depth)
            break;
        depth = depth - 1;
        self = self->parent;
    } while (self);
    return false;
}

bool Scope::lookup(Symbol name, Any &dest, size_t depth) const {
    AnyDoc entry = { none, nullptr };
    if (lookup(name, entry, depth)) {
        dest = entry.value;
        return true;
    }
    return false;
}

bool Scope::lookup_local(Symbol name, AnyDoc &dest) const {
    return lookup(name, dest, 0);
}

bool Scope::lookup_local(Symbol name, Any &dest) const {
    return lookup(name, dest, 0);
}

StyledStream &Scope::stream(StyledStream &ss) {
    size_t totalcount = this->totalcount();
    size_t count = this->count();
    size_t levelcount = this->levelcount();
    ss << Style_Keyword << "Scope" << Style_Comment << "<" << Style_None
        << format("L:%i T:%i in %i levels", count, totalcount, levelcount)->data
        << Style_Comment << ">" << Style_None;
    return ss;
}

Scope *Scope::from(Scope *_parent, Scope *_borrow) {
    return new Scope(_parent, _borrow?(_borrow->map):nullptr);
}

//------------------------------------------------------------------------------

Scope *globals = Scope::from();

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Scope *scope) {
    scope->stream(ost);
    return ost;
}


} // namespace scopes

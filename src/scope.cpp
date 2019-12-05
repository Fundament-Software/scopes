/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "scope.hpp"
#include "value.hpp"
#include "error.hpp"

#include <algorithm>
#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------
// SCOPE
//------------------------------------------------------------------------------

Scope::Scope(const String *_doc, const Scope *_parent) :
    map(nullptr),
    index(0),
    name(ConstRef()),
    value(ValueRef()),
    doc(_doc),
    next(_parent),
    start(this) {
    if (!doc && next) {
        doc = next->header_doc();
    }
}

Scope::Scope(const ConstRef &_name, const ValueRef &_value, const String *_doc, const Scope *_next) :
    map(nullptr),
    name(_name),
    value(_value),
    doc(_doc),
    next(_next) {
    assert(_name);
    assert(_next && _next->start);
    start = _next->start;
    index = _next->index + 1;
}

const Scope *Scope::parent() const {
    return start->next;
}

const String *Scope::header_doc() const {
    return start->doc;
}

bool Scope::is_header() const {
    return start == this;
}

size_t Scope::count() const {
    return index;
}

size_t Scope::totalcount() const {
    size_t count = 0;
    const Scope *self = this;
    while (self) {
        count += self->index;
        self = self->parent();
    }
    return count;
}

size_t Scope::levelcount() const {
    size_t count = 0;
    const Scope *self = this;
    while (self) {
        count += 1;
        self = self->parent();
    }
    return count;
}

const Scope::Map &Scope::table() const {
    if (!map) {
        auto map = new Map();
        const Scope *self = this;
        while (true) {
            if (self->map) {
                // copy all elements from map
                size_t i = self->map->keys.size();
                while (i-- > 0) {
                    auto &&key = self->map->keys[i];
                    auto &&entry = self->map->values[i];
                    map->insert(key, entry);
                }
                break;
            } else if (!self->is_header()) {
                map->insert(self->name, { self->value, self->doc });
                self = self->next;
            } else {
                break;
            }
        }
        map->flip();
        this->map = map;
    }
    return *map;
}

const Scope *Scope::reparent_from(const Scope *content, const Scope *parent) {
    auto &&map = content->table();
    auto self = from(content->header_doc(), parent);
    #if 0
    auto count = map.keys.size();
    for (size_t i = 0; i < count; ++i) {
        auto &&key = map.keys[i];
        auto &&entry = map.values[i];
        self = bind_from(key, entry.value, entry.doc, self);
    }
    #endif
    // can reuse the map because the content is the same
    self->map = &map;
    self->index = map.keys.size();
    return self;
}

const Scope *Scope::bind_from(const ConstRef &name, const ValueRef &value, const String *doc, const Scope *next) {
    assert(value);
    return new Scope(name, value, doc, next);
}

const Scope *Scope::unbind_from(const ConstRef &name, const Scope *next) {
    // check if value is contained
    ValueRef dest;
    if (next->lookup(name, dest)) {
        return new Scope(name, ValueRef(), nullptr, next);
    }
    return next;
}

const Scope *Scope::from(const String *doc, const Scope *parent) {
    return new Scope(doc, parent);
}

std::vector<Symbol> Scope::find_closest_match(Symbol name) const {
    const String *s = name.name();
    std::unordered_set<Symbol, Symbol::Hash> done;
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    const Scope *self = this;
    do {
        if (self->map) {
            size_t count = self->map->keys.size();
            for (size_t i = 0; i < count; ++i) {
                auto &&key = self->map->keys[i];
                if (key->get_type() == TYPE_Symbol) {
                    Symbol sym = Symbol::wrap(key.cast<ConstInt>()->value());
                    if (!done.count(sym)) {
                        auto &&value = self->map->values[i];
                        if (value.value) {
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
                }
            }
            self = self->start;
        } else if (!self->is_header()) {
            auto &&key = self->name;
            if (key->get_type() == TYPE_Symbol) {
                Symbol sym = Symbol::wrap(key.cast<ConstInt>()->value());
                if (!done.count(sym)) {
                    if (self->value) {
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
            }
        }
        self = self->next;
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
        if (self->map) {
            size_t count = self->map->keys.size();
            for (size_t i = 0; i < count; ++i) {
                auto &&key = self->map->keys[i];
                if (key->get_type() == TYPE_Symbol) {
                    Symbol sym = Symbol::wrap(key.cast<ConstInt>()->value());
                    if (!done.count(sym)) {
                        auto &&value = self->map->values[i];
                        if (value.value) {
                            if (sym.name()->count >= s->count &&
                                    (sym.name()->substr(0, s->count) == s))
                                found.push_back(sym);
                        }
                        done.insert(sym);
                    }
                }
            }
            self = self->start;
        } else if (!self->is_header()) {
            auto &&key = self->name;
            if (key->get_type() == TYPE_Symbol) {
                Symbol sym = Symbol::wrap(key.cast<ConstInt>()->value());
                if (!done.count(sym)) {
                    if (self->value) {
                        if (sym.name()->count >= s->count &&
                                (sym.name()->substr(0, s->count) == s))
                            found.push_back(sym);
                    }
                    done.insert(sym);
                }
            }
        }
        self = self->next;
    } while (self);
    std::sort(found.begin(), found.end(), [](Symbol a, Symbol b){
                return a.name()->count < b.name()->count; });
    return found;
}

bool Scope::lookup(const ConstRef &name, ValueRef &dest, const String *&doc, size_t depth) const {
    const Scope *self = this;
    const Scope *last_end = this;
    int iterations = 0;
    do {
        // compact when we searched too long and retry
        if (iterations == 16) {
            self = last_end;
            self->table();
            iterations = 0;
        }
        if (self->map) {
            int i = self->map->find_index(name);
            if (i != -1) {
                auto &&value = self->map->values[i];
                if (value.value) {
                    dest = value.value;
                    doc = value.doc;
                    return true;
                } else {
                    return false;
                }
            }
            self = self->start;
            last_end = self->next;
            //if (self->parent()) self->parent()->table();
        }
        if (self->is_header()) {
            if (!depth)
                break;
            depth = depth - 1;
        } else {
            iterations++;
            if (self->name == name) {
                if (self->value) {
                    dest = self->value;
                    doc = self->doc;
                    return true;
                } else {
                    return false;
                }
            }
        }
        self = self->next;
    } while (self);
    return false;
}

bool Scope::lookup(const ConstRef &name, ValueRef &dest, size_t depth) const {
    const String *doc;
    return lookup(name, dest, doc, depth);
}

bool Scope::lookup_local(const ConstRef &name, ValueRef &dest, const String *&doc) const {
    return lookup(name, dest, doc, 0);
}

bool Scope::lookup_local(const ConstRef & name, ValueRef &dest) const {
    return lookup(name, dest, 0);
}

StyledStream &Scope::stream(StyledStream &ss) const {
    size_t totalcount = this->totalcount();
    size_t count = this->count();
    size_t levelcount = this->levelcount();
    ss << Style_Keyword << "Scope" << Style_Comment << "<" << Style_None
        << format("L:%i T:%i in %i levels", count, totalcount, levelcount)->data
        << Style_Comment << ">" << Style_None;
    return ss;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, const Scope *scope) {
    scope->stream(ost);
    return ost;
}


} // namespace scopes

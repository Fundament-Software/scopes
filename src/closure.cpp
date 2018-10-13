/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "closure.hpp"
#include "hash.hpp"
#include "styled_stream.hpp"
#include "value.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------

Closure::Closure(Template *_func, Function *_frame) :
    func(_func), frame(_frame) {}

std::size_t Closure::Hash::operator()(const Closure &k) const {
    return hash2(
        std::hash<Template *>{}(k.func),
        std::hash<Function *>{}(k.frame));
}

bool Closure::operator ==(const Closure &k) const {
    return (func == k.func)
        && (frame == k.frame);
}

const Closure *Closure::from(Template *func, Function *frame) {
    Closure cl(func, frame);
    auto it = map.find(cl);
    if (it != map.end()) {
        return it->second;
    }
    const Closure *result = new Closure(func, frame);
    map.insert({cl, result});
    return result;
}

StyledStream &Closure::stream(StyledStream &ost) const {
    ost << Style_Comment << "<" << Style_None
        << Style_Symbol << frame->name.name()->data << "λ" << (void *)frame << Style_None
        << Style_Comment << "::" << Style_None
        << Style_Symbol << func->name.name()->data << "λ" << (void *)func << Style_None
        << Style_Comment << ">" << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

std::unordered_map<Closure, const Closure *, Closure::Hash> Closure::map;

StyledStream& operator<<(StyledStream& ss, const Closure *closure) {
    closure->stream(ss);
    return ss;
}

} // namespace scopes

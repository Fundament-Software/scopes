/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "closure.hpp"
#include "label.hpp"

#include "cityhash/city.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------

Closure::Closure(Label *_label, Frame *_frame) :
    label(_label), frame(_frame) {}

std::size_t Closure::Hash::operator()(const Closure &k) const {
    return HashLen16(
        std::hash<Label *>{}(k.label),
        std::hash<Frame *>{}(k.frame));
}

bool Closure::operator ==(const Closure &k) const {
    return (label == k.label)
        && (frame == k.frame);
}

const Closure *Closure::from(Label *label, Frame *frame) {
    assert (label->is_template());
    Closure cl(label, frame);
    auto it = map.find(cl);
    if (it != map.end()) {
        return it->second;
    }
    const Closure *result = new Closure(label, frame);
    map.insert({cl, result});
    return result;
}

StyledStream &Closure::stream(StyledStream &ost) const {
    ost << Style_Comment << "<" << Style_None
        << frame
        << Style_Comment << "::" << Style_None;
    label->stream_short(ost);
    ost << Style_Comment << ">" << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

std::unordered_map<Closure, const Closure *, Closure::Hash> Closure::map;

StyledStream& operator<<(StyledStream& ss, const Closure *closure) {
    closure->stream(ss);
    return ss;
}

} // namespace scopes

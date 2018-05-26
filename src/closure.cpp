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

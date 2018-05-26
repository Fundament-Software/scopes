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

#ifndef SCOPES_CLOSURE_HPP
#define SCOPES_CLOSURE_HPP

#include <cstddef>

#include <unordered_map>

namespace scopes {

struct Label;
struct Frame;
struct StyledStream;

struct Closure {
protected:

    Closure(Label *_label, Frame *_frame);

public:

    struct Hash {
        std::size_t operator()(const Closure &k) const;
    };

    bool operator ==(const Closure &k) const;

    static std::unordered_map<Closure, const Closure *, Closure::Hash> map;

    Label *label;
    Frame *frame;

    static const Closure *from(Label *label, Frame *frame);

    StyledStream &stream(StyledStream &ost) const;
};

StyledStream& operator<<(StyledStream& ss, const Closure *closure);

} // namespace scopes

#endif // SCOPES_CLOSURE_HPP
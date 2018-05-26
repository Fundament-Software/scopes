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

#ifndef SCOPES_BODY_HPP
#define SCOPES_BODY_HPP

#include "argument.hpp"
#include "any.hpp"

namespace scopes {

struct Anchor;
struct Label;

enum LabelBodyFlags {
    LBF_RawCall = (1 << 0),
    LBF_Complete = (1 << 1)
};

struct Body {
    const Anchor *anchor;
    Any enter;
    Args args;
    uint64_t flags;

    // if there's a scope label, the current frame will be truncated to the
    // parent frame that maps the scope label.
    Label *scope_label;

    Body();

    bool is_complete() const;
    void set_complete();
    void unset_complete();

    bool is_rawcall();

    void set_rawcall(bool enable = true);

    void copy_traits_from(const Body &other);
};


} // namespace scopes

#endif // SCOPES_BODY_HPP
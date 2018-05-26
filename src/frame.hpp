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

#ifndef SCOPES_FRAME_HPP
#define SCOPES_FRAME_HPP

#include "argument.hpp"

#include <stddef.h>

namespace scopes {

struct Label;

struct Frame {
    Frame();
    Frame(Frame *_parent, Label *_label, Label *_instance = nullptr, size_t _loop_count = 0);

    Args args;
    Frame *parent;
    Label *label;
    size_t loop_count;
    bool inline_merge;

    Frame *find_parent_frame(Label *label);

    static Frame *from(Frame *parent, Label *label, Label *instance, size_t loop_count);

    bool all_args_constant() const;

    struct ArgsKey {
        Label *label;
        scopes::Args args;

        ArgsKey();

        bool operator==(const ArgsKey &other) const;

        struct Hash {
            std::size_t operator()(const ArgsKey& s) const;
        };

    };

    Frame *find_frame(const ArgsKey &key) const;

    Frame *find_any_frame(Label *label, ArgsKey &key) const;

    void insert_frame(const ArgsKey &key, Frame *frame);

    Label *get_instance() const;

    static Frame *root;
protected:
    std::unordered_map<ArgsKey, Frame *, ArgsKey::Hash> frames;
    Label *instance;
};

} // namespace scopes

#endif // SCOPES_FRAME_HPP
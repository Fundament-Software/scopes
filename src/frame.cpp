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

#include "frame.hpp"
#include "label.hpp"

#include <assert.h>

#include "cityhash/city.h"

namespace scopes {

Frame::Frame() :
    parent(nullptr),
    label(nullptr),
    loop_count(0),
    inline_merge(false),
    instance(nullptr)
{}
Frame::Frame(Frame *_parent, Label *_label, Label *_instance, size_t _loop_count) :
    parent(_parent),
    label(_label),
    loop_count(_loop_count),
    inline_merge(false),
    instance(_instance) {
    assert(parent);
    assert(label);
    args.reserve(_label->params.size());
}

Frame *Frame::find_parent_frame(Label *label) {
    Frame *top = this;
    while (top) {
        if (top->label == label) {
            return top;
        }
        top = top->parent;
    }
    return nullptr;
}

Frame *Frame::from(Frame *parent, Label *label, Label *instance, size_t loop_count) {
    return new Frame(parent, label, instance, loop_count);
}

bool Frame::all_args_constant() const {
    for (size_t i = 1; i < args.size(); ++i) {
        if (is_unknown(args[i].value))
            return false;
        if (!args[i].value.is_const())
            return false;
    }
    return true;
}

Frame::ArgsKey::ArgsKey() : label(nullptr) {}

bool Frame::ArgsKey::operator==(const ArgsKey &other) const {
    if (label != other.label) return false;
    if (args.size() != other.args.size()) return false;
    for (size_t i = 0; i < args.size(); ++i) {
        auto &&a = args[i];
        auto &&b = other.args[i];
        if (a != b)
            return false;
    }
    return true;
}

std::size_t Frame::ArgsKey::Hash::operator()(const ArgsKey& s) const {
    std::size_t h = std::hash<Label *>{}(s.label);
    for (auto &&arg : s.args) {
        h = HashLen16(h, arg.hash());
    }
    return h;
}

Frame *Frame::find_frame(const ArgsKey &key) const {
    auto it = frames.find(key);
    if (it != frames.end())
        return it->second;
    return nullptr;
}

Frame *Frame::find_any_frame(Label *label, ArgsKey &key) const {
    for (auto &&it : frames) {
        if (it.first.label == label) {
            key = it.first;
            return it.second;
        }
    }
    return nullptr;
}

void Frame::insert_frame(const ArgsKey &key, Frame *frame) {
    frames.insert({key, frame});
}

Label *Frame::get_instance() const {
    return instance;
}

Frame *Frame::root = nullptr;

} // namespace scopes

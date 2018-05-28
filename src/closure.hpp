/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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

} // namespace scopes

#endif // SCOPES_CLOSURE_HPP
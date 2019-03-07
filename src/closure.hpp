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

struct Template;
struct Function;
struct StyledStream;

struct Closure {
protected:

    Closure(Template *_func, Function *_frame);

public:

    struct Hash {
        std::size_t operator()(const Closure &k) const;
    };

    bool operator ==(const Closure &k) const;

    static std::unordered_map<Closure, const Closure *, Closure::Hash> map;

    Template *func;
    Function *frame;

    static const Closure *from(Template *func, Function *frame);

    StyledStream &stream(StyledStream &ost) const;
};

} // namespace scopes

#endif // SCOPES_CLOSURE_HPP
/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STREAM_ANCHORS_HPP
#define SCOPES_STREAM_ANCHORS_HPP

namespace scopes {

struct StyledStream;
struct Anchor;

struct StreamAnchors {
    StyledStream &ss;
    const Anchor *last_anchor;

    StreamAnchors(StyledStream &_ss);

    void stream_anchor(const Anchor *anchor, bool quoted = false);
};

} // namespace scopes

#endif // SCOPES_STREAM_ANCHORS_HPP
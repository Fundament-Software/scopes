/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_anchors.hpp"
#include "styled_stream.hpp"
#include "anchor.hpp"

namespace scopes {

//------------------------------------------------------------------------------

StreamAnchors::StreamAnchors(StyledStream &_ss) :
    ss(_ss), last_anchor(nullptr) {
}

void StreamAnchors::stream_anchor(const Anchor *anchor, bool quoted) {
    if (anchor) {
        ss << Style_Location;
        auto rss = StyledStream::plain(ss);
        // ss << path.name()->data << ":" << lineno << ":" << column << ":";
        if (!last_anchor || (last_anchor->path != anchor->path)) {
            rss << anchor->path.name()->data
                << ":" << anchor->lineno
                << ":" << anchor->column
                << ":";
        } else if (!last_anchor || (last_anchor->lineno != anchor->lineno)) {
            rss << ":" << anchor->lineno
                << ":" << anchor->column
                << ":";
        } else if (!last_anchor || (last_anchor->column != anchor->column)) {
            rss << "::" << anchor->column
                << ":";
        } else {
            rss << ":::";
        }
        if (quoted) { rss << "'"; }
        ss << Style_None;
        last_anchor = anchor;
    }
}

} // namespace scopes

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
        if (!last_anchor || (last_anchor->path() != anchor->path())) {
            rss << anchor->path().name()->data
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

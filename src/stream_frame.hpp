/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STREAM_FRAME_HPP
#define SCOPES_STREAM_FRAME_HPP

#include "stream_anchors.hpp"

namespace scopes {

struct Frame;

//------------------------------------------------------------------------------
// FRAME PRINTER
//------------------------------------------------------------------------------

struct StreamFrameFormat {
    enum Tagging {
        All,
        None,
    };

    Tagging follow;

    StreamFrameFormat();

    static StreamFrameFormat single();
};

struct StreamFrame : StreamAnchors {
    bool follow_all;
    StreamFrameFormat fmt;

    StreamFrame(StyledStream &_ss, const StreamFrameFormat &_fmt);

    void stream_frame(const Frame *frame);

    void stream(const Frame *frame);
};

void stream_frame(
    StyledStream &_ss, const Frame *frame, const StreamFrameFormat &_fmt);

} // namespace scopes

#endif // SCOPES_STREAM_FRAME_HPP
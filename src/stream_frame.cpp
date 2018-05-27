/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_frame.hpp"
#include "frame.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// FRAME PRINTER
//------------------------------------------------------------------------------

StreamFrameFormat::StreamFrameFormat()
    : follow(All)
{}

StreamFrameFormat StreamFrameFormat::single() {
    StreamFrameFormat fmt;
    fmt.follow = None;
    return fmt;
}

//------------------------------------------------------------------------------

StreamFrame::StreamFrame(StyledStream &_ss, const StreamFrameFormat &_fmt) :
    StreamAnchors(_ss), fmt(_fmt) {
    follow_all = (fmt.follow == StreamFrameFormat::All);
}

void StreamFrame::stream_frame(const Frame *frame) {
    if (follow_all) {
        if (frame->parent != Frame::root)
            stream_frame(frame->parent);
    }
    ss << frame;
    if (frame->loop_count) {
        ss << " [loop=" << frame->loop_count << "]" << std::endl;
    }
    ss << std::endl;
    //ss << "    instance = " << frame->instance << std::endl;
    ss << "    original label = " << frame->label << std::endl;
    auto &&args = frame->args;
    for (size_t i = 0; i < args.size(); ++i) {
        ss << "    " << i << " = " << args[i] << std::endl;
    }
}

void StreamFrame::stream(const Frame *frame) {
    stream_frame(frame);
}

//------------------------------------------------------------------------------

void stream_frame(
    StyledStream &_ss, const Frame *frame, const StreamFrameFormat &_fmt) {
    StreamFrame streamer(_ss, _fmt);
    streamer.stream(frame);
}

} // namespace scopes

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

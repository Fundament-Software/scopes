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

#include "error.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "main.hpp"

#include "scopes.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// ERROR HANDLING
//------------------------------------------------------------------------------

static const Anchor *_active_anchor = nullptr;

void set_active_anchor(const Anchor *anchor) {
    assert(anchor);
    _active_anchor = anchor;
}

const Anchor *get_active_anchor() {
    return _active_anchor;
}

//------------------------------------------------------------------------------

Exception::Exception() :
    anchor(nullptr),
    msg(nullptr) {}

Exception::Exception(const Anchor *_anchor, const String *_msg) :
    anchor(_anchor),
    msg(_msg) {}

//------------------------------------------------------------------------------

ExceptionPad::ExceptionPad() : value(none) {
}

void ExceptionPad::invoke(const Any &value) {
    this->value = value;
    longjmp(retaddr, 1);
}

//------------------------------------------------------------------------------

ExceptionPad *_exc_pad = nullptr;

void location_message(const Anchor *anchor, const String* str) {
    assert(anchor);
    auto cerr = StyledStream(std::cerr);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

void print_exception(const Any &value) {
    auto cerr = StyledStream(std::cerr);
    if (value.type == TYPE_Exception) {
        const Exception *exc = value;
        if (exc->anchor) {
            cerr << exc->anchor << " ";
        }
        cerr << Style_Error << "error:" << Style_None << " "
            << exc->msg->data << std::endl;
        if (exc->anchor) {
            exc->anchor->stream_source_line(cerr);
        }
    } else {
        cerr << "exception raised: " << value << std::endl;
    }
}

static void default_exception_handler(const Any &value) {
    print_exception(value);
    f_abort();
}

void error(const Any &value) {
#if SCOPES_EARLY_ABORT
    default_exception_handler(value);
#else
    if (!_exc_pad) {
        default_exception_handler(value);
    } else {
        _exc_pad->invoke(value);
    }
#endif
}

void location_error(const String *msg) {
    const Exception *exc = new Exception(_active_anchor, msg);
    error(exc);
}

} // namespace scopes

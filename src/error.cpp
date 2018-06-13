/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "error.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "boot.hpp"

#include "scopes/config.h"

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
    auto cerr = StyledStream(SCOPES_CERR);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

void print_exception(const Any &value) {
    auto cerr = StyledStream(SCOPES_CERR);
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

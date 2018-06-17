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

Error::Error() :
    anchor(nullptr),
    msg(nullptr) {}

Error::Error(const Anchor *_anchor, const String *_msg) :
    anchor(_anchor),
    msg(_msg) {}

//------------------------------------------------------------------------------

static Any _last_error = none;

void set_last_error(const Any &err) {
    _last_error = err;
}

Any get_last_error() {
    Any result = _last_error;
    _last_error = none;
    return result;
}

//------------------------------------------------------------------------------

void location_message(const Anchor *anchor, const String* str) {
    assert(anchor);
    auto cerr = StyledStream(SCOPES_CERR);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

void stream_error_string(StyledStream &ss, const Any &value) {
    if (value.type == TYPE_Error) {
        const Error *exc = value;
        ss << exc->msg->data;
    } else {
        ss << "exception raised: " << value;
    }
}

void stream_error(StyledStream &ss, const Any &value) {
    if (value.type == TYPE_Error) {
        const Error *exc = value;
        if (exc->anchor) {
            ss << exc->anchor << " ";
        }
        ss << Style_Error << "error:" << Style_None << " "
            << exc->msg->data << std::endl;
        if (exc->anchor) {
            exc->anchor->stream_source_line(ss);
        }
    } else {
        ss << "exception raised: " << value << std::endl;
    }
}

void print_error(const Any &value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Any make_location_error(const String *msg) {
    const Error *exc = new Error(_active_anchor, msg);
    return exc;
}

Any make_runtime_error(const String *msg) {
    const Error *exc = new Error(nullptr, msg);
    return exc;
}

void set_last_location_error(const String *msg) {
    set_last_error(make_location_error(msg));
}

} // namespace scopes

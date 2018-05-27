/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ERROR_HPP
#define SCOPES_ERROR_HPP

#include "any.hpp"

#ifdef SCOPES_WIN32
#include <setjmpex.h>
#else
#include <setjmp.h>
#endif

namespace scopes {

struct String;
struct Anchor;

void set_active_anchor(const Anchor *anchor);
const Anchor *get_active_anchor();

struct Exception {
    const Anchor *anchor;
    const String *msg;

    Exception();

    Exception(const Anchor *_anchor, const String *_msg);
};

struct ExceptionPad {
    jmp_buf retaddr;
    Any value;

    ExceptionPad();

    void invoke(const Any &value);
};

#ifdef SCOPES_WIN32
#define SCOPES_TRY() \
    ExceptionPad exc_pad; \
    ExceptionPad *_last_exc_pad = _exc_pad; \
    _exc_pad = &exc_pad; \
    if (!_setjmpex(exc_pad.retaddr, nullptr)) {
#else
#define SCOPES_TRY() \
    ExceptionPad exc_pad; \
    ExceptionPad *_last_exc_pad = _exc_pad; \
    _exc_pad = &exc_pad; \
    if (!setjmp(exc_pad.retaddr)) {
#endif

#define SCOPES_CATCH(EXCNAME) \
        _exc_pad = _last_exc_pad; \
    } else { \
        _exc_pad = _last_exc_pad; \
        auto &&EXCNAME = exc_pad.value;

#define SCOPES_TRY_END() \
    }

extern ExceptionPad *_exc_pad;

void print_exception(const Any &value);
void error(const Any &value);

void location_error(const String *msg);
void location_message(const Anchor *anchor, const String* str);
} // namespace scopes

#endif // SCOPES_ERROR_HPP
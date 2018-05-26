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
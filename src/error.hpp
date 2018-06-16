/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ERROR_HPP
#define SCOPES_ERROR_HPP

#include "any.hpp"
#include "result.hpp"
#include "scopes/config.h"

namespace scopes {

//------------------------------------------------------------------------------

struct String;
struct Anchor;

void set_active_anchor(const Anchor *anchor);
const Anchor *get_active_anchor();

//------------------------------------------------------------------------------

struct Error {
    const Anchor *anchor;
    const String *msg;

    Error();

    Error(const Anchor *_anchor, const String *_msg);
};

//------------------------------------------------------------------------------

void set_last_error(const Any &err);
Any get_last_error();

void print_error(const Any &value);

void set_last_location_error(const String *msg);

#if SCOPES_EARLY_ABORT
#define SCOPES_LOCATION_ERROR(MSG) \
    set_last_location_error((MSG)); \
    assert(false); \
    return Result<_result_type>();
#else
#define SCOPES_LOCATION_ERROR(MSG) \
    set_last_location_error((MSG)); \
    return Result<_result_type>();
#endif

void location_message(const Anchor *anchor, const String* str);

} // namespace scopes

#endif // SCOPES_ERROR_HPP
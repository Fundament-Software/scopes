/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_RESULT_HPP
#define SCOPES_RESULT_HPP

#include <assert.h>

#pragma GCC diagnostic error "-Wunused-result"
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

namespace scopes {

struct Error;

// use this as return type; together with the attribute and the warning-as-error
// setting above, we get a good trap for any ignores of result values
#define SCOPES_RESULT(T) Result<T> __attribute__ ((warn_unused_result))

// declared at top of function so the subsequent macros all work
#define SCOPES_RESULT_TYPE(T) \
    (void)0; /* ensure macro can only be used in function bodies */ \
    typedef T _result_type;

#define SCOPES_RETURN_ERROR(ERR) return Result<_result_type>::raise(ERR);
// if ok fails, return
#define SCOPES_CHECK_OK(OK, ERR) if (!OK) { SCOPES_RETURN_ERROR(ERR); }
// if an expression returning a result fails, return
#define SCOPES_CHECK_RESULT(EXPR) { \
    auto _result = (EXPR); \
    SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
}
// execute expression and return an error
#define SCOPES_EXPECT_ERROR(EXPR) {\
    auto _tmp = (EXPR); \
    assert(!_tmp.ok()); \
    SCOPES_RETURN_ERROR(_tmp.unsafe_error()); \
}
// try to extract a value from a result or return
#define SCOPES_GET_RESULT(EXPR) ({ \
        auto _result = (EXPR); \
        SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
        _result.unsafe_extract(); \
    })
#define SCOPES_ASSERT_CAST(T, EXPR) ({ \
        Result<T> _result = (EXPR); \
        _result.assert_ok(); \
    })
#define SCOPES_CHECK_CAST(T, EXPR) ({ \
        Result<T> _result = (EXPR); \
        SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
        _result.unsafe_extract(); \
    })

template<typename T>
struct Result {
    Result(const T &value) : _value(value), _error(nullptr), _ok(true) {}

    inline bool ok() const { return _ok; }
    inline const T &assert_ok() const { assert(_ok); return _value; }
    inline const T &unsafe_extract() const { return _value; }
    inline Error *assert_error() const { assert(!_ok); return _error; }
    inline Error *unsafe_error() const { return _error; }

    static Result<T> raise(Error *err) { return Result<T>(err, false); }

private:
    Result(Error *error, bool ok) : _error(error), _ok(ok) {}

    T _value;
    Error *_error;
    bool _ok;
};

template<>
struct Result<void> {
    Result(void) : _ok(true) {}

    inline bool ok() const { return _ok; }
    inline void assert_ok() const { assert(_ok); }
    inline Error *assert_error() const { assert(!_ok); return _error; }
    inline Error *unsafe_error() const { return _error; }

    static Result<void> raise(Error *err) { return Result<void>(err); }

private:
    Result(Error *err) : _error(err), _ok(false) { assert(err); }

    Error *_error;
    bool _ok;
};

} // namespace scopes

#endif // SCOPES_RESULT_HPP
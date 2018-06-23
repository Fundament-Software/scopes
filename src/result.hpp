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

// use this as return type; together with the attribute and the warning-as-error
// setting above, we get a good trap for any ignores of result values
#define SCOPES_RESULT(T) Result<T> __attribute__ ((warn_unused_result))

// declared at top of function so the subsequent macros all work
#define SCOPES_RESULT_TYPE(T) \
    (void)0; /* ensure macro can only be used in function bodies */ \
    typedef T _result_type;

#define SCOPES_RETURN_ERROR() return Result<_result_type>();
// if ok fails, return
#define SCOPES_CHECK_OK(OK) if (!OK) { SCOPES_RETURN_ERROR(); }
// if an expression returning a result fails, return
#define SCOPES_CHECK_RESULT(EXPR) SCOPES_CHECK_OK((EXPR).ok())
// execute expression and return an error
#define SCOPES_EXPECT_ERROR(EXPR) {\
    auto _tmp = (EXPR); \
    assert(!_tmp.ok()); \
    SCOPES_RETURN_ERROR(); \
}
// try to extract a value from a result or return
#define SCOPES_GET_RESULT(EXPR) ({ \
        auto _result = (EXPR); \
        SCOPES_CHECK_RESULT(_result); \
        _result.assert_ok(); \
    })
#define SCOPES_ASSERT_CAST(T, EXPR) ({ \
        Result<T> _result = (EXPR); \
        _result.assert_ok(); \
    })
#define SCOPES_CHECK_CAST(T, EXPR) ({ \
        Result<T> _result = (EXPR); \
        SCOPES_CHECK_RESULT(_result); \
        _result.assert_ok(); \
    })

template<typename T>
struct Result {
    Result(const T &value) : _value(value), _ok(true) {}
    Result() : _ok(false) {}

    inline bool ok() const { return _ok; }
    inline const T &assert_ok() const { assert(_ok); return _value; }
    inline const T &unsafe_extract() const { return _value; }

protected:
    T _value;
    bool _ok;
};

template<>
struct Result<void> {
    Result(bool ok) : _ok(ok) {}
    Result() : _ok(false) {}

    inline bool ok() const { return _ok; }
    inline void assert_ok() const { assert(_ok); }

protected:
    bool _ok;
};

} // namespace scopes

#endif // SCOPES_RESULT_HPP
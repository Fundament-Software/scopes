/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "string.hpp"
#include "gc.hpp"
#include "utils.hpp"
#include "hash.hpp"

#define STB_SPRINTF_DECORATE(name) stb_##name
#define STB_SPRINTF_NOUNALIGNED
#include "stb_sprintf.h"

#include <assert.h>
#include <memory.h>
#include <string.h>

#include <locale>
#include <codecvt>
#include <unordered_set>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

static char parse_hexchar(char c) {
    if ((c >= '0') && (c <= '9')) {
        return c - '0';
    } else if ((c >= 'a') && (c <= 'f')) {
        return c - 'a' + 10;
    } else if ((c >= 'A') && (c <= 'F')) {
        return c - 'A' + 10;
    }
    return -1;
}

int unescape_string(char *buf) {
    char *dst = buf;
    char *src = buf;
    while (*src) {
        if (*src == '\\') {
            src++;
            if (*src == 0) {
                break;
            } if (*src == 'n') {
                *dst = '\n';
            } else if (*src == 't') {
                *dst = '\t';
            } else if (*src == 'r') {
                *dst = '\r';
            } else if (*src == 'x') {
                char c0 = parse_hexchar(*(src + 1));
                char c1 = parse_hexchar(*(src + 2));
                if ((c0 >= 0) && (c1 >= 0)) {
                    *dst = (c0 << 4) | c1;
                    src += 2;
                } else {
                    src--;
                    *dst = *src;
                }
            } else {
                *dst = *src;
            }
        } else {
            *dst = *src;
        }
        src++;
        dst++;
    }
    // terminate
    *dst = 0;
    return dst - buf;
}

#define B_SNFORMAT 512 // how many characters per callback
typedef char *(*vsformatcb_t)(const char *buf, void *user, int len);

struct vsformat_cb_ctx {
    int count;
    char *dest;
    char tmp[B_SNFORMAT];
};

static char *vsformat_cb(const char *buf, void *user, int len) {
    vsformat_cb_ctx *ctx = (vsformat_cb_ctx *)user;
    if (buf != ctx->dest) {
        char *d = ctx->dest;
        char *e = d + len;
        while (d != e) {
            *d++ = *buf++;
        }
    }
    ctx->dest += len;
    return ctx->tmp;
}

static char *vsformat_cb_null(const char *buf, void *user, int len) {
    vsformat_cb_ctx *ctx = (vsformat_cb_ctx *)user;
    ctx->count += len;
    return ctx->tmp;
}

static int escapestrcb(vsformatcb_t cb, void *user, char *buf,
    const char *str, int strcount,
    const char *quote_chars = nullptr) {
    assert(buf);
    const char *fmt_start = str;
    const char *fmt = fmt_start;
    char *p = buf;
#define VSFCB_CHECKWRITE(N) \
    if (((p - buf) + (N)) > B_SNFORMAT) { buf = p = cb(buf, user, p - buf); }
#define VSFCB_PRINT(MAXCOUNT, FMT, SRC) { \
        VSFCB_CHECKWRITE(MAXCOUNT+1); \
        p += snprintf(p, B_SNFORMAT - (p - buf), FMT, SRC); }
    for(;;) {
        char c = *fmt;
        switch(c) {
        case '\n': VSFCB_CHECKWRITE(2); *p++ = '\\'; *p++ = 'n'; break;
        case '\r': VSFCB_CHECKWRITE(2); *p++ = '\\'; *p++ = 'r'; break;
        case '\t': VSFCB_CHECKWRITE(2); *p++ = '\\'; *p++ = 't'; break;
        case 0: if ((fmt - fmt_start) == strcount) goto done;
            // otherwise, fall through
        default:
            if ((c < 32) || (c >= 127)) {
                VSFCB_PRINT(4, "\\x%02x", (unsigned char)c);
            } else {
                if ((c == '\\') || (quote_chars && strchr(quote_chars, c))) {
                    VSFCB_CHECKWRITE(1);
                    *p++ = '\\';
                }
                *p++ = c;
            }
            break;
        }
        fmt++;
    }
done:
    VSFCB_CHECKWRITE(B_SNFORMAT); // force flush if non-empty
    return 0;
#undef VSFCB_CHECKWRITE
#undef VSFCB_PRINT
}

int escape_string(char *buf, const char *str, int strcount, const char *quote_chars) {
    vsformat_cb_ctx ctx;
    if (buf) {
        ctx.dest = buf;
        escapestrcb(vsformat_cb, &ctx, ctx.tmp, str, strcount, quote_chars);
        int l = ctx.dest - buf;
        buf[l] = 0;
        return l;
    } else {
        ctx.count = 0;
        escapestrcb(vsformat_cb_null, &ctx, ctx.tmp, str, strcount, quote_chars);
        return ctx.count + 1;
    }
}

//------------------------------------------------------------------------------
// STRING
//------------------------------------------------------------------------------

static std::unordered_set<const String *, String::Hash, String::KeyEqual> string_map;

//------------------------------------------------------------------------------

std::size_t String::Hash::operator()(const String *s) const {
    return s->hash();
}

bool String::KeyEqual::operator()( const String *lhs, const String *rhs ) const {
    if (lhs->count == rhs->count) {
        return !memcmp(lhs->data, rhs->data, lhs->count);
    }
    return false;
}

//------------------------------------------------------------------------------

std::size_t String::hash() const {
    return hash_bytes(data, count);
}

String::String(const char *_data, size_t _count)
    : data(_data), count(_count) {}

const String *String::from(const char *buf, size_t count) {
    String key(buf, count);
    auto it = string_map.find(&key);
    if (it != string_map.end()) {
        return *it;
    }
    char *s = (char *)tracked_malloc(sizeof(char) * (count + 1));
    memcpy(s, buf, count * sizeof(char));
    s[count] = 0;
    const String *str = new String(s, count);
    string_map.insert(str);
    return str;
}

const String *String::from_cstr(const char *s) {
    return from(s, strlen(s));
}

// small strings on the stack, big strings on the heap
#define SCOPES_BEGIN_TEMP_STRING(NAME, SIZE) \
    bool NAME ## _use_stack = ((SIZE) < 1024); \
    char stack ## NAME[NAME ## _use_stack?((SIZE)+1):1]; \
    char *NAME = (NAME ## _use_stack?stack ## NAME:((char *)malloc(sizeof(char) * ((SIZE)+1))));

#define SCOPES_END_TEMP_STRING(NAME) \
    if (!NAME ## _use_stack) free(NAME);

const String *String::join(const String *a, const String *b) {
    size_t ac = a->count;
    size_t bc = b->count;
    size_t cc = ac + bc;
    SCOPES_BEGIN_TEMP_STRING(tmp, cc);
    memcpy(tmp, a->data, sizeof(char) * ac);
    memcpy(tmp + ac, b->data, sizeof(char) * bc);
    const String *result = String::from(tmp, cc);
    SCOPES_END_TEMP_STRING(tmp);
    return result;
}

const String *String::from_stdstring(const std::string &s) {
    return from(s.c_str(), s.size());
}

const String *String::from_stdstring(const std::wstring &ws) {
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string s = converter.to_bytes(ws);
    return from_stdstring(s);
}

StyledStream& String::stream(StyledStream& ost, const char *escape_chars) const {
    auto c = escape_string(nullptr, data, count, escape_chars);
    SCOPES_BEGIN_TEMP_STRING(tmp, c);
    escape_string(tmp, data, count, escape_chars);
    ost << tmp;
    SCOPES_END_TEMP_STRING(tmp);
    return ost;
}

const String *String::substr(int64_t i0, int64_t i1) const {
    assert(i1 >= i0);
    return from(data + i0, (size_t)(i1 - i0));
}

StyledStream& operator<<(StyledStream& ost, const String *s) {
    ost << Style_String << "\"";
    s->stream(ost, "\"");
    ost << "\"" << Style_None;
    return ost;
}

//------------------------------------------------------------------------------

StyledString::StyledString() :
    out(_ss) {
}

StyledString::StyledString(StreamStyleFunction ssf) :
    out(_ss, ssf) {
}

StyledString StyledString::plain() {
    return StyledString(stream_plain_style);
}

const String *StyledString::str() const {
    return String::from_stdstring(_ss.str());
}

CppString StyledString::cppstr() const {
    return _ss.str();
}

//------------------------------------------------------------------------------

const String *vformat( const char *fmt, va_list va ) {
    va_list va2;
    va_copy(va2, va);
    size_t size = stb_vsnprintf( nullptr, 0, fmt, va2 );
    va_end(va2);
    SCOPES_BEGIN_TEMP_STRING(tmp, size);
    stb_vsnprintf( tmp, size + 1, fmt, va );
    const String *result = String::from_cstr(tmp);
    SCOPES_END_TEMP_STRING(tmp);
    return result;
}

const String *format( const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    const String *result = vformat(fmt, va);
    va_end(va);
    return result;
}

// computes the levenshtein distance between two strings
size_t distance(const String *_s, const String *_t) {
    const char *s = _s->data;
    const char *t = _t->data;
    const size_t n = _s->count;
    const size_t m = _t->count;
    if (!m) return n;
    if (!n) return m;

    size_t _v0[m + 1];
    size_t _v1[m + 1];

    size_t *v0 = _v0;
    size_t *v1 = _v1;
    for (size_t i = 0; i <= m; ++i) {
        v0[i] = i;
    }

    for (size_t i = 0; i < n; ++i) {
        v1[0] = i + 1;

        for (size_t j = 0; j < m; ++j) {
            size_t cost = (s[i] == t[j])?0:1;
            v1[j + 1] = std::min(v1[j] + 1,
                std::min(v0[j + 1] + 1, v0[j] + cost));
        }

        size_t *tmp = v0;
        v0 = v1;
        v1 = tmp;
    }

    //std::cout << "lev(" << s << ", " << t << ") = " << v0[m] << std::endl;

    return v0[m];
}

} // namespace scopes

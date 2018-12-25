/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "styled_stream.hpp"

#define STB_SPRINTF_DECORATE(name) stb_##name
#define STB_SPRINTF_NOUNALIGNED
#include "stb_sprintf.h"

#pragma GCC diagnostic ignored "-Wvla-extension"

#if SCOPES_USE_WCHAR
#include <locale>
#include <windows.h>
#endif

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// ANSI COLOR FORMATTING
//------------------------------------------------------------------------------

#define RGBCOLORS

namespace ANSI {
static const char RESET[]           = "\033[0m";
#ifdef RGBCOLORS
static const char COLOR_YELLOW[]    = "\033[33m";
static const char COLOR_XYELLOW[]   = "\033[33;1m";
static const char COLOR_XRED[]      = "\033[31;1m";
#else
static const char COLOR_BLACK[]     = "\033[30m";
static const char COLOR_RED[]       = "\033[31m";
static const char COLOR_GREEN[]     = "\033[32m";
static const char COLOR_YELLOW[]    = "\033[33m";
static const char COLOR_BLUE[]      = "\033[34m";
static const char COLOR_MAGENTA[]   = "\033[35m";
static const char COLOR_CYAN[]      = "\033[36m";
static const char COLOR_GRAY60[]    = "\033[37m";

static const char COLOR_GRAY30[]    = "\033[30;1m";
static const char COLOR_XRED[]      = "\033[31;1m";
static const char COLOR_XGREEN[]    = "\033[32;1m";
static const char COLOR_XYELLOW[]   = "\033[33;1m";
static const char COLOR_XBLUE[]     = "\033[34;1m";
static const char COLOR_XMAGENTA[]  = "\033[35;1m";
static const char COLOR_XCYAN[]     = "\033[36;1m";
static const char COLOR_WHITE[]     = "\033[37;1m";
#endif

static void COLOR_RGB(OStream &ost, const char prefix[], int hexcode) {
    ost << prefix
        << ((hexcode >> 16) & 0xff) << ";"
        << ((hexcode >> 8) & 0xff) << ";"
        << (hexcode & 0xff) << "m";
}

static void COLOR_RGB_FG(OStream &ost, int hexcode) {
    return COLOR_RGB(ost, "\033[38;2;", hexcode);
}
#if 0
static void COLOR_RGB_BG(OStream &ost, int hexcode) {
    return COLOR_RGB(ost, "\033[48;2;", hexcode);
}
#endif

} // namespace ANSI

// support 24-bit ANSI colors (ISO-8613-3)
// works on most bash shells as well as windows 10
void ansi_from_style(OStream &ost, Style style) {
    switch(style) {
#ifdef RGBCOLORS
    case Style_None: ost << ANSI::RESET; break;
    case Style_Symbol: ANSI::COLOR_RGB_FG(ost, 0xCCCCCC); break;
    case Style_String: ANSI::COLOR_RGB_FG(ost, 0x99CC99); break;
    case Style_Number: ANSI::COLOR_RGB_FG(ost, 0xF99157); break;
    case Style_Keyword: ANSI::COLOR_RGB_FG(ost, 0xCC99CC); break;
    case Style_Function: ANSI::COLOR_RGB_FG(ost, 0x6699CC); break;
    case Style_SfxFunction: ANSI::COLOR_RGB_FG(ost, 0xCC6666); break;
    case Style_Operator: ANSI::COLOR_RGB_FG(ost, 0x66CCCC); break;
    case Style_Instruction: ost << ANSI::COLOR_YELLOW; break;
    case Style_Type: ANSI::COLOR_RGB_FG(ost, 0xFFCC66); break;
    case Style_Comment: ANSI::COLOR_RGB_FG(ost, 0x999999); break;
    case Style_Error: ost << ANSI::COLOR_XRED; break;
    case Style_Warning: ost << ANSI::COLOR_XYELLOW; break;
    case Style_Location: ANSI::COLOR_RGB_FG(ost, 0x999999); break;
#else
    case Style_None: ost << ANSI::RESET; break;
    case Style_Symbol: ost << ANSI::COLOR_GRAY60; break;
    case Style_String: ost << ANSI::COLOR_XMAGENTA; break;
    case Style_Number: ost << ANSI::COLOR_XGREEN; break;
    case Style_Keyword: ost << ANSI::COLOR_XBLUE; break;
    case Style_Function: ost << ANSI::COLOR_GREEN; break;
    case Style_SfxFunction: ost << ANSI::COLOR_RED; break;
    case Style_Operator: ost << ANSI::COLOR_XCYAN; break;
    case Style_Instruction: ost << ANSI::COLOR_YELLOW; break;
    case Style_Type: ost << ANSI::COLOR_XYELLOW; break;
    case Style_Comment: ost << ANSI::COLOR_GRAY30; break;
    case Style_Error: ost << ANSI::COLOR_XRED; break;
    case Style_Warning: ost << ANSI::COLOR_XYELLOW; break;
    case Style_Location: ost << ANSI::COLOR_GRAY30; break;
#endif
    default: break;
    }
}
#undef RGBCOLORS

void stream_ansi_style(OStream &ost, Style style) {
    ansi_from_style(ost, style);
}

void stream_plain_style(OStream &ost, Style style) {
}

StreamStyleFunction stream_default_style = stream_plain_style;

StyledStream::StyledStream(OStream &ost, StreamStyleFunction ssf) :
    _ssf(ssf),
    _ost(ost)
{}

StyledStream::StyledStream(OStream &ost) :
    _ssf(stream_default_style),
    _ost(ost)
{}

StyledStream::StyledStream() :
    _ssf(stream_default_style),
    _ost(SCOPES_CERR)
{}

StyledStream StyledStream::plain(OStream &ost) {
    return StyledStream(ost, stream_plain_style);
}

StyledStream StyledStream::plain(StyledStream &ost) {
    return StyledStream(ost._ost, stream_plain_style);
}

StyledStream& StyledStream::operator<<(OStream &(*o)(OStream&)) {
    _ost << o; return *this; }

StyledStream& StyledStream::operator<<(Style s) {
    _ssf(_ost, s);
    return *this;
}

StyledStream& StyledStream::operator<<(bool s) {
    _ssf(_ost, Style_Keyword);
    _ost << (s?"true":"false");
    _ssf(_ost, Style_None);
    return *this;
}

StyledStream& StyledStream::stream_number(int8_t x) {
    _ssf(_ost, Style_Number); _ost << (int)x; _ssf(_ost, Style_None);
    return *this;
}

StyledStream& StyledStream::stream_number(uint8_t x) {
    _ssf(_ost, Style_Number); _ost << (int)x; _ssf(_ost, Style_None);
    return *this;
}

StyledStream& StyledStream::stream_number(double x) {
    size_t size = stb_snprintf( nullptr, 0, "%g", x );
    char dest[size+1];
    stb_snprintf( dest, size + 1, "%g", x );
    _ssf(_ost, Style_Number); _ost << dest; _ssf(_ost, Style_None);
    return *this;
}

StyledStream& StyledStream::stream_number(float x) {
    return stream_number((double)x);
}

#if SCOPES_USE_WCHAR
StyledStream& StyledStream::operator<<(const char * const s) {
    int sz = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (sz) {
        wchar_t buf[sz];
        MultiByteToWideChar(CP_UTF8, 0, s, -1, buf, sz);
        _ost << buf;
    }
    return *this;
}

StyledStream& StyledStream::operator<<(const std::string &s) {
    return *this << s.c_str();
}

#endif

//------------------------------------------------------------------------------

// based on https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
static uint32_t permute(uint32_t x, uint32_t prime) {
    if (x >= prime)
        return x;
    uint32_t residue = ((uint64_t) x * x) % prime;
    return (x <= prime / 2) ? residue : prime - residue;
}

static uint64_t scramble(uint64_t x) {
    // find msb
    int bits = -1;
    for (int i = 64; i-- > 0;) {
        if (x & (1ull << i)) {
            bits = i;
            break;
        }
    }
    if (bits < 1) return x;
    // scramble up to first 32 bits
    bits = std::min(32,bits);
    uint64_t mask = (1ull << bits) - 1ull;
    uint64_t top = x & ~mask;
    x = x & mask;
    if (bits >= 8) {
        uint32_t delta = 0;
        // from https://primes.utm.edu/lists/2small/0bit.html
        switch(bits) {
        case  8: delta =  5; break; case  9: delta =  3; break; case 10: delta =  3; break; case 11: delta =  9; break;
        case 12: delta =  3; break; case 13: delta =  1; break; case 14: delta =  3; break; case 15: delta = 19; break;
        case 16: delta = 15; break; case 17: delta =  1; break; case 18: delta =  5; break; case 19: delta =  1; break;
        case 20: delta =  3; break; case 21: delta =  9; break; case 22: delta =  3; break; case 23: delta = 15; break;
        case 24: delta =  3; break; case 25: delta = 39; break; case 26: delta =  5; break; case 27: delta = 39; break;
        case 28: delta = 57; break; case 29: delta =  3; break; case 30: delta = 35; break; case 31: delta =  1; break;
        case 32: delta =  5; break; default: assert(false); break;
        }
        uint32_t prime = (1u << bits) - delta;
        uint32_t m = 0x5bf03635 & mask;
        x = permute(x, prime);
        x = permute(x ^ m, prime);
    }
    return x | top;
}

#if 1
// alternating base 15 / base 5
static const char even_letters[] = "bdfghklmnprstwx";
static const char odd_letters[] = "aeiou";
//static const char even_letters[] = "0123456789ABCDEF";
//static const char odd_letters[] = "0123456789ABCDEF";

static void print_uid_digit(StyledStream &ss, uint64_t n, bool odd) {
	if (n != 0) {
        uint64_t base;
        const char *letters;
        if (odd) {
            // odd
            base = sizeof(odd_letters) / sizeof(char) - 1;
            letters = odd_letters;
        } else {
            // even
            base = sizeof(even_letters) / sizeof(char) - 1;
            letters = even_letters;
        }
        print_uid_digit(ss, n / base, !odd);
        char k = letters[(n % base)];
        char buf[2] = { k, '\0' };
        ss << buf;
	}
}

void stream_uid(StyledStream &ss, uint64_t uid) {
    print_uid_digit(ss, scramble(uid), false);
}

#else
// partial katakana romaji, base 65

static const char *syllables[] = {
    "ka", "ki","ku","ke","ko",//"kya","kyu","kyo",
    "sa",/*"shi",*/"su","se","so",//"sha","shu","sho",
    "ta",/*"chi","tsu",*/"te","to",//"cha","chu","cho",
    "na","ni","nu","ne","no",//"nya","nyu","nyo",
    "ha","hi","fu","he","ho",//"hya","hyu","hyo",
    "ma","mi","mu","me","mo",//"mya","myu","myo",
    "ya","yu","yo",
    "ra","ri","ru","re","ro",//"rya","ryu","ryo",
    "wa","wi","we","wo",
    "ga","gi","gu","ge","go",//"gya","gyu","gyo",
    "za","ji","zu","ze","zo","ja","ju","jo",
    "da","de","do",
    "ba","bi","bu","be","bo",//"bya","byu","byo",
    "pa","pi","pu","pe","po",//"pya","pyu","pyo",
};

static void print_uid_digit(StyledStream &ss, uint64_t n) {
	if (n != 0) {
        uint64_t base = sizeof(syllables) / sizeof(const char *);
        print_uid_digit(ss, n / base);
        ss << syllables[(n % base)];
	}
}

void stream_uid(StyledStream &ss, uint64_t uid) {
    print_uid_digit(ss, scramble(uid));
}

#endif

void stream_address(StyledStream &ss, const void *ptr) {
    uint64_t addr = (uint64_t)ptr;
    stream_uid(ss, addr);
}


} // namespace scopes

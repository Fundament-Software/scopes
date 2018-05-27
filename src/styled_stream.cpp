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

static void COLOR_RGB(std::ostream &ost, const char prefix[], int hexcode) {
    ost << prefix
        << ((hexcode >> 16) & 0xff) << ";"
        << ((hexcode >> 8) & 0xff) << ";"
        << (hexcode & 0xff) << "m";
}

static void COLOR_RGB_FG(std::ostream &ost, int hexcode) {
    return COLOR_RGB(ost, "\033[38;2;", hexcode);
}
#if 0
static void COLOR_RGB_BG(std::ostream &ost, int hexcode) {
    return COLOR_RGB(ost, "\033[48;2;", hexcode);
}
#endif

} // namespace ANSI

// support 24-bit ANSI colors (ISO-8613-3)
// works on most bash shells as well as windows 10
void ansi_from_style(std::ostream &ost, Style style) {
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

void stream_ansi_style(std::ostream &ost, Style style) {
    ansi_from_style(ost, style);
}

void stream_plain_style(std::ostream &ost, Style style) {
}

StreamStyleFunction stream_default_style = stream_plain_style;

StyledStream::StyledStream(std::ostream &ost, StreamStyleFunction ssf) :
    _ssf(ssf),
    _ost(ost)
{}

StyledStream::StyledStream(std::ostream &ost) :
    _ssf(stream_default_style),
    _ost(ost)
{}

StyledStream::StyledStream() :
    _ssf(stream_default_style),
    _ost(std::cerr)
{}

StyledStream StyledStream::plain(std::ostream &ost) {
    return StyledStream(ost, stream_plain_style);
}

StyledStream StyledStream::plain(StyledStream &ost) {
    return StyledStream(ost._ost, stream_plain_style);
}

StyledStream& StyledStream::operator<<(std::ostream &(*o)(std::ostream&)) {
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

} // namespace scopes

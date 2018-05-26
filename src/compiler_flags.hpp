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

#ifndef SCOPES_COMPILER_FLAGS_HPP
#define SCOPES_COMPILER_FLAGS_HPP

namespace scopes {

enum {
    CF_DumpDisassembly  = (1 << 0),
    CF_DumpModule       = (1 << 1),
    CF_DumpFunction     = (1 << 2),
    CF_DumpTime         = (1 << 3),
    CF_NoDebugInfo      = (1 << 4),
    CF_O1               = (1 << 5),
    CF_O2               = (1 << 6),
    CF_O3               = CF_O1 | CF_O2,
};

} // namespace scopes

#endif // SCOPES_COMPILER_FLAGS_HPP

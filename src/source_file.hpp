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

#ifndef SCOPES_SOURCE_FILE_HPP
#define SCOPES_SOURCE_FILE_HPP

#include "symbol.hpp"

#include <unordered_map>

namespace scopes {

//------------------------------------------------------------------------------
// SOURCE FILE
//------------------------------------------------------------------------------

struct SourceFile {
protected:
    static std::unordered_map<Symbol, SourceFile *, Symbol::Hash> file_cache;

    SourceFile(Symbol _path);

public:
    Symbol path;
    int fd;
    int length;
    void *ptr;
    const String *_str;

    void close();

    bool is_open();

    const char *strptr();

    static SourceFile *from_file(Symbol _path);

    static SourceFile *from_string(Symbol _path, const String *str);

    size_t size() const;

    StyledStream &stream(StyledStream &ost, int offset,
        const char *indent = "    ");
};

} // namespace scopes

#endif // SCOPES_SOURCE_FILE_HPP
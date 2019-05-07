/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
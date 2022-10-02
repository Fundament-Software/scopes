/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SOURCE_FILE_HPP
#define SCOPES_SOURCE_FILE_HPP

#include "symbol.hpp"

#include "absl/container/flat_hash_map.h"

namespace scopes {

//------------------------------------------------------------------------------
// SOURCE FILE
//------------------------------------------------------------------------------

struct SourceFile {
protected:
    SourceFile(Symbol _path);

    void close();

public:
    ~SourceFile();
    Symbol path;
    int fd;
    int length;
    void *ptr;
    const String *_str;

    bool is_open();

    const char *strptr();

    static std::unique_ptr<SourceFile> from_file(Symbol _path);

    static std::unique_ptr<SourceFile> from_string(Symbol _path, const String *str);

    size_t size() const;

    static StyledStream &stream_buffer(StyledStream &ost, int offset,
        const char *str, int length, const char *indent = "    ");
    StyledStream &stream(StyledStream &ost, int offset,
        const char *indent = "    ");
};

} // namespace scopes

#endif // SCOPES_SOURCE_FILE_HPP
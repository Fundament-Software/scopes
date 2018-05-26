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

#include "source_file.hpp"

#ifdef SCOPES_WIN32
#include "mman.h"
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <fcntl.h>
#include <assert.h>
#include <memory.h>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

//------------------------------------------------------------------------------
// SOURCE FILE
//------------------------------------------------------------------------------

SourceFile::SourceFile(Symbol _path) :
    path(_path),
    fd(-1),
    length(0),
    ptr(MAP_FAILED),
    _str(nullptr) {
}

void SourceFile::close() {
    assert(!_str);
    auto it = file_cache.find(path);
    if (it != file_cache.end()) {
        file_cache.erase(it);
    }
    if (ptr != MAP_FAILED) {
        munmap(ptr, length);
        ptr = MAP_FAILED;
        length = 0;
    }
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool SourceFile::is_open() {
    return fd != -1;
}

const char *SourceFile::strptr() {
    assert(is_open() || _str);
    return (const char *)ptr;
}

SourceFile *SourceFile::from_file(Symbol _path) {
    auto it = file_cache.find(_path);
    if (it != file_cache.end()) {
        return it->second;
    }
    SourceFile *file = new SourceFile(_path);
    file->fd = ::open(_path.name()->data, O_RDONLY);
    if (file->fd >= 0) {
        file->length = lseek(file->fd, 0, SEEK_END);
        file->ptr = mmap(nullptr,
            file->length, PROT_READ, MAP_PRIVATE, file->fd, 0);
        if (file->ptr != MAP_FAILED) {
            file_cache[_path] = file;
            return file;
        }
        file->close();
    }
    file->close();
    delete file;
    return nullptr;
}

SourceFile *SourceFile::from_string(Symbol _path, const String *str) {
    SourceFile *file = new SourceFile(_path);
    // loading from string buffer rather than file
    file->ptr = (void *)str->data;
    file->length = str->count;
    file->_str = str;
    return file;
}

size_t SourceFile::size() const {
    return length;
}

StyledStream &SourceFile::stream(StyledStream &ost, int offset,
    const char *indent) {
    auto str = strptr();
    if (offset >= length) {
        ost << "<cannot display location in source file (offset "
            << offset << " is beyond length " << length << ")>" << std::endl;
        return ost;
    }
    auto start = offset;
    auto send = offset;
    while (start > 0) {
        if (str[start-1] == '\n') {
            break;
        }
        start = start - 1;
    }
    while (start < offset) {
        if (!isspace(str[start])) {
            break;
        }
        start = start + 1;
    }
    while (send < length) {
        if (str[send] == '\n') {
            break;
        }
        send = send + 1;
    }
    auto linelen = send - start;
    char line[linelen + 1];
    memcpy(line, str + start, linelen);
    line[linelen] = 0;
    ost << indent << line << std::endl;
    auto column = offset - start;
    if (column > 0) {
        ost << indent;
        for (int i = 0; i < column; ++i) {
            ost << " ";
        }
        ost << Style_Operator << "^" << Style_None << std::endl;
    }
    return ost;
}

std::unordered_map<Symbol, SourceFile *, Symbol::Hash> SourceFile::file_cache;

} // namespace scopes

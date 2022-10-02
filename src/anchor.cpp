/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "anchor.hpp"
#include "source_file.hpp"
#include "hash.hpp"

#include "absl/container/flat_hash_set.h"

namespace scopes {

namespace AnchorSet {
struct Hash {
    std::size_t operator()(const Anchor *s) const {
        std::size_t h = s->path.hash();
        h = hash2(h, std::hash<int>{}(s->lineno));
        h = hash2(h, std::hash<int>{}(s->column));
        h = hash2(h, std::hash<int>{}(s->offset));
        h = hash2(h, std::hash<const String *>{}(s->buffer));
        return h;
    }
};

struct KeyEqual {
    bool operator()( const Anchor *lhs, const Anchor *rhs ) const {
        return lhs->is_same(rhs);
    }
};
} // namespace AnchorSet

static absl::flat_hash_set<const Anchor *, AnchorSet::Hash, AnchorSet::KeyEqual> anchors;

static const Anchor *_builtin_anchor = nullptr;
static const Anchor *_unknown_anchor = nullptr;

const Anchor *builtin_anchor() {
    if (!_builtin_anchor) {
        _builtin_anchor = Anchor::from(Symbol("builtin"), 1, 1);
    }
    return _builtin_anchor;
}

const Anchor *unknown_anchor() {
    if (!_unknown_anchor) {
        _unknown_anchor = Anchor::from(Symbol("unknown"), 1, 1);
    }
    return _unknown_anchor;
}

//------------------------------------------------------------------------------
// ANCHOR
//------------------------------------------------------------------------------

Anchor::Anchor(Symbol _path, int _lineno, int _column, int _offset, const String *_buffer) :
    path(_path),
    lineno(_lineno),
    column(_column),
    offset(_offset),
    buffer(_buffer) {}

bool Anchor::is_boring() const {
    return this == builtin_anchor() || this == unknown_anchor();
}

bool Anchor::is_same(const Anchor *other) const {
    return path == other->path
        && lineno == other->lineno
        && column == other->column
        && offset == other->offset
        && buffer == other->buffer;
}

const Anchor *Anchor::from(
    Symbol _path, int _lineno, int _column, int _offset, const String *_buffer) {
    Anchor key(_path, _lineno, _column, _offset, _buffer);
    auto it = anchors.find(&key);
    if (it != anchors.end())
        return *it;
    auto result = new Anchor(_path, _lineno, _column, _offset, _buffer);
    anchors.insert(result);
    return result;
}

const Anchor *Anchor::from(
    const std::unique_ptr<SourceFile> &file, int _lineno, int _column, int _offset) {
    if (file->_str) {
        return Anchor::from(file->path, _lineno, _column, _offset, file->_str);
    } else {
        return Anchor::from(file->path, _lineno, _column, _offset);
    }
}

StyledStream& Anchor::stream(StyledStream& ost) const {
    ost << Style_Location;
    auto ss = StyledStream::plain(ost);
    ss << path.name()->data << ":" << lineno << ":" << column << ":";
    ost << Style_None;
    return ost;
}

StyledStream &Anchor::stream_source_line(StyledStream &ost, const char *indent) const {
    if (buffer) {
        SourceFile::stream_buffer(ost, offset, buffer->data, buffer->count);
    } else {
        auto file = SourceFile::from_file(path);
        if (file) {
            file->stream(ost, offset, indent);
        }
    }
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, const Anchor *anchor) {
    return anchor->stream(ost);
}

} // namespace scopes

/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "anchor.hpp"
#include "source_file.hpp"
#include "hash.hpp"

#include <unordered_set>

namespace scopes {

namespace AnchorSet {
struct Hash {
    std::size_t operator()(const Anchor *s) const {
        std::size_t h = std::hash<SourceFile *>{}(s->file);
        h = hash2(h, std::hash<int>{}(s->lineno));
        h = hash2(h, std::hash<int>{}(s->column));
        return h;
    }
};

struct KeyEqual {
    bool operator()( const Anchor *lhs, const Anchor *rhs ) const {
        return lhs->is_same(rhs);
    }
};
} // namespace AnchorSet

static std::unordered_set<const Anchor *, AnchorSet::Hash, AnchorSet::KeyEqual> anchors;

static const Anchor *_builtin_anchor = nullptr;
static const Anchor *_unknown_anchor = nullptr;

const Anchor *builtin_anchor() {
    if (!_builtin_anchor) {
        auto stub_file = SourceFile::from_string(Symbol("builtin"), String::from_cstr(""));
        _builtin_anchor = Anchor::from(stub_file, 1, 1);
    }
    return _builtin_anchor;
}

const Anchor *unknown_anchor() {
    if (!_unknown_anchor) {
        auto stub_file = SourceFile::from_string(Symbol("unknown"), String::from_cstr(""));
        _unknown_anchor = Anchor::from(stub_file, 1, 1);
    }
    return _unknown_anchor;
}

//------------------------------------------------------------------------------
// ANCHOR
//------------------------------------------------------------------------------

Anchor::Anchor(SourceFile *_file, int _lineno, int _column, int _offset) :
    file(_file),
    lineno(_lineno),
    column(_column),
    offset(_offset) {}

Symbol Anchor::path() const {
    return file->path;
}

bool Anchor::is_same(const Anchor *other) const {
    return file->path == other->file->path
        && lineno == other->lineno
        && column == other->column;
}

const Anchor *Anchor::from(
    SourceFile *_file, int _lineno, int _column, int _offset) {
    Anchor key(_file, _lineno, _column, _offset);
    auto it = anchors.find(&key);
    if (it != anchors.end())
        return *it;
    auto result = new Anchor(_file, _lineno, _column, _offset);
    anchors.insert(result);
    return result;
}

StyledStream& Anchor::stream(StyledStream& ost) const {
    ost << Style_Location;
    auto ss = StyledStream::plain(ost);
    ss << path().name()->data << ":" << lineno << ":" << column << ":";
    ost << Style_None;
    return ost;
}

StyledStream &Anchor::stream_source_line(StyledStream &ost, const char *indent) const {
    file->stream(ost, offset, indent);
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, const Anchor *anchor) {
    return anchor->stream(ost);
}

} // namespace scopes

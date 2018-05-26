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

#include "integer.hpp"
#include "utils.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// INTEGER TYPE
//------------------------------------------------------------------------------

bool IntegerType::classof(const Type *T) {
    return T->kind() == TK_Integer;
}

IntegerType::IntegerType(size_t _width, bool _issigned)
    : Type(TK_Integer), width(_width), issigned(_issigned) {
    std::stringstream ss;
    if ((_width == 1) && !_issigned) {
        ss << "bool";
    } else {
        if (issigned) {
            ss << "i";
        } else {
            ss << "u";
        }
        ss << width;
    }
    _name = String::from_stdstring(ss.str());
}

static const Type *_Integer(size_t _width, bool _issigned) {
    return new IntegerType(_width, _issigned);
}
static auto m_Integer = memoize(_Integer);

const Type *Integer(size_t _width, bool _issigned) {
    return m_Integer(_width, _issigned);
}

} // namespace scopes


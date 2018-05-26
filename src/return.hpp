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

#ifndef SCOPES_RETURN_HPP
#define SCOPES_RETURN_HPP

#include "type.hpp"
#include "argument.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// RETURN LABEL TYPE
//------------------------------------------------------------------------------

enum ReturnLabelMode {
    RLM_Return,
    RLM_NoReturn,
};

struct ReturnLabelType : Type {
    static bool classof(const Type *T);

    bool is_returning() const;

    const Type *to_unconst() const;

    ReturnLabelType(ReturnLabelMode _mode, const Args &_values);

    bool has_constants() const;

    bool has_variables() const;

    bool has_multiple_return_values() const;

    Args values;
    const Type *return_type;
    ReturnLabelMode mode;
protected:
    bool has_mrv;
    bool has_const;
    bool has_vars;
};

const Type *ReturnLabel(ReturnLabelMode mode, const Args &values);

const Type *ReturnLabel(const Args &values);

const Type *NoReturnLabel();

} // namespace scopes

#endif // SCOPES_RETURN_HPP
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

#ifndef SCOPES_STREAM_LABEL_HPP
#define SCOPES_STREAM_LABEL_HPP

#include "stream_anchors.hpp"
#include "argument.hpp"

#include <unordered_set>

namespace scopes {

struct Label;
struct Parameter;

//------------------------------------------------------------------------------
// IL PRINTER
//------------------------------------------------------------------------------

struct StreamLabelFormat {
    enum Tagging {
        All,
        Line,
        Scope,
        None,
    };

    Tagging anchors;
    Tagging follow;
    bool show_users;
    bool show_scope;

    StreamLabelFormat();

    static StreamLabelFormat debug_all();

    static StreamLabelFormat debug_scope();

    static StreamLabelFormat debug_single();

    static StreamLabelFormat single();

    static StreamLabelFormat scope();
};

struct StreamLabel : StreamAnchors {
    StreamLabelFormat fmt;
    bool line_anchors;
    bool atom_anchors;
    bool follow_labels;
    bool follow_scope;
    std::unordered_set<Label *> visited;

    StreamLabel(StyledStream &_ss, const StreamLabelFormat &_fmt);

    void stream_label_label(Label *alabel);

    void stream_label_label_user(Label *alabel);

    void stream_param_label(Parameter *param, Label *alabel);

    void stream_argument(Argument arg, Label *alabel);

    void stream_label (Label *alabel);

    void stream_any(const Any &afunc);

    void stream(Label *label);

};

void stream_label(
    StyledStream &_ss, Label *label, const StreamLabelFormat &_fmt);

} // namespace scopes

#endif // SCOPES_STREAM_LABEL_HPP
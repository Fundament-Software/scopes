/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
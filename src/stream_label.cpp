/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_label.hpp"
#include "label.hpp"
#include "type.hpp"
#include "stream_expr.hpp"

namespace scopes {

//------------------------------------------------------------------------------

StreamLabelFormat::StreamLabelFormat() :
    anchors(None),
    follow(All),
    show_users(false),
    show_scope(false)
    {}

StreamLabelFormat StreamLabelFormat::debug_all() {
    StreamLabelFormat fmt;
    fmt.follow = All;
    fmt.show_users = true;
    fmt.show_scope = true;
    return fmt;
}

StreamLabelFormat StreamLabelFormat::debug_scope() {
    StreamLabelFormat fmt;
    fmt.follow = Scope;
    fmt.show_users = true;
    return fmt;
}

StreamLabelFormat StreamLabelFormat::debug_single() {
    StreamLabelFormat fmt;
    fmt.follow = None;
    fmt.show_users = true;
    fmt.anchors = Line;
    return fmt;
}

StreamLabelFormat StreamLabelFormat::single() {
    StreamLabelFormat fmt;
    fmt.follow = None;
    return fmt;
}

StreamLabelFormat StreamLabelFormat::scope() {
    StreamLabelFormat fmt;
    fmt.follow = Scope;
    return fmt;
}

//------------------------------------------------------------------------------

StreamLabel::StreamLabel(StyledStream &_ss, const StreamLabelFormat &_fmt) :
    StreamAnchors(_ss), fmt(_fmt) {
    line_anchors = (fmt.anchors == StreamLabelFormat::Line);
    atom_anchors = (fmt.anchors == StreamLabelFormat::All);
    follow_labels = (fmt.follow == StreamLabelFormat::All);
    follow_scope = (fmt.follow == StreamLabelFormat::Scope);
}

void StreamLabel::stream_label_label(Label *alabel) {
    alabel->stream_short(ss);
}

void StreamLabel::stream_label_label_user(Label *alabel) {
    alabel->stream_short(ss);
}

void StreamLabel::stream_param_label(Parameter *param, Label *alabel) {
    if (param->label == alabel) {
        param->stream_local(ss);
    } else {
        param->stream(ss);
    }
}

void StreamLabel::stream_argument(Argument arg, Label *alabel) {
    if (arg.key != SYM_Unnamed) {
        ss << arg.key << Style_Operator << "=" << Style_None;
    }
    if (arg.value.type == TYPE_Parameter) {
        stream_param_label(arg.value.parameter, alabel);
    } else if (arg.value.type == TYPE_Label) {
        stream_label_label(arg.value.label);
    } else if (arg.value.type == TYPE_List) {
        stream_expr(ss, arg.value, StreamExprFormat::singleline_digest());
    } else {
        ss << arg.value;
    }
}

void StreamLabel::stream_label (Label *alabel) {
    if (visited.count(alabel)) {
        return;
    }
    visited.insert(alabel);
    if (line_anchors) {
        stream_anchor(alabel->anchor);
    }
    if (alabel->is_inline()) {
        ss << Style_Keyword << "inline" << Style_None;
        ss << " ";
    }
    if (alabel->is_reentrant()) {
        ss << Style_Keyword << "reentrant" << Style_None;
        ss << " ";
    }
    if (alabel->is_merge()) {
        ss << Style_Keyword << "merge" << Style_None;
        ss << " ";
    }
    alabel->stream(ss, fmt.show_users);
    ss << Style_Operator << ":" << Style_None;
    if (fmt.show_scope && alabel->body.scope_label) {
        ss << " " << Style_Operator << "[" << Style_None;
        alabel->body.scope_label->stream_short(ss);
        ss << Style_Operator << "]" << Style_None;
    }
    //stream_scope(scopes[alabel])
    ss << std::endl;
    ss << "    ";
    if (line_anchors && alabel->body.anchor) {
        stream_anchor(alabel->body.anchor);
        ss << " ";
    }
    if (!alabel->body.is_complete()) {
        ss << Style_Keyword << "T " << Style_None;
    }
    if (alabel->body.is_rawcall()) {
        ss << Style_Keyword << "rawcall " << Style_None;
    }
    if (alabel->body.is_trycall()) {
        ss << Style_Keyword << "trycall " << Style_None;
    }
    stream_argument(alabel->body.enter, alabel);
    for (size_t i=1; i < alabel->body.args.size(); ++i) {
        ss << " ";
        stream_argument(alabel->body.args[i], alabel);
    }
    if (!alabel->body.args.empty()) {
        auto &&cont = alabel->body.args[0];
        if (cont.value.type != TYPE_Nothing) {
            ss << " " << Style_Comment << CONT_SEP << Style_None << " ";
            stream_argument(cont.value, alabel);
        }
    }
    ss << std::endl;

    if (follow_labels) {
        for (size_t i=0; i < alabel->body.args.size(); ++i) {
            stream_any(alabel->body.args[i].value);
        }
        stream_any(alabel->body.enter);
    }
}

void StreamLabel::stream_any(const Any &afunc) {
    if (afunc.type == TYPE_Label) {
        stream_label(afunc.label);
    }
}

void StreamLabel::stream(Label *label) {
    stream_label(label);
    if (follow_scope) {
        Labels scope;
        label->build_scope(scope);
        size_t i = scope.size();
        while (i > 0) {
            --i;
            stream_label(scope[i]);
        }
    }
}

//------------------------------------------------------------------------------

void stream_label(
    StyledStream &_ss, Label *label, const StreamLabelFormat &_fmt) {
    StreamLabel streamer(_ss, _fmt);
    streamer.stream(label);
}

} // namespace scopes

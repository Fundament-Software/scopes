/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "label.hpp"
#include "type.hpp"
#include "error.hpp"
#include "return.hpp"
#include "function.hpp"
#include "stream_label.hpp"
#include "dyn_cast.inc"

#include <assert.h>

namespace scopes {

template<typename T>
uint64_t Tag<T>::active_gen = 0;

//------------------------------------------------------------------------------

Label::Label(const Anchor *_anchor, Symbol _name, uint64_t _flags) :
    original(nullptr), docstring(nullptr),
    uid(0), next_instanceid(1), anchor(_anchor), name(_name),
    paired(nullptr), flags(_flags)
    {}

void Label::set_reentrant() {
    flags |= LF_Reentrant;
}

bool Label::is_reentrant() const {
    return flags & LF_Reentrant;
}

bool Label::is_debug() const {
    return flags & LF_Debug;
}

void Label::set_debug() {
    flags |= LF_Debug;
}

void Label::set_merge() {
    flags |= LF_Merge;
}

void Label::unset_merge() {
    flags &= ~LF_Merge;
}

void Label::set_inline() {
    flags |= LF_Inline;
}

void Label::unset_inline() {
    flags &= ~LF_Inline;
}

bool Label::is_merge() const {
    return flags & LF_Merge;
}

bool Label::is_inline() const {
    return flags & LF_Inline;
}

bool Label::is_important() const {
    return flags & (LF_Merge | LF_Reentrant);
}

bool Label::is_template() const {
    return flags & LF_Template;
}

Parameter *Label::get_param_by_name(Symbol name) {
    size_t count = params.size();
    for (size_t i = 1; i < count; ++i) {
        if (params[i]->name == name) {
            return params[i];
        }
    }
    return nullptr;
}

bool Label::is_jumping() const {
    auto &&args = body.args;
    assert(!args.empty());
    return args[0].value.type == TYPE_Nothing;
}

bool Label::is_calling(Label *callee) const {
    auto &&enter = body.enter;
    return (enter.type == TYPE_Label) && (enter.label == callee);
}

bool Label::is_continuing_to(Label *callee) const {
    auto &&args = body.args;
    assert(!args.empty());
    return (args[0].value.type == TYPE_Label) && (args[0].value.label == callee);
}

bool Label::is_basic_block_like() const {
    if (params.empty())
        return true;
    if (params[0]->type == TYPE_Nothing)
        return true;
    return false;
}

bool Label::is_return_param_typed() const {
    assert(!params.empty());
    return params[0]->is_typed();
}

bool Label::has_params() const {
    return params.size() > 1;
}

bool Label::is_variadic() const {
    return (!params.empty() && params.back()->is_vararg());
}

bool Label::is_valid() const {
    return !params.empty() && body.anchor && !body.args.empty();
}

void Label::verify_valid () {
    const String *msg = nullptr;
    if (params.empty()) {
        msg = String::from("label corrupt: parameters are missing");
    } else if (!body.anchor) {
        msg = String::from("label corrupt: body anchor is missing");
    } else if (body.args.empty()) {
        msg = String::from("label corrupt: body arguments are missing");
    }
    if (msg) {
        set_active_anchor(anchor);
        location_error(msg);
    }
}

//------------------------------------------------------------------------------

void Label::UserMap::clear() {
    label_map.clear();
    param_map.clear();
}

void Label::UserMap::insert(Label *source, Label *dest) {
    label_map[dest].insert(source);
}

void Label::UserMap::insert(Label *source, Parameter *dest) {
    param_map[dest].insert(source);
}

void Label::UserMap::remove(Label *source, Label *dest) {
    auto it = label_map.find(dest);
    if (it != label_map.end()) {
        it->second.erase(source);
    }
}

void Label::UserMap::remove(Label *source, Parameter *dest) {
    auto it = param_map.find(dest);
    if (it != param_map.end()) {
        it->second.erase(source);
    }
}

void Label::UserMap::stream_users(const std::unordered_set<Label *> &users,
    StyledStream &ss) const {
    ss << Style_Comment << "{" << Style_None;
    size_t i = 0;
    for (auto &&kv : users) {
        if (i > 0) {
            ss << " ";
        }
        Label *label = kv;
        label->stream_short(ss);
        i++;
    }
    ss << Style_Comment << "}" << Style_None;
}

void Label::UserMap::stream_users(Label *node, StyledStream &ss) const {
    auto it = label_map.find(node);
    if (it != label_map.end()) stream_users(it->second, ss);
}

void Label::UserMap::stream_users(Parameter *node, StyledStream &ss) const {
    auto it = param_map.find(node);
    if (it != param_map.end()) stream_users(it->second, ss);
}

//------------------------------------------------------------------------------

Label *Label::get_label_enter() const {
    assert(body.enter.type == TYPE_Label);
    return body.enter.label;
}

const Closure *Label::get_closure_enter() const {
    assert(body.enter.type == TYPE_Closure);
    return body.enter.closure;
}

Builtin Label::get_builtin_enter() const {
    assert(body.enter.type == TYPE_Builtin);
    return body.enter.builtin;
}

Label *Label::get_label_cont() const {
    assert(!body.args.empty());
    assert(body.args[0].value.type == TYPE_Label);
    return body.args[0].value.label;
}

const Type *Label::get_return_type() const {
    assert(params.size());
    assert(!is_basic_block_like());
    if (!params[0]->is_typed())
        return TYPE_Void;
    // verify that the return type is the one we expect
    cast<ReturnLabelType>(params[0]->type);
    return params[0]->type;
}

void Label::verify_compilable() const {
    if (params[0]->is_typed()
        && !params[0]->is_none()) {
        auto tl = dyn_cast<ReturnLabelType>(params[0]->type);
        if (!tl) {
            set_active_anchor(anchor);
            StyledString ss;
            ss.out << "cannot compile function with return type "
                << params[0]->type;
            location_error(ss.str());
        }
        for (size_t i = 0; i < tl->values.size(); ++i) {
            auto &&val = tl->values[i].value;
            if (is_unknown(val)) {
                auto T = val.typeref;
                if (is_opaque(T)) {
                    set_active_anchor(anchor);
                    StyledString ss;
                    ss.out << "cannot compile function with opaque return argument of type "
                        << T;
                    location_error(ss.str());
                }
            }
        }
    }

    ArgTypes argtypes;
    for (size_t i = 1; i < params.size(); ++i) {
        auto T = params[i]->type;
        if (T == TYPE_Unknown) {
            set_active_anchor(anchor);
            location_error(String::from("cannot compile function with untyped argument"));
        } else if (is_invalid_argument_type(T)) {
            set_active_anchor(anchor);
            StyledString ss;
            ss.out << "cannot compile function with opaque argument of type "
                << T;
            location_error(ss.str());
        }
    }
}

const Type *Label::get_params_as_return_label_type() const {
    scopes::Args values;
    for (size_t i = 1; i < params.size(); ++i) {
        values.push_back(unknown_of(params[i]->type));
    }
    return ReturnLabel(values);
}

const Type *Label::get_function_type() const {

    ArgTypes argtypes;
    for (size_t i = 1; i < params.size(); ++i) {
        argtypes.push_back(params[i]->type);
    }
    uint64_t flags = 0;
    assert(params.size());
    if (!params[0]->is_typed()) {
        flags |= FF_Divergent;
    }
    return Function(get_return_type(), argtypes, flags);
}

void Label::use(UserMap &um, const Any &arg, int i) {
    if (arg.type == TYPE_Parameter && (arg.parameter->label != this)) {
        um.insert(this, arg.parameter /*, i*/);
    } else if (arg.type == TYPE_Label && (arg.label != this)) {
        um.insert(this, arg.label /*, i*/);
    }
}

void Label::unuse(UserMap &um, const Any &arg, int i) {
    if (arg.type == TYPE_Parameter && (arg.parameter->label != this)) {
        um.remove(this, arg.parameter /*, i*/);
    } else if (arg.type == TYPE_Label && (arg.label != this)) {
        um.remove(this, arg.label /*, i*/);
    }
}

void Label::insert_into_usermap(UserMap &um) {
    use(um, body.enter, -1);
    size_t count = body.args.size();
    for (size_t i = 0; i < count; ++i) {
        use(um, body.args[i].value, i);
    }
}

void Label::remove_from_usermap(UserMap &um) {
    unuse(um, body.enter, -1);
    size_t count = body.args.size();
    for (size_t i = 0; i < count; ++i) {
        unuse(um, body.args[i].value, i);
    }
}

void Label::append(Parameter *param) {
    assert(!param->label);
    param->label = this;
    param->index = (int)params.size();
    params.push_back(param);
}

void Label::set_parameters(const Parameters &_params) {
    assert(params.empty());
    params = _params;
    for (size_t i = 0; i < params.size(); ++i) {
        Parameter *param = params[i];
        assert(!param->label);
        param->label = this;
        param->index = (int)i;
    }
}

void Label::build_reachable(std::unordered_set<Label *> &labels,
    Labels *ordered_labels) {
    labels.clear();
    labels.insert(this);
    if (ordered_labels)
        ordered_labels->push_back(this);
    Labels foreign_stack = { this };
    Labels stack = {};
    while (true) {
        Label *parent = nullptr;
        if (!stack.empty()) {
            parent = stack.back();
            stack.pop_back();
        } else if (!foreign_stack.empty()) {
            parent = foreign_stack.back();
            foreign_stack.pop_back();
            if (ordered_labels)
                ordered_labels->push_back(parent);
        } else {
                break;
        }

        int size = (int)parent->body.args.size();
        for (int i = -1; i < size; ++i) {
            Any arg = none;
            if (i == -1) {
                arg = parent->body.enter;
            } else {
                arg = parent->body.args[i].value;
            }

            if (arg.type == TYPE_Label) {
                Label *label = arg.label;
                if (!labels.count(label)) {
                    labels.insert(label);
                    if (label->is_basic_block_like()) {
                        if (ordered_labels)
                            ordered_labels->push_back(label);
                        stack.push_back(label);
                    } else {
                        foreign_stack.push_back(label);
                    }
                }
            }
        }
    }
}

void Label::build_scope(UserMap &um, Labels &tempscope) {
    tempscope.clear();

    std::unordered_set<Label *> visited;
    visited.clear();
    visited.insert(this);

    for (auto &&param : params) {
        auto it = um.param_map.find(param);
        if (it != um.param_map.end()) {
            auto &&users = it->second;
            // every label using one of our parameters is live in scope
            for (auto &&kv : users) {
                Label *live_label = kv;
                if (!visited.count(live_label)) {
                    visited.insert(live_label);
                    tempscope.push_back(live_label);
                }
            }
        }
    }

    size_t index = 0;
    while (index < tempscope.size()) {
        Label *scope_label = tempscope[index++];

        auto it = um.label_map.find(scope_label);
        if (it != um.label_map.end()) {
            auto &&users = it->second;
            // users of scope_label are indirectly live in scope
            for (auto &&kv : users) {
                Label *live_label = kv;
                if (!visited.count(live_label)) {
                    visited.insert(live_label);
                    tempscope.push_back(live_label);
                }
            }
        }

        for (auto &&param : scope_label->params) {
            auto it = um.param_map.find(param);
            if (it != um.param_map.end()) {
                auto &&users = it->second;
                // every label using scope_label's parameters is live in scope
                for (auto &&kv : users) {
                    Label *live_label = kv;
                    if (!visited.count(live_label)) {
                        visited.insert(live_label);
                        tempscope.push_back(live_label);
                    }
                }
            }
        }
    }
}

void Label::build_scope(Labels &tempscope) {
    std::unordered_set<Label *> visited;
    Labels reachable;
    build_reachable(visited, &reachable);
    UserMap um;
    for (auto it = reachable.begin(); it != reachable.end(); ++it) {
        (*it)->insert_into_usermap(um);
    }

    build_scope(um, tempscope);
}

Label *Label::get_original() {
    Label *l = this;
    while (l->original)
        l = l->original;
    return l;
}

StyledStream &Label::stream_id(StyledStream &ss) const {
    if (original) {
        original->stream_id(ss);
    }
    ss << uid;
    if (uid >= 10) {
        ss << "x";
    }
    return ss;
}

StyledStream &Label::stream_short(StyledStream &ss) const {
#if SCOPES_DEBUG_CODEGEN
    if (is_template()) {
        ss << Style_Keyword << "T:" << Style_None;
    }
#endif
    if (name != SYM_Unnamed) {
        ss << Style_Symbol;
        name.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
    }
    ss << Style_Keyword << "λ" << Style_Symbol;
    {
        StyledStream ps = StyledStream::plain(ss);
        if (!original) {
            ps << uid;
        } else {
            stream_id(ps);
        }
    }
    ss << Style_None;
    return ss;
}

StyledStream &Label::stream(StyledStream &ss, bool users) const {
    stream_short(ss);
    ss << Style_Operator << "(" << Style_None;
    size_t count = params.size();
    for (size_t i = 1; i < count; ++i) {
        if (i > 1) {
            ss << " ";
        }
        params[i]->stream_local(ss);
    }
    ss << Style_Operator << ")" << Style_None;
    if (count) {
        const Type *rtype = params[0]->type;
        if (rtype != TYPE_Nothing) {
            ss << Style_Comment << CONT_SEP << Style_None;
            if (rtype == TYPE_Unknown) {
                ss << Style_Comment << "?" << Style_None;
            } else {
                params[0]->stream_local(ss);
            }
        }
    }
    return ss;
}

const ReturnLabelType *Label::verify_return_label() {
    if (!params.empty()) {
        const ReturnLabelType *rt = dyn_cast<ReturnLabelType>(params[0]->type);
        if (rt)
            return rt;
    }
#if SCOPES_DEBUG_CODEGEN
    {
        StyledStream ss;
        stream_label(ss, this, StreamLabelFormat::debug_all());
    }
#endif
    set_active_anchor(anchor);
    location_error(String::from("label is not a function"));
    return nullptr;
}

Label *Label::from(const Anchor *_anchor, Symbol _name) {
    assert(_anchor);
    Label *result = new Label(_anchor, _name, LF_Template);
    result->uid = next_uid++;
    return result;
}
// only inherits name and anchor
Label *Label::from(Label *label) {
    Label *result = new Label(label->anchor, label->name, 0);
    result->original = label;
    result->uid = label->next_instanceid++;
    result->flags |= label->flags & (LF_Merge | LF_Debug);
    return result;
}

// a continuation that never returns
Label *Label::continuation_from(const Anchor *_anchor, Symbol _name) {
    Label *value = from(_anchor, _name);
    // first argument is present, but unused
    value->append(Parameter::from(_anchor, _name, TYPE_Nothing));
    return value;
}

// an inline function that eventually returns
Label *Label::inline_from(const Anchor *_anchor, Symbol _name) {
    Label *value = from(_anchor, _name);
    // continuation is always first argument
    // this argument will be eventually inlined
    value->append(
        Parameter::from(_anchor,
            Symbol(SYM_Unnamed),
            TYPE_Unknown));
    value->set_inline();
    return value;
}

// a function that eventually returns
Label *Label::function_from(const Anchor *_anchor, Symbol _name) {
    Label *value = from(_anchor, _name);
    // continuation is always first argument
    // this argument will be called when the function is done
    value->append(
        Parameter::from(_anchor,
            Symbol(SYM_Unnamed),
            TYPE_Unknown));
    return value;
}

//------------------------------------------------------------------------------

uint64_t Label::next_uid = 0;

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ss, Label *label) {
    label->stream(ss);
    return ss;
}

StyledStream& operator<<(StyledStream& ss, const Label *label) {
    label->stream(ss);
    return ss;
}

} // namespace scopes

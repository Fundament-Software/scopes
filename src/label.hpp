/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LABEL_HPP
#define SCOPES_LABEL_HPP

#include <stdint.h>
#include "parameter.hpp"
#include "body.hpp"

#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace scopes {

const char CONT_SEP[] = "▶";

struct Label;
struct Anchor;
struct ReturnLabelType;

typedef std::vector<Label *> Labels;
typedef std::deque<Label *> LabelQueue;

template<typename T>
struct Tag {
    static uint64_t active_gen;
    uint64_t gen;

    Tag() :
        gen(active_gen) {}

    static void clear() {
        active_gen++;
    }
    bool visited() const {
        return gen == active_gen;
    }
    void visit() {
        gen = active_gen;
    }
};

typedef Tag<Label> LabelTag;

enum LabelFlags {
    LF_Template = (1 << 0),
    // label is reentrant
    LF_Reentrant = (1 << 1),
    // this label is an inline
    LF_Inline = (1 << 2),
    // this label can not be instantiated more than once per frame and forces
    // all callers to use the same arguments
    LF_Merge = (1 << 3),
    // when this label is processed, output some information to screen
    LF_Debug = (1 << 4),
};

// IL form inspired by
// Leissa et al., Graph-Based Higher-Order Intermediate Representation
// http://compilers.cs.uni-saarland.de/papers/lkh15_cgo.pdf

struct Label {
protected:
    static uint64_t next_uid;

    Label(const Anchor *_anchor, Symbol _name, uint64_t _flags);

public:
    Label *original;
    const String *docstring;
    size_t uid;
    size_t next_instanceid;
    const Anchor *anchor;
    Symbol name;
    Parameters params;
    Body body;
    LabelTag tag;
    Label *paired;
    uint64_t flags;

    void set_reentrant();

    bool is_reentrant() const;

    bool is_debug() const;

    void set_debug();

    void set_merge();

    void unset_merge();

    void set_inline();

    void unset_inline();

    bool is_merge() const;

    bool is_inline() const;

    bool is_important() const;

    bool is_template() const;

    Parameter *get_param_by_name(Symbol name);

    bool is_jumping() const;

    bool is_calling(Label *callee) const;

    bool is_continuing_to(Label *callee) const;

    bool is_basic_block_like() const;

    bool is_return_param_typed() const;

    bool has_params() const;

    bool is_variadic() const;

    bool is_valid() const;

    void verify_valid ();

    struct UserMap {
        std::unordered_map<Label *, std::unordered_set<Label *> > label_map;
        std::unordered_map<Parameter *, std::unordered_set<Label *> > param_map;

        void clear();

        void insert(Label *source, Label *dest);

        void insert(Label *source, Parameter *dest);

        void remove(Label *source, Label *dest);

        void remove(Label *source, Parameter *dest);

        void stream_users(const std::unordered_set<Label *> &users,
            StyledStream &ss) const;

        void stream_users(Label *node, StyledStream &ss) const;

        void stream_users(Parameter *node, StyledStream &ss) const;
    };

    Label *get_label_enter() const;

    const Closure *get_closure_enter() const;

    Builtin get_builtin_enter() const;

    Label *get_label_cont() const;

    const ReturnLabelType *verify_return_label();

    const Type *get_return_type() const;

    void verify_compilable() const;

    const Type *get_params_as_return_label_type() const;

    const Type *get_function_type() const;

    void use(UserMap &um, const Any &arg, int i);

    void unuse(UserMap &um, const Any &arg, int i);

    void insert_into_usermap(UserMap &um);

    void remove_from_usermap(UserMap &um);

    void append(Parameter *param);

    void set_parameters(const Parameters &_params);

    void build_reachable(std::unordered_set<Label *> &labels,
        Labels *ordered_labels = nullptr);

    void build_scope(UserMap &um, Labels &tempscope);

    void build_scope(Labels &tempscope);

    Label *get_original();

    StyledStream &stream_id(StyledStream &ss) const;

    StyledStream &stream_short(StyledStream &ss) const;

    StyledStream &stream(StyledStream &ss, bool users = false) const;

    static Label *from(const Anchor *_anchor, Symbol _name);
    // only inherits name and anchor
    static Label *from(Label *label);

    // a continuation that never returns
    static Label *continuation_from(const Anchor *_anchor, Symbol _name);

    // an inline function that eventually returns
    static Label *inline_from(const Anchor *_anchor, Symbol _name);

    // a function that eventually returns
    static Label *function_from(const Anchor *_anchor, Symbol _name);

};

StyledStream& operator<<(StyledStream& ss, Label *label);

StyledStream& operator<<(StyledStream& ss, const Label *label);

} // namespace scopes

#endif // SCOPES_LABEL_HPP
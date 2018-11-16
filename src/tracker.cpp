/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "tracker.hpp"
#include "value.hpp"
#include "error.hpp"
#include "types.hpp"
#include "hash.hpp"
#include "builtin.hpp"
#include "dyn_cast.inc"
#include "stream_ast.hpp"
#include "prover.hpp"
#include "qualifiers.hpp"

#include <unordered_map>

// do not produce errors, just annotate
#define SCOPES_SOFT_TRACKING 1

namespace scopes {

/*
for more info on borrow inference, see
https://gist.github.com/paniq/71251083aa52c1577f2d1b22be0ac6e1

*/

//------------------------------------------------------------------------------

static int track_count = 0;

struct Tracker {
    enum VisitMode {
        // inject destructor when last reference
        VM_AUTO,
        // always borrow
        VM_FORCE_BORROW,
        // force move, no destructor injection
        VM_FORCE_MOVE,
    };

    struct Data {
        Data() :
            move_depth(-1), mover(nullptr), last_user(nullptr)
        {}

        bool will_be_used() const {
            return last_user != nullptr;
        }

        bool will_be_moved() const {
            return mover != nullptr;
        }

        void move(Value *_mover, int depth) {
            assert(!mover);
            assert(_mover);
            assert(depth >= 0);
            mover = _mover;
            move_depth = depth;
        }
        Value *get_mover() const { return mover; }
        int get_move_depth() const { return move_depth; }

        void use(Value *_user) {
            assert(_user);
            last_user = _user;
        }
        Value *get_user() const { return last_user; }

    protected:
        int move_depth;
        Value *mover;
        Value *last_user;
    };

    // per block
    struct State {
        State(Block &_block)
            : block(_block), parent(nullptr), where(nullptr), finalized(false)
        {
        }

        State(Block &_block, State &_parent)
            : block(_block), parent(&_parent), where(_parent.where)
        {
            for (auto &&key : _parent._data) {
                _data.insert({key.first, key.second});
            }
        }

        void finalize() {
            block.insert_at_end();
            finalized = true;
        }

        void annotate(const String *str) {
            assert(str);
            if (finalized) {
                block.annotate(str);
            } else {
                assert(where);
                where->annotate(str);
            }
        }

        void set_location(Value *_where) {
            assert(_where);
            where = _where;
        }

        Data *find_data(const ValueIndex &value) {
            auto it = _data.find(value);
            if (it != _data.end()) {
                Data &data = it->second;
                return &data;
            }
            return nullptr;
        }

        void delete_data(const ValueIndex &value) {
        #if SCOPES_SOFT_TRACKING
            StyledString ss;
            ss.out << "no longer tracking " << value;
            annotate(ss.str());
        #endif
            auto it = _data.find(value);
            if (it != _data.end()) {
                _data.erase(it);
            }
            _deleted.insert(value);
        }

        Data &ensure_data(const ValueIndex &value) {
        #if SCOPES_SOFT_TRACKING
            if (_deleted.count(value)) {
                StyledString ss;
                ss.out << Style_Error << "error: " << Style_None
                    << "referencing " << value << " before it is created";
                annotate(ss.str());
            }
        #else
            assert(!_deleted.count(value));
        #endif
            auto data = find_data(value);
            if (data)
                return *data;
            auto result = _data.insert({value, Data()});
            return result.first->second;
        }

        Block &block;
        State *parent;
        Value *where;
        bool finalized;
        std::unordered_map<ValueIndex, Data, ValueIndex::Hash> _data;
        std::unordered_set<ValueIndex, ValueIndex::Hash> _deleted;
    };

    Tracker(ASTContext &_ctx)
        : ctx(_ctx), function(_ctx.function)
    {}

    SCOPES_RESULT(void) track_Parameter(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Template(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Keyed(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Expression(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Quote(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Unquote(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_CompileStage(State &state, Value *node) {
        assert(false);
        return {};
    }

    void collect_local_keys(ValueIndexSet &set, State &from, int retdepth) {
        for (auto &entries : from._data) {
            const ValueIndex &key = entries.first;
            Data &data = entries.second;
            int depth = key.value->get_depth();
            if (data.will_be_moved()
                && (data.get_move_depth() > retdepth)) {
                set.insert(key);
            }
        }
    }

    SCOPES_RESULT(void) merge_state(State &state, State &sub, const char *context) {
        SCOPES_RESULT_TYPE(void);
        assert(&state != &sub);
        int retdepth = state.block.depth;
        assert(retdepth >= 0);
        for (auto &entries : sub._data) {
            const ValueIndex &key = entries.first;
            Data &data = entries.second;
            int depth = key.value->get_depth();
            if (data.will_be_moved()
                && (data.get_move_depth() > retdepth)) {
                SCOPES_CHECK_RESULT(move_argument(state, key, context));
            }
        }
        return {};
    }

    SCOPES_RESULT(void) merge_states(State &state, State &_then, State &_else) {
        SCOPES_RESULT_TYPE(void);
        assert(&state != &_then);
        assert(&_then != &_else);
        int retdepth = state.block.depth;
        assert(retdepth >= 0);
        ValueIndexSet then_moved;
        ValueIndexSet else_moved;
        collect_local_keys(then_moved, _then, retdepth);
        collect_local_keys(else_moved, _else, retdepth);
        _else.finalize();
        _then.finalize();
        for (auto key : then_moved) {
            if (else_moved.count(key))
                continue;
            SCOPES_CHECK_RESULT(move_argument(_else, key, "then-branch"));
            SCOPES_CHECK_RESULT(write_destructor(_else, key, "then-branch"));
        }
        for (auto key : else_moved) {
            if (then_moved.count(key))
                continue;
            SCOPES_CHECK_RESULT(move_argument(_then, key, "else-branch"));
            SCOPES_CHECK_RESULT(write_destructor(_then, key, "else-branch"));
        }
        state.finalize();
        SCOPES_CHECK_RESULT(merge_state(state, _then, "branch-merge"));
        return {};
    }

    SCOPES_RESULT(void) track_If(State &state, If *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(verify_deps(state, node->deps, node->get_type(), "branch"));
        auto &&clauses = node->clauses;
        auto clause_count = clauses.size();
        std::vector<State> states;
        std::vector<State> cond_states;
        states.reserve(clause_count);
        cond_states.reserve(clause_count);
        State *cond_state = &state;
        for (auto &&clause : clauses) {
            if (clause.is_then()) {
                cond_states.push_back(State(clause.cond_body, *cond_state));
                State &clause_cond_state = cond_states.back();
                states.push_back(State(clause.body, clause_cond_state));
                State &clause_state = states.back();
                SCOPES_CHECK_RESULT(track_block(clause_state, clause.value));
                SCOPES_CHECK_RESULT(track_block(clause_cond_state, clause.cond));
                cond_state = &clause_cond_state;
            } else {
                states.push_back(State(clause.body, *cond_state));
                State &clause_state = states.back();
                SCOPES_CHECK_RESULT(track_block(clause_state, clause.value));
                cond_state = nullptr;
            }
        }
        assert(!cond_state);
        int i = clause_count;
        State *else_state = nullptr;
        while (i-- > 0) {
            auto &&clause = clauses[i];
            State &clause_state = states[i];
            if (clause.is_then()) {
                State &clause_cond_state = cond_states[i];
                assert(else_state);
                SCOPES_CHECK_RESULT(merge_states(
                    clause_cond_state, clause_state, *else_state));
                else_state = &clause_cond_state;
            } else {
                else_state = &clause_state;
            }
        }
        assert(else_state);
        SCOPES_CHECK_RESULT(merge_state(state, *else_state, "if-merge"));
        return {};
    }
    SCOPES_RESULT(void) track_Switch(State &state, Switch *node) {
        SCOPES_RESULT_TYPE(void);
        std::vector<State> states;
        states.reserve(node->cases.size());
        for (auto &&_case : node->cases) {
            states.push_back(State(_case.body, state));
            State &case_state = states.back();
            SCOPES_CHECK_RESULT(track_block(case_state, _case.value));
        }
        return {};
    }

    SCOPES_RESULT(void) verify_deps(
        State &state, const Depends &depends, const Type *T, const char *context) {
        SCOPES_RESULT_TYPE(void);
        int count = get_argument_count(T);
        auto &&kinds = depends.kinds;
        for (int i = 0; i < count; ++i) {
            auto ET = get_argument(T, i);
            assert(i < kinds.size());
            if (kinds[i] == DK_Conflicted) {
            #if SCOPES_SOFT_TRACKING
                {
                    StyledString ss;
                    ss.out << Style_Error << "error: " << Style_None << context << " conflict, not all merges perform the same move/borrow operation";
                    state.annotate(ss.str());
                }
            #else
                SCOPES_EXPECT_ERROR(error_cannot_merge_moves(context));
            #endif
            }
        }
        return {};
    }

    SCOPES_RESULT(void) move_argument(State &state,
        const ValueIndex &arg, const char *context) {
        SCOPES_RESULT_TYPE(void);
        auto &data = state.ensure_data(arg);
        #if SCOPES_SOFT_TRACKING
        if (data.will_be_used() || data.will_be_moved()) {
            StyledString ss;
            ss.out
                << Style_Error << "error: " << Style_None
                << "attempt to move " << arg << " which is tagged as ";
            bool acount = 0;
            if (data.will_be_used()) {
                ss.out << "to-be-used by " << data.get_user();
                acount++;
            }
            if (data.will_be_moved()) {
                if (acount)
                    ss.out << " and ";
                ss.out << "to-be-moved by " << data.get_mover();
            }
            state.annotate(ss.str());
            return {};
        }
        #else
        if (data.will_be_used()) {
            SCOPES_EXPECT_ERROR(error_value_in_use(arg.value, data.get_user(), context));
        } else if (data.will_be_moved()) {
            SCOPES_EXPECT_ERROR(error_value_moved(arg.value, data.get_mover(), context));
        }
        #endif
        #if SCOPES_SOFT_TRACKING
        {
            StyledString ss;
            ss.out << context << " tagging " << arg << " as to-be-moved";
            state.annotate(ss.str());
        }
        #endif
        data.move(state.where, state.block.depth);
        return {};
    }

    SCOPES_RESULT(void) track_return_argument(State &state, Value *node,
        int retdepth, const char *context) {
        SCOPES_RESULT_TYPE(void);
        assert(retdepth >= 0);
        assert(node);
        auto T = node->get_type();
        if (!is_returning_value(T))
            return {};
        // collect values to be moved later which are in the scopes we are
        // jumping out of
        State *s = state.parent;
        ValueIndexSet moved;
        while (s && s->block.depth > retdepth) {
            for (auto &entries : s->_data) {
                const ValueIndex &key = entries.first;
                Data &data = entries.second;
                if (data.will_be_moved()
                    && (data.get_move_depth() > retdepth)) {
                    moved.insert(key);
                }
            }
            s = s->parent;
        }
        // finally generate destructors for those values
        state.block.insert_at_end();
        for (auto &&entry : moved) {
            // data will be moved, so we need to write a destructor
            SCOPES_CHECK_RESULT(write_destructor(state, entry, context));
        }
        SCOPES_CHECK_RESULT(visit_value(state, VM_AUTO, node, context, retdepth));
        return {};
    }

    SCOPES_RESULT(void) track_Call(State &state, Call *node) {
        SCOPES_RESULT_TYPE(void);
        auto callee = node->callee;
        const Type *T = callee->get_type();
        if (T == TYPE_Builtin) {
            Builtin b = SCOPES_GET_RESULT(extract_builtin_constant(callee));
            SCOPES_ANCHOR(node->anchor());
            switch(b.value()) {
            case FN_Move:
            case FN_Destroy: {
                assert(node->args.size() == 1);
                SCOPES_CHECK_RESULT(
                    visit_argument(state, VM_FORCE_MOVE,
                        ValueIndex(node->args[0]), "call"));
                return {};
            } break;
            default: break;
            }
        }
        SCOPES_CHECK_RESULT(visit_values(state, VM_AUTO, node->args, "call"));
        SCOPES_CHECK_RESULT(visit_argument(state, VM_AUTO, ValueIndex(node->callee), "call"));
        return {};
    }
    SCOPES_RESULT(void) track_Loop(State &state, Loop *node) {
        SCOPES_RESULT_TYPE(void);
        State loop_state(node->body, state);
        SCOPES_CHECK_RESULT(track_block(loop_state, node->value));
        return track_argument(state, node->init, "loop");
    }
    SCOPES_RESULT(void) track_Break(State &state, Break *node) {
        return track_argument(state, node->value, "break");
    }
    SCOPES_RESULT(void) track_Repeat(State &state, Repeat *node) {
        return track_argument(state, node->value, "repeat");
    }
    SCOPES_RESULT(void) track_Return(State &state, Return *node) {
        return track_return_argument(state, node->value, 0, "return");
    }
    SCOPES_RESULT(void) track_Label(State &state, Label *node) {
        SCOPES_RESULT_TYPE(void);
        State label_state(node->body, state);
        return track_block(label_state, node->value);
    }
    SCOPES_RESULT(void) track_Merge(State &state, Merge *node) {
        SCOPES_RESULT_TYPE(void);
        return track_argument(state, node->value, "merge");
    }
    SCOPES_RESULT(void) track_Raise(State &state, Raise *node) {
        return track_argument(state, node->value, "raise");
    }
    SCOPES_RESULT(void) track_ArgumentList(State &state, ArgumentList *node) {
        SCOPES_RESULT_TYPE(void);
        return {};
    }
    SCOPES_RESULT(void) track_ExtractArgument(State &state, ExtractArgument *node) {
        return track_argument(state, node->value, "extract argument");
    }
    SCOPES_RESULT(void) track_Function(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_Extern(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_ConstInt(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_ConstReal(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_ConstAggregate(State &state, Value *node) {
        assert(false);
        return {};
    }
    SCOPES_RESULT(void) track_ConstPointer(State &state, Value *node) {
        assert(false);
        return {};
    }

    SCOPES_RESULT(void) track_block(State &state, Value *return_value) {
        SCOPES_RESULT_TYPE(void);
        auto &&block = state.block;
        if (return_value) {
            block.insert_at_end();
            state.finalized = true;
            SCOPES_CHECK_RESULT(track_return_argument(
                state, return_value, state.block.depth - 1, "block result"));
            state.finalized = false;
        }
        auto loc = state.where;
        if (block.terminator) {
            auto val = block.terminator;
            block.insert_at_end();
            SCOPES_CHECK_RESULT(track_instruction(state, val));
        }
        auto &&body = block.body;
        int i = body.size();
        while (i-- > 0) {
            block.insert_at(i + 1);
            SCOPES_CHECK_RESULT(track_instruction(state, body[i]));
        }
        state.set_location(loc);
        return {};
    }

    SCOPES_RESULT(void) process() {
        SCOPES_RESULT_TYPE(void);
        StyledStream ss;
        ss << "processing #" << track_count << std::endl;
        const int HALT_AT = 400;
        if (track_count == HALT_AT) {
            StyledStream ss;
            stream_ast(ss, function, StreamASTFormat());
        }
        State root_state(function->body);
        root_state.set_location(function);
        auto fT = extract_function_type(function->get_type());
        auto return_type = fT->return_type;
        auto &&deps = function->deps;
        SCOPES_CHECK_RESULT(verify_deps(root_state, deps,
            return_type, "return"));
        SCOPES_CHECK_RESULT(track_block(root_state, function->value));
        {
            Types rettypes;
            int acount = get_argument_count(return_type);
            auto &&args = deps.args;
            auto &&kinds = deps.kinds;
            for (int i = 0; i < acount; ++i) {
                auto T = get_argument(return_type, i);
                if (i < args.size()) {
                    auto kind = kinds[i];
                    IDSet set;
                    if (kind == DK_Borrowed) {
                        auto &&arg = args[i];
                        for (auto &&key : arg) {
                            auto param = dyn_cast<Parameter>(key.value);
                            if (!param)
                                continue;
                            if (param->owner != function)
                                continue;
                            set.insert(param->index);
                        }
                    }
                    if (!set.empty())
                        T = view_type(T, set);
                }
                rettypes.push_back(T);
            }
            return_type = arguments_type(rettypes);
        }
        Types argtypes;
        for (int i = 0; i < function->params.size(); ++i) {
            auto param = function->params[i];
            const Type *T = param->get_type();
            const Data *data = root_state.find_data(ValueIndex(param));
            if (data) {
                if (data->will_be_moved()) {
                    T = move_type(T);
                }
            }
            argtypes.push_back(T);
        }
        auto newT = native_ro_pointer_type(raising_function_type(
            fT->except_type, return_type, argtypes, fT->flags));
        if (track_count++ == HALT_AT) {
            if (function->get_type() != newT) {
                ss << function->get_type() << " -> " << newT << std::endl;
                function->change_type(newT);
            }
            StyledStream ss;
            stream_ast(ss, function, StreamASTFormat());
            exit(0);
        }

        return {};
    }

    SCOPES_RESULT(void) write_destructor(State &state, const ValueIndex &arg, const char *context) {
        SCOPES_RESULT_TYPE(void);
        #if SCOPES_SOFT_TRACKING
        StyledString ss;
        ss.out << context << " destructor injection for " << arg;
        state.annotate(ss.str());
        #else
        auto argT = arg.get_type();
        // generate destructor
        Value *handler;
        if (!argT->lookup(SYM_DropHandler, handler))
            return {};
        auto where = state.where;
        assert(where);
        auto anchor = where->anchor();
        auto expr =
            Call::from(anchor, handler, {
                ExtractArgument::from(anchor, arg.value, arg.index) });
        SCOPES_CHECK_RESULT(
            prove(ctx.with_block(state.block).with_symbol_target(), expr));
        #endif
        return {};
    }

    // tag a unique value as will_be_used; if this is the first time the value
    // is seen, generate a destructor for it and tag it as will_be_moved
    SCOPES_RESULT(void) visit_unique_argument(State &state, VisitMode mode,
        const ValueIndex &arg, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        if (isa<Pure>(arg.value))
            return {};
        auto T = arg.get_type();
        assert(!arg.has_deps());
        auto &data = state.ensure_data(arg);
        switch(mode) {
        case VM_AUTO: {
            if (!data.will_be_moved()
                && !data.will_be_used()
                && (arg.value->get_depth() > 0) // do not destroy constants and
                                                // function parameters
                ) {
                SCOPES_CHECK_RESULT(
                    move_argument(state, arg, context));
                SCOPES_CHECK_RESULT(write_destructor(state, arg, context));
            }
        } break;
        case VM_FORCE_BORROW: {
        } break;
        case VM_FORCE_MOVE: {
            SCOPES_CHECK_RESULT(
                move_argument(state, arg, context));
        } break;
        }
        #if SCOPES_SOFT_TRACKING
        {
            StyledString ss;
            ss.out << context << " tagging " << arg << " as to-be-used";
            state.annotate(ss.str());
        }
        #endif
        {
            auto &data = state.ensure_data(arg);
            data.use(state.where);
        }
        return {};
    }

    SCOPES_RESULT(void) visit_argument(State &state, VisitMode mode,
        const ValueIndex &arg, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        auto T = arg.get_type();
        auto deps = arg.deps();
        if (retdepth < 0) {
            if (deps) {
                // tag all connected unique values
                const ValueIndexSet &args = *deps;
                for (auto arg : args) {
                    SCOPES_CHECK_RESULT(
                        visit_unique_argument(state, mode, arg, context, retdepth));
                }
            } else {
                // value is unique
                SCOPES_CHECK_RESULT(
                    visit_unique_argument(state, mode, arg, context, retdepth));
            }
            return {};
        }
        int depth = 0;
        if (deps) {
            const ValueIndexSet &args = *deps;
            for (auto arg : args) {
                // deepest depth tells us the shortest lifetime
                depth = std::max(depth, arg.value->get_depth());
            }
        } else {
            depth = arg.value->get_depth();
        }
        if (mode == VM_AUTO) {
            if (depth <= retdepth) {
                // values exist outside of block, we only borrow
                mode = VM_FORCE_BORROW;
            } else {
                // values are inside block, we must move
                mode = VM_FORCE_MOVE;
            }
        }
        if (deps) {
            // value is borrowing
            switch(mode) {
            case VM_FORCE_MOVE: {
            #if SCOPES_SOFT_TRACKING
            {
                StyledString ss;
                ss.out << Style_Error << "error: " << Style_None
                    << "cannot force " << arg << " to move";
                state.annotate(ss.str());
            }
            #else
                // we can't force a borrowed value to move
                SCOPES_EXPECT_ERROR(
                    error_value_is_borrowed(arg.value, state.where, context));
            #endif
            } break;
            default: {
                // tag all connected unique values
                const ValueIndexSet &args = *deps;
                for (auto arg : args) {
                    SCOPES_CHECK_RESULT(
                        visit_unique_argument(state, mode, arg, context, retdepth));
                }
            } break;
            }
        } else {
            // value is unique
            SCOPES_CHECK_RESULT(
                visit_unique_argument(state, mode, arg, context, retdepth));
        }
        return {};
    }

    SCOPES_RESULT(void) visit_value(State &state, VisitMode mode,
        Value *node, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        auto T = node->get_type();
        if (!is_returning(T))
            return {};
        int argc = get_argument_count(T);
        for (int i = 0; i < argc; ++i) {
            SCOPES_CHECK_RESULT(visit_argument(state, mode,
                ValueIndex(node, i), context, retdepth));
        }
        return {};
    }

    SCOPES_RESULT(void) visit_values(State &state, VisitMode mode,
        const Values &args, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        int i = args.size();
        while (i-- > 0) {
            SCOPES_CHECK_RESULT(visit_argument(state, mode,
                ValueIndex(args[i]), context, retdepth));
        }
        return {};
    }

    SCOPES_RESULT(void) track_subargument(State &state,
        const ValueIndex &arg, const char *context) {
        SCOPES_RESULT_TYPE(void);
        auto &data = state.ensure_data(arg);
        if (data.will_be_used())
            return {};
        SCOPES_CHECK_RESULT(
            move_argument(state, arg, context));
        return write_destructor(state, arg, context);
    }

    SCOPES_RESULT(void) track_argument(State &state, Value *node,
        const char *context) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = node->get_type();
        if (!is_returning(T))
            return {};
        int argc = get_argument_count(T);
        for (int i = 0; i < argc; ++i) {
            auto argT = get_argument(T, i);
            SCOPES_CHECK_RESULT(track_subargument(state,
                ValueIndex(node, i), context));
        }
        return {};
    }

    SCOPES_RESULT(void) track_declaration(State &state, Value *node) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = strip_qualifiers(node->get_type());
        if (!is_returning(T))
            return {};
        SCOPES_CHECK_RESULT(visit_value(state, VM_AUTO, node, "declaration"));
        if (!isa<ArgumentList>(node)) {
            int argc = get_argument_count(T);
            for (int i = 0; i < argc; ++i) {
                auto arg = ValueIndex(node, i);
                state.delete_data(arg);
            }
        }
        return {};
    }

    SCOPES_RESULT(void) track_arguments(State &state,
        const Values &args, const char *context) {
        SCOPES_RESULT_TYPE(void);
        int i = args.size();
        while (i-- > 0) {
            SCOPES_CHECK_RESULT(track_argument(state, args[i], context));
        }
        return {};
    }

    SCOPES_RESULT(void) track_instruction(State &state, Value *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ANCHOR(node->anchor());
        state.set_location(node);
        SCOPES_CHECK_RESULT(track_declaration(state, node));
        Result<_result_type> result;
        switch(node->kind()) {
    #define T(NAME, BNAME, CLASS) \
        case NAME: result = track_ ## CLASS(state, cast<CLASS>(node)); break;
        SCOPES_VALUE_KIND()
    #undef T
        default: assert(false);
        }
        SCOPES_CHECK_RESULT(result);
        return {};
    }

    ASTContext &ctx;
    Function *function;
};

SCOPES_RESULT(void) track(ASTContext &ctx) {
    SCOPES_RESULT_TYPE(void);
    Tracker tracker(ctx);
    SCOPES_CHECK_RESULT(tracker.process());
    return {};
}

} // scopes

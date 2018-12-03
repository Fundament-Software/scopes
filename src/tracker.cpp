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
#include "qualifier.inc"

#include <unordered_map>

// annotate blocks and instructions with actions
#define SCOPES_ANNOTATE_TRACKING 1

namespace scopes {

#define SCOPES_GEN_TARGET "Lifetime"

/*
for more info on borrow inference, see
https://gist.github.com/paniq/71251083aa52c1577f2d1b22be0ac6e1

lessons learned:
* nearly all built-ins consume arguments as handles
* "plain" typed arguments can be copied, and so views can also be copied to new handles
* any instruction with noreturn type requires post-actions to be executed first,
  and all arguments must be consumed by the instruction

*/

//------------------------------------------------------------------------------

static int track_count = 0;

struct Tracker {
    enum VisitMode {
        // inject destructor when last reference
        VM_AUTO,
        // always view
        VM_FORCE_VIEW,
        // force move (into parent context)
        VM_FORCE_MOVE,
        // force move (into another context)
        // plain types will be copied, and so no move takes place
        VM_FORCE_COPY_OR_MOVE,
        // force move with destructor injection, simulating the value
        // being destroyed by a called function
        // plain types will be copied, and so no drop takes place
        VM_FORCE_COPY_OR_DROP,
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
            : block(_block), parent(nullptr), where(nullptr),
                depth_offset(0), finalized(false), test(false)
        {
        }

        State(Block &_block, State &_parent)
            : block(_block), parent(&_parent), where(_parent.where),
                depth_offset(0), finalized(false), test(_parent.test)
        {
            for (auto &&key : _parent._data) {
                _data.insert({key.first, key.second});
            }
        }

        int translate_depth(int depth) {
            if (depth >= block.depth)
                return depth + depth_offset;
            return depth;
        }

        int get_block_depth(Block &block) {
            return translate_depth(block.depth);
        }

        int get_depth() {
            return get_block_depth(block);
        }

        int get_value_depth(Value *value) {
            return translate_depth(value->get_depth());
        }

        void set_depth_offset(int offset) {
            depth_offset = offset;
        }

        void set_test() {
            test = true;
        }

        bool is_test() {
            return test;
        }

        void finalize() {
            finalized = true;
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
            #if SCOPES_ANNOTATE_TRACKING
            //StyledStream ss;
            //ss << "deleting " << block.depth << " " << block.insert_index << " " << depth_offset << " " << test << " " << value.value << std::endl;
            #endif
            assert(!garbage.count(value));
            assert(!_deleted.count(value));
            auto it = _data.find(value);
            if (it != _data.end()) {
                _data.erase(it);
            }
            _deleted.insert(value);
        }

        Data &ensure_data(const ValueIndex &value) {
            #if SCOPES_ANNOTATE_TRACKING
            //StyledStream ss;
            //ss << "ensuring " << block.depth << " " << block.insert_index << " " << depth_offset << " " << test << " " << value.value << std::endl;
            if (_deleted.count(value)) {
                StyledStream ss;
                ss << "error resurrecting future value: " << std::endl;
                stream_ast(ss, value.value, StreamASTFormat::singleline());
                ss << std::endl;
            }
            #endif
            assert(!_deleted.count(value));
            auto data = find_data(value);
            if (data)
                return *data;
            auto result = _data.insert({value, Data()});
            return result.first->second;
        }

        bool garbage_empty() {
            return garbage.empty();
        }

        void clear_garbage() {
            garbage.clear();
            collect_order.clear();
        }

        void drop(const ValueIndex &value) {
            if (garbage.count(value))
                return;
            assert(!_deleted.count(value));
            garbage.insert(value);
            collect_order.push_back(value);
        }

        Block &block;
        State *parent;
        Value *where;
        int depth_offset;
        bool finalized;
        bool test;
        std::unordered_map<ValueIndex, Data, ValueIndex::Hash> _data;
        std::unordered_set<ValueIndex, ValueIndex::Hash> _deleted;
        ValueIndexSet garbage;
        ValueIndices collect_order;
    };

    typedef std::vector<State> States;

    Tracker(ASTContext &_ctx)
        : ctx(_ctx), function(_ctx.function), active_loop(nullptr)
    {}

#define T(NAME, BNAME, CLASS) \
    SCOPES_RESULT(void) track_ ## CLASS(State &state, Value *node) { \
        SCOPES_RESULT_TYPE(void); \
        SCOPES_EXPECT_ERROR(error_cannot_translate(SCOPES_GEN_TARGET, node)); \
    }
    SCOPES_TEMPLATE_VALUE_KIND()
#undef T

#define T(NAME, BNAME, CLASS) \
    SCOPES_RESULT(void) track_ ## CLASS(State &state, Value *node) { \
        assert(false); \
        return {}; \
    }
    SCOPES_PURE_VALUE_KIND()
#undef T

    SCOPES_RESULT(void) track_Parameter(State &state, Value *node) {
        assert(false);
        return {};
    }

    SCOPES_RESULT(void) track_LoopLabelArguments(State &state, Value *node) {
        assert(false);
        return {};
    }

    SCOPES_RESULT(void) track_LabelArguments(State &state, Value *node) {
        assert(false);
        return {};
    }

    SCOPES_RESULT(void) track_Exception(State &state, Value *node) {
        assert(false);
        return {};
    }

    SCOPES_RESULT(void) merge_state(State &state, State &sub, const char *context, bool drop_arguments = false) {
        SCOPES_RESULT_TYPE(void);
        assert(&state != &sub);
        int retdepth = state.get_depth();
        assert(retdepth >= 0);
        for (auto &entries : sub._data) {
            const ValueIndex &key = entries.first;
            Data &data = entries.second;
            int depth = sub.get_value_depth(key.value);
            if (data.will_be_moved()
                && (depth <= retdepth)
                && (data.get_move_depth() > retdepth)) {
                if (drop_arguments) {
                    SCOPES_CHECK_RESULT(drop_argument(state, key, context));
                } else {
                    SCOPES_CHECK_RESULT(move_argument(state, key, context));
                }
            }
        }
        return {};
    }

    void collect_moved_keys(State &substate, ValueIndexSet &moved, int retdepth) {
        for (auto &entries : substate._data) {
            const ValueIndex &key = entries.first;
            Data &data = entries.second;
            int depth = substate.get_value_depth(key.value);
            if (data.will_be_moved()
                && (data.get_move_depth() > retdepth)) {
                moved.insert(key);
            }
        }
    }

    SCOPES_RESULT(void) drop_moved_keys(State &state, const ValueIndexSet &moved, const char *context) {
        SCOPES_RESULT_TYPE(void);
        for (auto key : moved) {
            Data &data = state.ensure_data(key);
            if (data.will_be_moved())
                continue;
            SCOPES_CHECK_RESULT(drop_argument(state, key, context));
        }
        return {};
    }

    SCOPES_RESULT(void) merge_states(State &state, States &states) {// State &_then, State &_else) {
        SCOPES_RESULT_TYPE(void);
        int retdepth = state.get_depth();
        assert(retdepth >= 0);
        ValueIndexSet moved;
        for (auto &&substate : states) {
            collect_moved_keys(substate, moved, retdepth);
            substate.finalize();
        }
        for (auto &&substate : states) {
            SCOPES_CHECK_RESULT(drop_moved_keys(substate, moved, "merge-branches"));
        }
        return {};
    }

    SCOPES_RESULT(void) track_CondBr(State &state, CondBr *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(verify_deps(state, node, node->deps, node->get_type(), "branch"));
        States states = {
            State(node->then_body, state),
            State(node->else_body, state)
        };
        SCOPES_CHECK_RESULT(track_block(states[0]));
        SCOPES_CHECK_RESULT(track_block(states[1]));
        // problem here: the conditional is consumed by the branch check, but needs to be
        // destroyed afterwards; since we have no guarantee that our branches return, we need to
        // inject the destructors in both branches
        SCOPES_CHECK_RESULT(visit_value(states[0], VM_FORCE_COPY_OR_DROP, node->cond, "conditional"));
        SCOPES_CHECK_RESULT(collect(states[0]));
        SCOPES_CHECK_RESULT(visit_value(states[1], VM_FORCE_COPY_OR_DROP, node->cond, "conditional"));
        SCOPES_CHECK_RESULT(collect(states[1]));
        SCOPES_CHECK_RESULT(merge_states(
            state, states));
        SCOPES_CHECK_RESULT(merge_state(state, states[0], "branch-merge"));
        return {};
    }
    SCOPES_RESULT(void) track_Switch(State &state, Switch *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(verify_deps(state, node, node->deps, node->get_type(), "case"));
        int retdepth = state.get_depth();
        assert(retdepth >= 0);
        States states;
        const int count = node->cases.size();
        states.reserve(count);
        for (auto &&_case : node->cases) {
            states.push_back(State(_case.body, state));
        }
        State *next_state = nullptr;
        ValueIndexSet moved;
        for (int i = count; i-- > 0;) {
            State &case_state = states[i];
            auto &&_case = node->cases[i];
            switch(_case.kind) {
            case CK_Case:
                 assert(false); // continue
            case CK_Pass: {
                if (!_case.body.terminator) {
                    assert(next_state);
                    SCOPES_CHECK_RESULT(merge_state(case_state, *next_state, "pass-merge"));
                }
            } break;
            case CK_Default: break;
            }
            SCOPES_CHECK_RESULT(track_block(case_state));
            collect_moved_keys(case_state, moved, retdepth);
            case_state.finalize();
            next_state = &case_state;
        }

        //SCOPES_CHECK_RESULT(visit_value(case_state, VM_FORCE_COPY_OR_DROP, node->expr, "selector"));
        //SCOPES_CHECK_RESULT(collect(case_state));
        next_state = nullptr;
        for (int i = count; i-- > 0;) {
            auto &&_case = node->cases[i];
            auto &&case_state = states[i];
            switch(_case.kind) {
            case CK_Case:
                 assert(false); // continue
            case CK_Pass: {
                if (_case.body.terminator) {
                    SCOPES_CHECK_RESULT(drop_moved_keys(case_state, moved, "merge-case"));
                } else {
                    assert(next_state);
                }
            } break;
            case CK_Default: {
                SCOPES_CHECK_RESULT(drop_moved_keys(case_state, moved, "merge-default"));
            } break;
            }
            next_state = &case_state;
        }
        return {};
    }

    SCOPES_RESULT(void) verify_deps(
        State &state, Value *from,
        const Depends &depends, const Type *T, const char *context) {
        SCOPES_RESULT_TYPE(void);
        int count = get_argument_count(T);
        auto &&kinds = depends.kinds;
        for (int i = 0; i < count; ++i) {
            auto ET = get_argument(T, i);
            if (is_plain(ET))
                continue;
            #if SCOPES_ANNOTATE_TRACKING
            if (i >= kinds.size()) {
                StyledStream ss;
                ss << "possibly incomplete instruction:" << std::endl;
                stream_ast(ss, from, StreamASTFormat());
            }
            #endif
            assert(i < kinds.size());
            if (kinds[i] == DK_Conflicted) {
                SCOPES_EXPECT_ERROR(error_cannot_merge_moves(context));
            }
        }
        return {};
    }

    SCOPES_RESULT(void) track_Call(State &state, Call *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ANCHOR(node->anchor());
        // special care needs to be taken with nonreturning functions, which
        // must move all their arguments.
        auto callee = node->callee;
        const Type *T = callee->get_type();
        if (is_function_pointer(T)) {
            const FunctionType *ft = extract_function_type(T);
            bool returning = is_returning(ft->return_type);
            int raise_retdepth = -1;
            State except_state(node->except_body, state);
            if (ft->has_exception()) {
                #if 0
                if (!returning) {
                    auto term = node->except_body.terminator;
                    assert(term);
                    switch(term->kind()) {
                    case VK_Raise:
                        raise_retdepth = 0;
                        break;
                    case VK_Merge:
                        raise_retdepth = state.get_value_depth(cast<Merge>(term)->label);
                        break;
                    default:
                        assert(false); break;
                    }
                }
                #endif
                // generate unwind destructors for an untimely exit
                SCOPES_CHECK_RESULT(track_block(except_state));
            }
            int numargs = (int)ft->argument_types.size();
            assert(numargs <= node->args.size());
            for (int i = 0; i < numargs; ++i) {
                Value *arg = node->args[i];
                const Type *argT = ft->argument_types[i];
                auto vq = try_qualifier<ViewQualifier>(argT);
                if (vq) {
                    if (!returning) {
                        SCOPES_EXPECT_ERROR(error_nonreturning_function_must_move());
                    }
                    // we're viewing
                    SCOPES_CHECK_RESULT(visit_value(state, VM_FORCE_VIEW, arg, "argument", raise_retdepth));
                } else {
                    // we're moving
                    SCOPES_CHECK_RESULT(visit_value(state,
                        VM_FORCE_COPY_OR_MOVE, arg,
                        "argument", raise_retdepth));
                }
            }
            if (ft->has_exception()) {
                SCOPES_CHECK_RESULT(write_collected_destructors(except_state, state.collect_order, "argument"));
            }
        } else if (T == TYPE_Builtin) {
            Builtin b = SCOPES_GET_RESULT(extract_builtin_constant(callee));
            switch(b.value()) {
            case FN_Annotate: {
                return {};
            } break;
            case FN_Move:
            case FN_Forget: {
                assert(node->args.size() == 1);
                SCOPES_CHECK_RESULT(
                    visit_argument(state, VM_FORCE_COPY_OR_MOVE,
                        ValueIndex(node->args[0]), "call"));
                return {};
            } break;
            default: {
                SCOPES_CHECK_RESULT(visit_values(state, VM_FORCE_COPY_OR_DROP, node->args, "call"));
            } break;
            }
        } else {
            SCOPES_CHECK_RESULT(visit_values(state, VM_AUTO, node->args, "call"));
        }
        SCOPES_CHECK_RESULT(visit_argument(state, VM_AUTO, ValueIndex(node->callee), "call"));
        return {};
    }
    SCOPES_RESULT(void) track_LoopLabel(State &state, LoopLabel *node) {
        SCOPES_RESULT_TYPE(void);
        auto old_active_loop = active_loop;
        active_loop = node;
        State loop_state(node->body, state);
        State test_loop_state(node->body, loop_state);
        test_loop_state.set_test();
        test_loop_state.set_depth_offset(1);
        /*
            1. we need to look at how the loop moves values to the next loop and pull init values
                in the same way; we also need to ensure that all repeats perform the same
                operation.
            2. moving values from outside the loop scope is illegal, because if the loop
                runs a second time, the value has already been moved by the previous iteration.

                we can either carry a flag that enforces borrows everywhere we would otherwise
                move, or just try to prove [iteration n [iteration n-1]]; that means we run the
                loop twice; once, to generate the initial state, and then starting from that state,
                we run it again; but we need to suppress the generation of destructors on the first
                run.

                for the second run, we need to ensure that the loop itself is not tracked yet

            3. after the loop, we need to inject destructors for all outer values that have been moved inside
                this must be done from the break label
        */
        SCOPES_CHECK_RESULT(track_block(test_loop_state));
        SCOPES_CHECK_RESULT(merge_state(loop_state, test_loop_state, "loop-merge-test"));
        SCOPES_CHECK_RESULT(track_block(loop_state));
        SCOPES_CHECK_RESULT(visit_value(loop_state, VM_FORCE_COPY_OR_MOVE, node->init, "loop-init"));
        SCOPES_CHECK_RESULT(merge_state(state, loop_state, "loop-merge"));
        active_loop = old_active_loop;
        return {};
    }
    SCOPES_RESULT(void) track_Repeat(State &state, Repeat *node) {
        assert(node->loop);
        assert(node->loop == active_loop);
        int retdepth = state.get_value_depth(node->loop);
        // TODO: we're not breaking, so this is not entirely true. but maybe it's enough?
        return track_return_argument(state, node->value, retdepth, "repeat");
    }
    SCOPES_RESULT(void) track_Return(State &state, Return *node) {
        return track_return_argument(state, node->value, 0, "return");
    }
    SCOPES_RESULT(void) track_Label(State &state, Label *node) {
        SCOPES_RESULT_TYPE(void);

        bool do_drop = false;
        #if 0
        if (is_returning(node->get_type())) {
            if (node->label_kind == LK_Break) {
                do_drop = true;
            }
        }
        #endif
        State label_state(node->body, state);
        SCOPES_CHECK_RESULT(track_block(label_state));
        SCOPES_CHECK_RESULT(merge_state(state, label_state, "label-merge", do_drop));
        return {};
    }
    SCOPES_RESULT(void) track_Merge(State &state, Merge *node) {
        SCOPES_RESULT_TYPE(void);
        int retdepth = state.get_value_depth(node->label);
        return track_return_argument(state, node->value, retdepth,
            get_label_kind_name(node->label->label_kind));
    }
    SCOPES_RESULT(void) track_Raise(State &state, Raise *node) {
        return track_return_argument(state, node->value, 0, "raise");
    }
    SCOPES_RESULT(void) track_ArgumentList(State &state, ArgumentList *node) {
        SCOPES_RESULT_TYPE(void);
        return {};
    }
    SCOPES_RESULT(void) track_ExtractArgument(State &state, ExtractArgument *node) {
        assert(!node->vararg);
        return visit_argument(state, VM_AUTO, ValueIndex(node->value, node->index), "extract argument");
    }

    SCOPES_RESULT(void) track_block(State &state) {
        SCOPES_RESULT_TYPE(void);
        auto &&block = state.block;
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
        block.insert_at(0);
        state.set_location(loc);
        return {};
    }

    /*
    SCOPES_RESULT(void) track_block(State &state, Value *return_value) {
        SCOPES_RESULT_TYPE(void);
        auto &&block = state.block;
        if (return_value) {
            block.insert_at_end();
            state.finalized = true;
            SCOPES_CHECK_RESULT(track_return_argument(
                state, return_value, state.get_depth() - 1, "block result"));
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
    */

    SCOPES_RESULT(void) process() {
        SCOPES_RESULT_TYPE(void);
        StyledStream ss;
        ss << "processing #" << track_count << std::endl;
        //const int HALT_AT = 9; // loop
        //const int HALT_AT = 11; // another loop
        //const int HALT_AT = 62; // nested try/except blocks
        //const int HALT_AT = 120; // switch case
        //const int HALT_AT = 174; // function with mixed return type
        const int HALT_AT = 327; // use of argument list
        //const int HALT_AT = -1;
        if (track_count == HALT_AT) {
            StyledStream ss;
            stream_ast(ss, function, StreamASTFormat());
        }
        State root_state(function->body);
        root_state.set_location(function);
        auto fT = extract_function_type(function->get_type());
        auto return_type = fT->return_type;
        auto &&deps = function->deps;
        SCOPES_CHECK_RESULT(verify_deps(root_state, function, deps,
            return_type, "return"));
        SCOPES_CHECK_RESULT(track_block(root_state));
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
                    if (kind == DK_Viewed) {
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
                    if (!set.empty() && !is_plain(T))
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
            if (!(data && data->will_be_moved()) && !is_plain(T)) {
                T = view_type(T, {});
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

    void view_argument(State &state,
        const ValueIndex &arg, const char *context) {
        auto &data = state.ensure_data(arg);
        data.use(state.where);
    }

    SCOPES_RESULT(void) move_argument(State &state,
        const ValueIndex &arg, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        auto &data = state.ensure_data(arg);
        // if this is a return move
        if (retdepth >= 0) {
            // if the data we are moving becomes inaccessible anyway
            if (state.get_value_depth(arg.value) > retdepth) {
                // if it has not been moved
                if (!data.will_be_moved()) {
                    // move it
                    data.move(state.where, state.get_depth());
                }
                // otherwise do nothing
                return {};
            }
        }
        if (data.will_be_used()) {
            SCOPES_EXPECT_ERROR(error_value_in_use(arg.value, data.get_user(), context));
        } else if (data.will_be_moved()) {
            SCOPES_EXPECT_ERROR(error_value_moved(arg.value, data.get_mover(), context));
        }
        data.move(state.where, state.get_depth());
        return {};
    }

    SCOPES_RESULT(void) write_return_destructors(State &state,
        int retdepth, Value *ignore_node, const char *context) {
        SCOPES_RESULT_TYPE(void);
        assert(retdepth >= 0);
        // collect values to be moved later which are in the scopes we are
        // jumping out of
        State *s = state.parent;
        ValueIndexSet moved;
        while (s && s->get_depth() > retdepth) {
            for (auto &entries : s->_data) {
                const ValueIndex &key = entries.first;
                if (key.value == ignore_node)
                    continue;
                Data &data = entries.second;
                if (data.will_be_moved()
                    && (data.get_move_depth() > retdepth)) {
                    moved.insert(key);
                }
            }
            for (auto vi : s->collect_order) {
                moved.insert(vi);
            }
            s = s->parent;
        }
        // finally generate destructors for those values
        state.block.insert_at_end();
        for (auto &&entry : moved) {
            // data will be moved, so we need to write a destructor
            SCOPES_CHECK_RESULT(write_destructor(state, entry, context));
        }

        return {};
    }

    SCOPES_RESULT(void) track_return_argument(State &state, Value *node,
        int retdepth, const char *context) {
        SCOPES_RESULT_TYPE(void);
        assert(retdepth >= 0);
        assert(node);
        SCOPES_CHECK_RESULT(write_return_destructors(state, retdepth, node, context));
        auto T = node->get_type();
        if (is_returning_value(T)) {
            SCOPES_CHECK_RESULT(visit_value(state, VM_AUTO, node, context, retdepth));
        }
        return {};
    }

    SCOPES_RESULT(void) drop_argument(State &state, const ValueIndex &arg, const char *context, int retdepth = -1) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(move_argument(state, arg, context, retdepth));
        return write_destructor(state, arg, context);
    }

    void write_annotation(State &state, const String *msg, Values values) {
        auto where = state.where;
        assert(where);
        auto anchor = where->anchor();
        values.insert(values.begin(), ConstPointer::string_from(anchor, msg));
        auto expr =
            CallTemplate::from(anchor,
                ConstInt::builtin_from(anchor, Builtin(FN_Annotate)),
                    values);
        prove(ctx.with_block(state.block), expr).assert_ok();
    }

    SCOPES_RESULT(void) write_destructor(State &state, const ValueIndex &arg, const char *context) {
        SCOPES_RESULT_TYPE(void);
        if (state.test)
            return {};
        // generate destructor
        auto argT = arg.get_type();
        Value *handler;
        if (!argT->lookup(SYM_DropHandler, handler)) {
            #if SCOPES_ANNOTATE_TRACKING
            StyledString ss;
            ss.out << context << ": forget";
            write_annotation(state, ss.str(), { arg.value });
            #endif
            return {};
        }
        #if SCOPES_ANNOTATE_TRACKING
        StyledString ss;
        ss.out << context << " destruct";
        write_annotation(state, ss.str(), { arg.value });
        #endif
        auto where = state.where;
        assert(where);
        auto anchor = where->anchor();
        auto expr =
            CallTemplate::from(anchor, handler, {
                ExtractArgument::from(anchor, arg.value, arg.index) });
        SCOPES_CHECK_RESULT(
            prove(ctx.with_block(state.block), expr));
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
        bool last_appearance = !(data.will_be_moved() || data.will_be_used());
        bool is_local = (state.get_value_depth(arg.value) > 0);
        // only move/destroy when inside function - don't touch function
        // parameters and global constants
        bool must_drop = last_appearance && is_local;
        switch(mode) {
        case VM_AUTO: {
            if (must_drop) {
                state.drop(arg);
            } else {
                view_argument(state, arg, context);
            }
        } break;
        case VM_FORCE_VIEW: {
            if (must_drop) {
                state.drop(arg);
            } else {
                view_argument(state, arg, context);
            }
        } break;
        case VM_FORCE_MOVE: {
            SCOPES_CHECK_RESULT(
                move_argument(state, arg, context, retdepth));
        } break;
        case VM_FORCE_COPY_OR_MOVE: {
            if (is_plain(arg.get_type())) {
                // last use of original, which we have copied?
                if (must_drop) {
                    state.drop(arg);
                } else {
                    view_argument(state, arg, context);
                }
            } else {
                SCOPES_CHECK_RESULT(
                    move_argument(state, arg, context, retdepth));
            }
        } break;
        case VM_FORCE_COPY_OR_DROP: {
            if (is_plain(arg.get_type())) {
                // last use of original, which we have copied?
                if (must_drop) {
                    state.drop(arg);
                } else {
                    view_argument(state, arg, context);
                }
            } else {
                // drop immediately
                SCOPES_CHECK_RESULT(
                    drop_argument(state, arg, context, retdepth));
            }
        } break;
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
                depth = std::max(depth, state.get_value_depth(arg.value));
            }
        } else {
            depth = state.get_value_depth(arg.value);
        }
        if (mode == VM_AUTO) {
            if (depth <= retdepth) {
                // values exist outside of block, we only view
                mode = VM_FORCE_VIEW;
            } else {
                // values are inside block, we must move
                mode = VM_FORCE_MOVE;
            }
        }
        if (deps) {
            // value is viewing
            switch(mode) {
            case VM_FORCE_COPY_OR_DROP:
            case VM_FORCE_COPY_OR_MOVE:
            case VM_FORCE_MOVE: {
                if (is_plain(arg.get_type())) {
                    // tag all connected unique values as viewed
                    const ValueIndexSet &args = *deps;
                    for (auto arg : args) {
                        SCOPES_CHECK_RESULT(
                            visit_unique_argument(state, VM_FORCE_VIEW, arg, context, retdepth));
                    }
                } else {
                    // we can't force a viewed value to move
                    SCOPES_EXPECT_ERROR(
                        error_value_is_viewed(arg.value, state.where, context));
                }
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

    SCOPES_RESULT(void) delete_declaration(State &state, Value *node) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = strip_qualifiers(node->get_type());
        if (!is_returning(T))
            return {};
        if (!isa<ArgumentList>(node)) {
            int argc = get_argument_count(T);
            for (int i = 0; i < argc; ++i) {
                auto arg = ValueIndex(node, i);
                state.delete_data(arg);
            }
        }
        return {};
    }

    SCOPES_RESULT(void) track_instruction(State &state, Value *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ANCHOR(node->anchor());
        assert(state.garbage_empty());
        state.set_location(node);
        SCOPES_CHECK_RESULT(visit_value(state, VM_AUTO, node, "declaration"));
        SCOPES_CHECK_RESULT(collect(state));
        // lifetime must end before body is evaluated
        SCOPES_CHECK_RESULT(delete_declaration(state, node));
        Result<_result_type> result;
        switch(node->kind()) {
    #define T(NAME, BNAME, CLASS) \
        case NAME: result = track_ ## CLASS(state, cast<CLASS>(node)); break;
        SCOPES_VALUE_KIND()
    #undef T
        default: assert(false);
        }
        SCOPES_CHECK_RESULT(result);
        SCOPES_CHECK_RESULT(collect(state));
        return {};
    }

    SCOPES_RESULT(void) write_collected_destructors(State &state,
        const ValueIndices &collect_order, const char *context) {
        SCOPES_RESULT_TYPE(void);
        // drop all collected values
        for (auto arg : collect_order) {
            SCOPES_CHECK_RESULT(drop_argument(state, arg, context));
        }
        return {};
    }

    SCOPES_RESULT(void) collect(State &state) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(write_collected_destructors(state, state.collect_order, "collector"));
        state.clear_garbage();
        return {};
    }

    ASTContext &ctx;
    Function *function;
    LoopLabel *active_loop;
};

SCOPES_RESULT(void) track(ASTContext &ctx) {
    SCOPES_RESULT_TYPE(void);
    Tracker tracker(ctx);
    SCOPES_CHECK_RESULT(tracker.process());
    return {};
}

} // scopes

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

#include <unordered_map>

namespace scopes {

/*

* iterate from bottom to top

* when tracked type is first generated: remove value data from state, as the
  value does not exist earlier than this OR tag data as "unborn"

* when tracked type is passed to return/raise/break/merge:
    * when return block depth is greater-equal than value block depth, then the
        value exists outside of the block, add merge instruction as viewer
    * otherwise the value must be moved, add merge instruction as mover

* when returning out of expression:
    * generate destructors for all objects that will be moved later

*/

//------------------------------------------------------------------------------

struct Tracker {

    struct ValueIndex {
        struct Hash {
            std::size_t operator()(const ValueIndex & s) const {
                return hash2(std::hash<Value *>{}(s.value), s.index);
            }
        };

        ValueIndex(Value *_value, int _index = 0)
            : value(_value), index(_index) {}

        bool operator ==(const ValueIndex &other) const {
            return value == other.value && index == other.index;
        }

        Value *value;
        int index;
    };

    typedef std::unordered_set<ValueIndex, ValueIndex::Hash> ValueIndexSet;

    struct Data {
        Data() :
            unborn(false), mover(nullptr), move_depth(-1)
        {}

        bool will_be_used() const {
            return will_be_moved() || will_be_viewed();
        }

        bool will_be_moved() const {
            return mover != nullptr;
        }

        bool will_be_viewed() const {
            return !viewers.empty();
        }

        void set_unborn() {
            unborn = true;
        }

        bool is_born() const {
            return !unborn;
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

        ValueSet &get_viewers() { return viewers; }
        const ValueSet &get_viewers() const { return viewers; }

    protected:
        bool unborn;
        int move_depth;
        Value *mover;
        ValueSet viewers;
    };

    // per block
    struct State {
        State(Block &_block)
            : block(_block), parent(nullptr), insert_index(-1)
        {}

        State(Block &_block, State &_parent)
            : block(_block), parent(&_parent), insert_index(-1)
        {
            for (auto &&key : _parent._data) {
                _data.insert({key.first, key.second});
            }
        }

        Data *find_data(const ValueIndex &value) {
            auto it = _data.find(value);
            if (it != _data.end()) {
                Data &data = it->second;
                return &data;
            }
            return nullptr;
        }

        Data &get_data(const ValueIndex &value) {
            auto data = find_data(value);
            if (data)
                return *data;
            auto result = _data.insert({value, Data()});
            return result.first->second;
        }

        void merge_branch(State &s) {
            for (auto &&key : _data) {
                auto it = s._data.find(key.first);
                if (it == s._data.end())
                    continue;
                auto &topdata = key.second;
                auto &data = it->second;
            }
        }

        void merge_branches(State &a, State &b) {
            /*
            for (auto &&key : a) {
            }
            */

        }

        Block &block;
        State *parent;
        int insert_index;
        std::unordered_map<ValueIndex, Data, ValueIndex::Hash> _data;
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
            int depth = get_value_depth(key.value);
            if (data.will_be_moved()
                && data.is_born()
                && (data.get_move_depth() > retdepth)) {
                set.insert(key);
            }
        }
    }

    SCOPES_RESULT(void) merge_states(State &state, Value *instr, State &_then, State &_else) {
        SCOPES_RESULT_TYPE(void);
        assert(&state != &_then);
        assert(&_then != &_else);
        int retdepth = state.block.depth;
        assert(retdepth >= 0);
        ValueIndexSet then_moved;
        ValueIndexSet else_moved;
        collect_local_keys(then_moved, _then, retdepth);
        collect_local_keys(else_moved, _else, retdepth);
        for (auto key : then_moved) {
            SCOPES_CHECK_RESULT(move_argument(state, instr, key));
            if (else_moved.count(key))
                continue;
            auto T = strip_qualifiers(key.value->get_type());
            auto argT = get_argument(T, key.index);
            SCOPES_CHECK_RESULT(write_destructor(_else, instr, argT, key));
        }
        for (auto key : else_moved) {
            if (then_moved.count(key))
                continue;
            SCOPES_CHECK_RESULT(move_argument(state, instr, key));
            auto T = strip_qualifiers(key.value->get_type());
            auto argT = get_argument(T, key.index);
            SCOPES_CHECK_RESULT(write_destructor(_then, instr, argT, key));
        }

        return {};
    }
    SCOPES_RESULT(void) track_If(State &state, If *node) {
        SCOPES_RESULT_TYPE(void);
        /*
        if cond1
            drop 1
        else
            if cond2
                drop 2
            else
                if cond3
                    drop 3
                else
                    drop 4
        */
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
                SCOPES_CHECK_RESULT(track_block(clause_state, node, clause.value));
                SCOPES_CHECK_RESULT(track_block(clause_cond_state, node, clause.cond));
                cond_state = &clause_cond_state;
            } else {
                states.push_back(State(clause.body, *cond_state));
                State &clause_state = states.back();
                SCOPES_CHECK_RESULT(track_block(clause_state, node, clause.value));
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
                    clause_cond_state, node, clause_state, *else_state));
                else_state = &clause_cond_state;
            } else {
                else_state = &clause_state;
            }
        }
        return {};
    }
    SCOPES_RESULT(void) track_Switch(State &state, Switch *node) {
        SCOPES_RESULT_TYPE(void);
        std::vector<State> states;
        states.reserve(node->cases.size());
        for (auto &&_case : node->cases) {
            states.push_back(State(_case.body, state));
            State &case_state = states.back();
            SCOPES_CHECK_RESULT(track_block(case_state, node, _case.value));
        }
        return {};
    }
    SCOPES_RESULT(void) track_Try(State &state, Try *node) {
        SCOPES_RESULT_TYPE(void);
        State except_state(node->except_body, state);
        SCOPES_CHECK_RESULT(track_block(except_state, node, node->except_value));
        State try_state(node->try_body, state);
        return track_block(try_state, node, node->try_value);
    }

    SCOPES_RESULT(void) move_argument(State &state, Value *mover,
        const ValueIndex &arg) {
        SCOPES_RESULT_TYPE(void);
        auto &data = state.get_data(arg);
        if (data.will_be_viewed()) {
            SCOPES_EXPECT_ERROR(error_value_already_in_use(arg.value, data.get_viewers()));
        } else if (data.will_be_moved()) {
            SCOPES_EXPECT_ERROR(error_value_moved(arg.value, data.get_mover()));
        }
        data.move(mover, state.block.depth);
        return {};
    }

    int get_value_depth(Value *value) {
        if (isa<Parameter>(value)) {
            auto param = cast<Parameter>(value);
            assert(param->owner);
            if (isa<Function>(param->owner)) {
                assert(param->owner == function);
                // outside of function
                return 0;
            } else if (isa<Instruction>(param->owner)) {
                auto instr = cast<Instruction>(param->owner);
                assert(instr->block);
                return instr->block->depth;
            } else {
                assert(false);
                return 0;
            }
        } else if (isa<Instruction>(value)) {
            if (value->is_pure())
                return 0;
            auto instr = cast<Instruction>(value);
            assert(instr->block);
            return instr->block->depth;
        }
        return 0;
    }

    SCOPES_RESULT(void) track_return_argument(State &state, Value *instr, Value *node,
        int retdepth) {
        SCOPES_RESULT_TYPE(void);
        assert(retdepth >= 0);
        assert(node);
        auto T = strip_qualifiers(node->get_type());
        if (!is_returning(T))
            return {};
        State *s = state.parent;
        std::unordered_set<ValueIndex, ValueIndex::Hash> moved;
        while (s && s->block.depth > retdepth) {
            //StyledStream ss;
            //ss << s->block.depth << " " << retdepth << std::endl;
            for (auto &entries : s->_data) {
                const ValueIndex &key = entries.first;
                Data &data = entries.second;
                if (data.will_be_moved()
                    && data.is_born()
                    && !moved.count(key)
                    && (data.get_move_depth() > retdepth)) {
                    // data will be moved, so we need to write a destructor
                    auto T = strip_qualifiers(key.value->get_type());
                    auto argT = get_argument(T, key.index);
                    SCOPES_CHECK_RESULT(write_destructor(state, instr, argT, key));
                    moved.insert(key);
                }
            }
            s = s->parent;
        }
        int depth = get_value_depth(node);
        if (depth <= retdepth) {
            // values exist outside of block, mark them as surviving
            int argc = get_argument_count(T);
            for (int i = 0; i < argc; ++i) {
                auto argT = get_argument(T, i);
                if (!is_tracked(argT))
                    continue;
                auto arg = ValueIndex(node, i);
                auto &data = state.get_data(arg);
                data.get_viewers().insert(instr);
            }
        } else {
            // values must be moved
            int argc = get_argument_count(T);
            for (int i = 0; i < argc; ++i) {
                auto argT = get_argument(T, i);
                if (!is_tracked(argT))
                    continue;
                SCOPES_CHECK_RESULT(move_argument(state, instr, ValueIndex(node, i)));
            }
        }
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
                    move_argument(state, node, ValueIndex(node->args[0])));
                return {};
            } break;
            default: break;
            }
        }
        SCOPES_CHECK_RESULT(track_arguments(state, node, node->args));
        return track_argument(state, node, node->callee);
    }
    SCOPES_RESULT(void) track_Loop(State &state, Loop *node) {
        SCOPES_RESULT_TYPE(void);
        State loop_state(node->body, state);
        SCOPES_CHECK_RESULT(track_block(loop_state, node, node->value));
        return track_argument(state, node, node->init);
    }
    SCOPES_RESULT(void) track_Break(State &state, Break *node) {
        return track_argument(state, node, node->value);
    }
    SCOPES_RESULT(void) track_Repeat(State &state, Repeat *node) {
        return track_argument(state, node, node->value);
    }
    SCOPES_RESULT(void) track_Return(State &state, Return *node) {
        return track_return_argument(state, node, node->value, 0);
    }
    SCOPES_RESULT(void) track_Label(State &state, Label *node) {
        SCOPES_RESULT_TYPE(void);
        State label_state(node->body, state);
        return track_block(label_state, node, node->value);
    }
    SCOPES_RESULT(void) track_Merge(State &state, Merge *node) {
        SCOPES_RESULT_TYPE(void);
        return track_argument(state, node, node->value);
    }
    SCOPES_RESULT(void) track_Raise(State &state, Raise *node) {
        return track_argument(state, node, node->value);
    }
    SCOPES_RESULT(void) track_ArgumentList(State &state, ArgumentList *node) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = node->get_type();
        int argc = get_argument_count(T);
        for (int i = 0; i < argc; ++i) {
            auto argT = get_argument(T, i);
            if (!is_tracked(argT))
                continue;
            auto &data = state.get_data(ValueIndex(node, i));
            auto arg = node->values[i];
            if (!data.will_be_used()) {
                SCOPES_CHECK_RESULT(track_subargument(state, node,
                    argT, ValueIndex(arg)));
            } else {
                if (data.will_be_moved()) {
                    SCOPES_CHECK_RESULT(move_argument(state, node, ValueIndex(arg)));
                }
                if (data.will_be_viewed()) {
                    auto &argdata = state.get_data(ValueIndex(arg));
                    auto &argviewers = argdata.get_viewers();
                    for (auto user : data.get_viewers()) {
                        argviewers.insert(user);
                    }
                }
            }
        }
        return {};

        return track_arguments(state, node, node->values);
    }
    SCOPES_RESULT(void) track_ExtractArgument(State &state, ExtractArgument *node) {
        return track_argument(state, node, node->value);
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

    SCOPES_RESULT(void) track_block(State &state, Value *instr, Value *return_value) {
        SCOPES_RESULT_TYPE(void);
        if (return_value) {
            SCOPES_CHECK_RESULT(track_return_argument(
                state, instr, return_value, state.block.depth - 1));
        }
        auto &&block = state.block;
        if (block.terminator) {
            auto val = block.terminator;
            block.insert_at_end();
            SCOPES_CHECK_RESULT(track_instruction(state, instr, val));
            /*
            auto T = strip_qualifiers(val->get_type());
            auto argc = get_argument_count(T);
            for (int i = 0; i < argc; ++i) {
                auto &subval = get_data(ValueIndex(val, i));

            }
            */
        }
        auto &&body = block.body;
        int i = body.size();
        while (i-- > 0) {
            block.insert_at(i + 1);
            SCOPES_CHECK_RESULT(track_instruction(state, instr, body[i]));
        }
        return {};
    }

    SCOPES_RESULT(void) process() {
        SCOPES_RESULT_TYPE(void);
        State root_state(function->body);
        SCOPES_CHECK_RESULT(track_block(root_state, function, function->value));
        return {};
    }

    SCOPES_RESULT(void) write_destructor(State &state, Value *instr,
        const Type *argT, const ValueIndex &arg) {
        SCOPES_RESULT_TYPE(void);
        // generate destructor
        Value *handler;
        if (!argT->lookup(SYM_DropHandler, handler))
            return {};
        StyledStream ss;
        ss << instr->anchor() << " last visit: " << arg.index << " ";
        stream_ast(ss, arg.value, StreamASTFormat::singleline());
        ss << " in ";
        stream_ast(ss, instr, StreamASTFormat::singleline());
        ss << std::endl;
        auto expr =
            Call::from(instr->anchor(), handler, {
                ExtractArgument::from(instr->anchor(), arg.value, arg.index) });
        SCOPES_CHECK_RESULT(
            prove(ctx.with_block(state.block).with_symbol_target(), expr));
        return {};
    }

    SCOPES_RESULT(void) track_subargument(State &state, Value *instr,
        const Type *argT, const ValueIndex &arg) {
        SCOPES_RESULT_TYPE(void);
        auto &data = state.get_data(arg);
        if (data.will_be_used())
            return {};
        SCOPES_CHECK_RESULT(
            move_argument(state, instr, arg));
        return write_destructor(state, instr, argT, arg);
    }

    SCOPES_RESULT(void) track_argument(State &state, Value *instr, Value *node) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = strip_qualifiers(node->get_type());
        if (!is_returning(T))
            return {};
        int argc = get_argument_count(T);
        for (int i = 0; i < argc; ++i) {
            auto argT = get_argument(T, i);
            if (!is_tracked(argT))
                continue;
            SCOPES_CHECK_RESULT(track_subargument(state, instr,
                argT, ValueIndex(node, i)));
        }
        return {};
    }

    SCOPES_RESULT(void) track_declaration(State &state, Value *instr, Value *node) {
        SCOPES_RESULT_TYPE(void);
        assert(node);
        auto T = strip_qualifiers(node->get_type());
        if (!is_returning(T))
            return {};
        int argc = get_argument_count(T);
        for (int i = 0; i < argc; ++i) {
            auto argT = get_argument(T, i);
            if (!is_tracked(argT))
                continue;
            auto arg = ValueIndex(node, i);
            auto &data = state.get_data(arg);
            if (!data.will_be_used()) {
                SCOPES_CHECK_RESULT(move_argument(state, instr, arg));
                SCOPES_CHECK_RESULT(write_destructor(state, instr, argT, arg));
            }
            data.set_unborn();
        }
        return {};
    }

    SCOPES_RESULT(void) track_arguments(State &state, Value *instr, const Values &args) {
        SCOPES_RESULT_TYPE(void);
        int i = args.size();
        while (i-- > 0) {
            SCOPES_CHECK_RESULT(track_argument(state, instr, args[i]));
        }
        return {};
    }

    SCOPES_RESULT(void) track_instruction(State &state, Value *instr, Value *node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(track_declaration(state, instr, node));
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

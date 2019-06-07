/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "lifetime.hpp"
#include "value.hpp"
#include "prover.hpp"
#include "error.hpp"

namespace scopes {

#define HANDLER(CLASS) \
    SCOPES_RESULT(void) tag_ ## CLASS(const ASTContext &ctx, const CLASS ## Ref &node)

HANDLER(Merge) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Repeat) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Return) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Raise) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Unreachable) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Discard) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Label) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(LoopLabel) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(CondBr) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Switch) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Call) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Select) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ExtractValue) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(InsertValue) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(GetElementPtr) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ExtractElement) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(InsertElement) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ShuffleVector) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Alloca) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Malloc) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Free) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Load) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Store) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ICmp) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(FCmp) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(UnOp) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(BinOp) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(TriOp) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Annotate) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Sample) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageQuerySize) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageQueryLod) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageQueryLevels) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageQuerySamples) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageRead) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ImageWrite) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(ExecutionMode) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}
HANDLER(Cast) {
    //SCOPES_RESULT_TYPE(void);
    return {};
}

SCOPES_RESULT(void) tag_instruction(const ASTContext &ctx, const InstructionRef &node) {
    SCOPES_RESULT_TYPE(void);
    switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME: SCOPES_CHECK_RESULT(tag_ ## CLASS(ctx, node.cast<CLASS>())); break;
    SCOPES_INSTRUCTION_VALUE_KIND()
#undef T
    default: assert(false);
    }
    return {};
}

#undef HANDLER

} // namespace scopes

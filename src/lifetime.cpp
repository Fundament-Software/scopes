/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "lifetime.hpp"

namespace scopes {

#if 0
SCOPES_RESULT(void) tag_instruction(const InstructionRef &node) {
    SCOPES_RESULT_TYPE(void);
    switch(node->kind()) {
#define T(NAME, BNAME, CLASS) \
    case NAME: result = SCOPES_GET_RESULT(tag_ ## CLASS(ctx, node.cast<CLASS>())); break;
    SCOPES_INSTRUCTION_VALUE_KIND()
#undef T
    default: assert(false);
    }
    return {};
}
#endif

} // namespace scopes

/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SPECIALIZER_HPP
#define SCOPES_SPECIALIZER_HPP

#include "type.hpp"
#include "result.hpp"

namespace scopes {

struct Frame;
struct Label;

SCOPES_RESULT(Label *) specialize(Frame *frame, Label *label, const ArgTypes &argtypes);
void enable_specializer_step_debugger();

} // namespace scopes

#endif // SCOPES_SPECIALIZER_HPP
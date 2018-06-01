/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_PROFILER_HPP
#define SCOPES_PROFILER_HPP

namespace scopes {

struct Label;

void on_label_specialized(Label *l);
void print_profiler_info();

} // namespace scopes

#endif // SCOPES_PROFILER_HPP
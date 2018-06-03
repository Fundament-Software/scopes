/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_BOOT_HPP
#define SCOPES_BOOT_HPP

namespace scopes {

void on_shutdown();

extern bool signal_abort;
void f_abort();
void f_exit(int c);

} // namespace scopes

#endif // SCOPES_BOOT_HPP

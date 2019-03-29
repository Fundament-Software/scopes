/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_BOOT_HPP
#define SCOPES_BOOT_HPP

#include "valueref.inc"
#include "result.hpp"

namespace scopes {

void on_startup();
void on_shutdown();

extern bool signal_abort;
void f_abort();
void f_exit(int c);

SCOPES_RESULT(ValueRef) load_custom_core(const char *executable_path);

int run_main(const char *exepath, int argc, char *argv[]);

} // namespace scopes

#endif // SCOPES_BOOT_HPP

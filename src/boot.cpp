/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "boot.hpp"
#include "scopes.h"

#ifdef SCOPES_WIN32
#include "stdlib_ex.h"
#include "dlfcn.h"
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <unistd.h>
#include <libgen.h>
#include <cstdlib>

namespace scopes {

void on_shutdown() {
#if SCOPES_PRINT_TIMERS
    print_profiler_info();
    Timer::print_timers();
    std::cerr << "largest recorded stack size: " << g_largest_stack_size << std::endl;
#endif
}

bool signal_abort = false;
void f_abort() {
    on_shutdown();
    if (SCOPES_EARLY_ABORT || signal_abort) {
        std::abort();
    } else {
        exit(1);
    }
}


void f_exit(int c) {
    on_shutdown();
    exit(c);
}

} // namespace scopes

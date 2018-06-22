/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "boot.hpp"
#include "timer.hpp"
#include "gc.hpp"
#include "scopes/config.h"

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

static Timer *main_compile_time = nullptr;
void on_startup() {
    main_compile_time = new Timer(TIMER_Main);
}

void on_shutdown() {
    delete main_compile_time;
#if SCOPES_PRINT_TIMERS
    //print_profiler_info();
    Timer::print_timers();
    StyledStream ss(SCOPES_CERR);
    ss << "largest recorded stack size: " << g_largest_stack_size << std::endl;
#endif
}

bool signal_abort = false;
void f_abort() {
    on_shutdown();
    if (signal_abort) {
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

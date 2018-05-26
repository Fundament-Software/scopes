/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "main.hpp"
#include "timer.hpp"
#include "gc.hpp"

#include "scopes.h"

namespace scopes {

void on_shutdown() {
#if SCOPES_PRINT_TIMERS
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

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

#include "timer.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// TIMER
//------------------------------------------------------------------------------

Timer::Timer(Symbol _name) : name(_name), start(std::chrono::high_resolution_clock::now()) {}
Timer::~Timer() {
    std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
    timers[name] = timers[name] + (diff.count() * 1000.0);
}

void Timer::print_timers() {
    StyledStream ss;
    for (auto it = timers.begin(); it != timers.end(); ++it) {
        ss << it->first.name()->data << ": " << it->second << "ms" << std::endl;
    }
}

std::unordered_map<Symbol, double, Symbol::Hash> Timer::timers;

} // namespace scopes

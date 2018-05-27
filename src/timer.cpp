/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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

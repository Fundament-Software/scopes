/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "timer.hpp"

#include <unordered_map>

namespace scopes {

struct TimerData {
    double time;
    int refcount;

    TimerData() : time(0.0), refcount(0) {}
};

static std::unordered_map<Symbol, TimerData, Symbol::Hash> timers;

//------------------------------------------------------------------------------
// TIMER
//------------------------------------------------------------------------------

Timer::Timer(Symbol _name) : name(_name), start(std::chrono::high_resolution_clock::now()) {
    auto &&data = timers[name];
    data.refcount++;
}
Timer::~Timer() {
    std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
    auto &&data = timers[name];
    data.refcount--;
    if (!data.refcount) {
        data.time += (diff.count() * 1000.0);
    }
}

void Timer::print_timers() {
    StyledStream ss;
    for (auto &&it : timers) {
        ss << it.first.name()->data << ": " << it.second.time << "ms" << std::endl;
    }
}

} // namespace scopes

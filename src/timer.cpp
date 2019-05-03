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

    TimerData() : time(0.0) {}
};

static std::unordered_map<Symbol, TimerData, Symbol::Hash> timers;

//------------------------------------------------------------------------------
// TIMER
//------------------------------------------------------------------------------

static Timer *active_timer = nullptr;
static Timer unknown_timer(TIMER_Unknown);

void Timer::pause() {
    std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
    auto &&data = timers[name];
    data.time += (diff.count() * 1000.0);
}

void Timer::resume() {
    start = std::chrono::high_resolution_clock::now();
}

Timer::Timer(Symbol _name) : prev_timer(active_timer), name(_name) {
    if (active_timer)
        active_timer->pause();
    active_timer = this;
    resume();
}
Timer::~Timer() {
    pause();
    active_timer = prev_timer;
    if (active_timer)
        active_timer->resume();
}

void Timer::print_timers() {
    StyledStream ss;
    double real_sum = 0.0;
    double non_user_sum = timers[TIMER_Main].time;
    for (auto &&it : timers) {
        ss << it.first.name()->data << ": " << it.second.time << "ms" << std::endl;
        real_sum += it.second.time;
    }
    ss << "cumulative real: " << real_sum << "ms" << std::endl;
    ss << "cumulative user: " << (real_sum - non_user_sum) << "ms" << std::endl;
}

} // namespace scopes

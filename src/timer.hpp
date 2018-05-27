/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TIMER_HPP
#define SCOPES_TIMER_HPP

#include "symbol.hpp"

#include <chrono>

namespace scopes {

//------------------------------------------------------------------------------
// TIMER
//------------------------------------------------------------------------------

struct Timer {
    static std::unordered_map<Symbol, double, Symbol::Hash> timers;
    Symbol name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;

    Timer(Symbol _name);
    ~Timer();

    static void print_timers();
};

} // namespace scopes

#endif // SCOPES_TIMER_HPP
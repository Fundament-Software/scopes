/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "body.hpp"

namespace scopes {

Body::Body() :
    anchor(nullptr), enter(none), flags(0), scope_label(nullptr) {}

bool Body::is_complete() const {
    return flags & LBF_Complete;
}
void Body::set_complete() {
    flags |= LBF_Complete;
}
void Body::unset_complete() {
    flags &= ~LBF_Complete;
}

bool Body::is_optimized() const {
    return flags & LBF_Optimized;
}

void Body::set_optimized() {
    flags |= LBF_Optimized;
}

void Body::unset_optimized() {
    flags &= ~LBF_Optimized;
}

bool Body::is_rawcall() {
    return (flags & LBF_RawCall) == LBF_RawCall;
}

void Body::set_rawcall(bool enable) {
    if (enable) {
        flags |= LBF_RawCall;
    } else {
        flags &= ~LBF_RawCall;
    }
}

bool Body::is_rawcont() {
    return (flags & LBF_RawCont) == LBF_RawCont;
}

void Body::set_rawcont(bool enable) {
    if (enable) {
        flags |= LBF_RawCont;
    } else {
        flags &= ~LBF_RawCont;
    }
}

bool Body::is_trycall() {
    return (flags & LBF_TryCall) == LBF_TryCall;
}

void Body::set_trycall(bool enable) {
    if (enable) {
        flags |= LBF_TryCall;
    } else {
        flags &= ~LBF_TryCall;
    }
}

void Body::copy_traits_from(const Body &other) {
    flags = other.flags;
    anchor = other.anchor;
    scope_label = other.scope_label;
}


} // namespace scopes

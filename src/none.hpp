/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_NONE_HPP
#define SCOPES_NONE_HPP

namespace scopes {

//------------------------------------------------------------------------------
// NONE
//------------------------------------------------------------------------------

struct Nothing {
};

extern Nothing none;

struct StyledStream;
StyledStream& operator<<(StyledStream& ost, const Nothing &value);

} // namespace scopes

#endif // SCOPES_NONE_HPP
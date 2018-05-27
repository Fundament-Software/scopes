/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_DYN_CAST_INC
#define SCOPES_DYN_CAST_INC

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

}

#endif // SCOPES_DYN_CAST_INC
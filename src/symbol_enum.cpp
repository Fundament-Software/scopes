/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "symbol_enum.hpp"
#include "symbol_enum.inc"

namespace scopes {

const char *get_known_symbol_name(KnownSymbol sym) {
    switch(sym) {
#define T(SYM, NAME) case SYM: return #SYM;
#define T0 T
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    case FN_ ## UNAME ## PFIX: return "FN_" #UNAME #PFIX;
    SCOPES_SYMBOLS()
#undef T
#undef T0
#undef T1
#undef T2
#undef T2T

#define T(NAME) \
case SYM_SPIRV_StorageClass ## NAME: return "SYM_SPIRV_StorageClass" #NAME;
B_SPIRV_STORAGE_CLASS()
#undef T
#define T(NAME) \
    case SYM_SPIRV_BuiltIn ## NAME: return "SYM_SPIRV_BuiltIn" #NAME;
B_SPIRV_BUILTINS()
#undef T
#define T(NAME) \
case SYM_SPIRV_ExecutionMode ## NAME: return "SYM_SPIRV_ExecutionMode" #NAME;
B_SPIRV_EXECUTION_MODE()
#undef T
#define T(NAME) \
    case SYM_SPIRV_Dim ## NAME: return "SYM_SPIRV_Dim" #NAME;
B_SPIRV_DIM()
#undef T
#define T(NAME) \
    case SYM_SPIRV_ImageFormat ## NAME: return "SYM_SPIRV_ImageFormat" #NAME;
B_SPIRV_IMAGE_FORMAT()
#undef T
#define T(NAME) \
    case SYM_SPIRV_ImageOperand ## NAME: return "SYM_SPIRV_ImageOperand" #NAME;
B_SPIRV_IMAGE_OPERAND()
#undef T
case SYM_Count: return "SYM_Count";
    }
    return "?";
}

} // namespace scopes
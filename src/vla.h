/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VLA_H
#define SCOPES_VLA_H

#include <malloc.h>

#ifndef _MSC_VER
#define VLA(type, name, len) type name[len]
#else
#define VLA(type, name, len) type* name = (type*)alloca(sizeof(type) * (len))
#endif

#endif
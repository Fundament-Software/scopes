/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#undef SCOPESRT_IMPL
#include "scopes/scopes.h"

void *get_executable_function_pointer() {
  return (void*) (intptr_t) get_executable_function_pointer;
}

int main(int argc, char *argv[]) {
    return sc_main(get_executable_function_pointer(), argc, argv);
}

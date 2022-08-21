vvv bind lib
include
    """"#include "stdio.h"
        
        FILE* __stdout_scopes__() { return stdout; }
        FILE* __stdin_scopes__() { return stdin; }
        FILE* __stderr_scopes__() { return stderr; }

let windows? = (operating-system == 'windows)
do
    using lib.extern #filter "^(str(.+))$"

    # todo: fix this export up beyond the bare essentials

    let printf =
        static-if windows? lib.extern.printf_s
        else lib.extern.printf

    let stdout = (lib.extern.__stdout_scopes__)
    let stdin = (lib.extern.__stdin_scopes__)
    let stderr = (lib.extern.__stderr_scopes__)
    locals;

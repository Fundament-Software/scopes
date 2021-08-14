vvv bind lib
include
    """"#include "stdio.h"

let windows? = (operating-system == 'windows)
do
    using lib.extern #filter "^(str(.+))$"

    # todo: fix this export up beyond the bare essentials

    let printf =
        static-if windows? lib.extern.printf_s
        else lib.extern.printf

    locals;

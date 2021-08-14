vvv bind lib
include
    """"#include "stdlib.h"

#let windows? = (operating-system == 'windows)
do
    using lib.extern #filter "^(str(.+))$"

    locals;

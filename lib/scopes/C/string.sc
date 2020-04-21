vvv bind lib
include
    """"#include "string.h"

do
    #using lib.define filter "^(AF_(.+)|PF_(.+)|SOCK_(.+)|IPPROTO_(.+)|INADDR_(.+)|SHUT_(.+))$"
    #using lib.const filter "^(INADDR_(.+))$"
    #using lib.typedef filter "^(sockaddr)$"
    #using lib.struct filter "^(sockaddr|sockaddr_(.+))$"
    using lib.extern filter "^(str(.+))$"

    locals;

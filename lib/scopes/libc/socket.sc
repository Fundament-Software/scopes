vvv bind lib
include
    """"#include "sys/socket.h"
        #include "netinet/in.h"
        #include "arpa/inet.h"
        #include "unistd.h"

do
    using lib.define filter "^(AF_(.+)|PF_(.+)|SOCK_(.+)|IPPROTO_(.+)|INADDR_(.+)|SHUT_(.+))$"
    #using lib.const filter "^(INADDR_(.+))$"
    #using lib.typedef filter "^(sockaddr)$"
    using lib.struct filter "^(sockaddr|sockaddr_(.+))$"
    using lib.extern filter "^(socket|bind|listen|accept|connect|htons|htonl|close|read|recv|write|shutdown|setsockopt|inet_(.+))$"

    let
        INADDR_ANY       = 0x00000000:u32
        INADDR_BROADCAST = 0xffffffff:u32

    locals;

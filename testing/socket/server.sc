
using import libc.socket

let perror = (extern 'perror (function void rawstring))

fn main ()
    let socketfd =
        socket PF_INET
            SOCK_STREAM as integer
            IPPROTO_TCP as integer
    if (socketfd < 0)
        error "cannot create socket"
    defer
        inline ()
            print "closing socket"
            close socketfd

    local sa =
        sockaddr_in
            sin_family = AF_INET
            sin_port = (htons 1104)
            sin_addr =
                typeinit
                    s_addr = (htonl INADDR_ANY)

    if ((bind socketfd (bitcast &sa (pointer sockaddr)) (sizeof sa)) == -1)
        error "bind failed"

    if ((listen socketfd 10) < 0)
        error "listen failed"

    let connectfd =
        accept socketfd null null
    if (connectfd < 0)
        error "accept failed"
    print "accepted connection"
    defer
        inline ()
            print "closing connection"
            close connectfd

    # perform read write operations
    let buf = (alloca-array i8 256)
    let r = (read connectfd buf 256)
    if (r > 0)
        print (string buf (r as usize))
    let msg = "hello from server"
    write connectfd (msg as rawstring) (countof msg)

    if ((shutdown connectfd (SHUT_RDWR as integer)) < 0)
        error "shutdown failed"

main;



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
    let res =
        inet_pton AF_INET "127.0.0.1" &sa.sin_addr

    if ((connect socketfd (bitcast &sa (pointer sockaddr)) (sizeof sa)) == -1)
        error "connect failed"

    # perform read write operations
    let msg = "hello from client"
    write socketfd (msg as rawstring) (countof msg)
    let buf = (alloca-array i8 256)
    let r = (read socketfd buf 256)
    if (r > 0)
        print (string buf (r as usize))

    shutdown socketfd (SHUT_RDWR as integer)

main;

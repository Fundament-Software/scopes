
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

#
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>

    int main(void)
    {
        struct sockaddr_in sa;
        int res;
        int SocketFD;

        SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (SocketFD == -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
        }

        memset(&sa, 0, sizeof sa);

        sa.sin_family = AF_INET;
        sa.sin_port = htons(1100);
        res = inet_pton(AF_INET, "192.168.1.3", &sa.sin_addr);

        if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
        }

        /* perform read write operations ... */

        shutdown(SocketFD, SHUT_RDWR);

        close(SocketFD);
        return EXIT_SUCCESS;
    }

using
    import-c "tty.c" "
    #include <sys/ioctl.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    " '()

fn window-size ()
    var w = (winsize)
    ioctl STDOUT_FILENO TIOCGWINSZ w
    return
        i32 w.ws_row
        i32 w.ws_col

var terminal-settings = (termios)
fn init-terminal ()
    var d = (termios)
    tcgetattr STDOUT_FILENO d
    terminal-settings = (load d)
    cfmakeraw d
    tcsetattr STDOUT_FILENO TCSAFLUSH d
    fcntl STDIN_FILENO F_SETFL
        (fcntl STDIN_FILENO F_GETFL) | O_NONBLOCK

fn exit-terminal ()
    tcsetattr STDOUT_FILENO TCSAFLUSH terminal-settings

fn reset ()
    printf "\x1b[0m"

fn fgcolor (r g b)
    printf "\x1b[38;2;%i;%i;%im"
        i32 r; i32 g; i32 b

fn bgcolor (r g b)
    printf "\x1b[48;2;%i;%i;%im"
        i32 r; i32 g; i32 b

fn save-cursor ()
    printf "\x1b7"

fn restore-cursor ()
    printf "\x1b8"

fn locate (row column)
    printf "\x1b[%i;%iH" (i32 row) (i32 column)

fn scroll-up (n)
    printf "\x1b[%iS" (i32 n)

fn scroll-down (n)
    printf "\x1b[3T" (i32 n)

fn clear-screen ()
    printf "\x1b[2J"
fn clear-below ()
    printf "\x1b[0J"
fn clear-above ()
    printf "\x1b[1J"
fn clear-right ()
    printf "\x1b[0K"
fn clear-left ()
    printf "\x1b[1K"
fn clear-line ()
    printf "\x1b[2K"

fn cursor-on ()
    printf "\x1b[?25h"
fn cursor-off ()
    printf "\x1b[?25l"

fn mouse-on ()
    printf "\x1b[?1003h"
fn mouse-off ()
    printf "\x1b[?1003l"

fn hexrgb (code)
    return
        (code >> 16) & 0xff
        (code >> 8) & 0xff
        code & 0xff

init-terminal;
do
    locate 1 1
    bgcolor
        hexrgb 0x202020
    clear-screen;
    mouse-on;
    cursor-off;
    fgcolor
        hexrgb 0xa0a0a0

    printf "test\r\n"

    printf "console size: %i %i\r\n"
        window-size;

    print "done."
    while true
        let c =
            getchar;

        if (c == 0x1b)
            let c = (getchar)
            if (c != 0x5b)
                break;
            let c = (getchar)
            locate 1 1
            if (c == 0x4d)
                let s y x =
                    (getchar) - 32
                    (getchar) - 32
                    (getchar) - 32
                let shiftdown = ((s & 4) == 4)
                let altdown = ((s & 8) == 8)
                let ctrldown = ((s & 16) == 16)
                let dragging = ((s & 32) == 32)
                let scrolling = ((s & 64) == 64)
                let s = (s ^ (s & 124))
                printf "button=%i %i %i %s%s%s%s%s" s x y
                    (? shiftdown "shift " "") as rawstring
                    (? altdown "alt " "") as rawstring
                    (? ctrldown "ctrl " "") as rawstring
                    (? dragging "drag " "") as rawstring
                    (? scrolling "scroll" "") as rawstring
                clear-right;
                locate x y
            else
                while true
                    let c =
                        getchar;
                    if (c == -1)
                        break;
                    printf "%i " c
                clear-right;
        elseif (c == -1)
        else
            locate 1 1
            printf "%i " c
            while true
                let c =
                    getchar;
                if (c == -1)
                    break;
                printf "%i " c
            clear-right;


reset;
mouse-off;
clear-screen;
locate 1 1
cursor-on;
exit-terminal;


using
    import-c "tty.c" "
    #include <sys/ioctl.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <signal.h>
    " '()

let tty-file =
    fopen "/dev/tty" "w"
setvbuf tty-file null _IONBF 0
let tty-fd =
    fileno tty-file
freopen "test_tty.log" "a+" stdout
setvbuf stdout null _IONBF 0
freopen "test_tty.log" "a+" stderr
setvbuf stderr null _IONBF 0

fn countof-utf8str (s)
    "counts the codepoints of any value that is convertible to a
     zero-terminated rawstring in UTF-8 encoding"
    let s = (s as rawstring)
    let loop (i j) = 0 0
    if ((s @ i) != 0:i8)
        loop (i + 1)
            ? (((s @ i) & 0xc0:i8) != 0x80:i8) (j + 1) j
    else
        j

fn writescr (...)
    fprintf tty-file ...

fn window-size ()
    var w = (winsize)
    ioctl tty-fd TIOCGWINSZ w
    return
        i32 w.ws_col
        i32 w.ws_row

var terminal-settings = (termios)
fn init-terminal ()
    var d = (termios)
    tcgetattr tty-fd d
    terminal-settings = (load d)
    cfmakeraw d
    tcsetattr tty-fd TCSAFLUSH d
    fcntl STDIN_FILENO F_SETFL
        (fcntl STDIN_FILENO F_GETFL) | O_NONBLOCK

fn exit-terminal ()
    tcsetattr tty-fd TCSAFLUSH terminal-settings

fn reset ()
    writescr "\x1b[0m"

fn fgcolor (r g b)
    writescr "\x1b[38;2;%i;%i;%im"
        i32 r; i32 g; i32 b

fn bgcolor (r g b)
    writescr "\x1b[48;2;%i;%i;%im"
        i32 r; i32 g; i32 b

fn save-cursor ()
    writescr "\x1b7"

fn restore-cursor ()
    writescr "\x1b8"

fn locate (row column)
    writescr "\x1b[%i;%iH" (i32 row) (i32 column)

fn scroll-up (n)
    writescr "\x1b[%iS" (i32 n)

fn scroll-down (n)
    writescr "\x1b[%iT" (i32 n)

fn scroll-region (a1 a2)
    writescr "\x1b[%i;%ir" (i32 a1) (i32 a2)

fn clear-screen ()
    writescr "\x1b[2J"
fn clear-below ()
    writescr "\x1b[0J"
fn clear-above ()
    writescr "\x1b[1J"
fn clear-right ()
    writescr "\x1b[0K"
fn clear-left ()
    writescr "\x1b[1K"
fn clear-line ()
    writescr "\x1b[2K"

fn cursor-on ()
    writescr "\x1b[?25h"
fn cursor-off ()
    writescr "\x1b[?25l"

fn mouse-on ()
    writescr "\x1b[?1003h"
fn mouse-off ()
    writescr "\x1b[?1003l"


fn hexrgb (code)
    return
        (code >> 16) & 0xff
        (code >> 8) & 0xff
        code & 0xff

init-terminal;

global window-size-changed = false
fn on-sigwinch (k)
    window-size-changed = true
    return;

do
    signal SIGWINCH on-sigwinch

    let w h = (window-size)
    printf "window size: %i x %i\n"
        i32 w; i32 h
    var width = w
    var height = h
    var frame = 0

    fn reset-screen ()
        locate 1 1
        bgcolor
            hexrgb 0x202020
        clear-screen;
        mouse-on;
        cursor-off;
        fgcolor
            hexrgb 0xa0a0a0

    fn on-idle ()
        usleep (1000 * 16)
        if (load window-size-changed)
            let w h = (window-size)
            window-size-changed = false
            width = w
            height = h
            reset-screen;
        locate 2 1
        writescr "console size: %i x %i" (i32 width) (i32 height)
        locate 3 1
        writescr "frame: %i" (i32 frame)
        #printf "frame: %i\n" (i32 frame)
        clear-right;
        frame = frame + 1

    reset-screen;
    while true
        let c =
            getchar;

        if (c == 0x1b)
            let c = (getchar)
            if (c != 0x5b)
                break;
            let c = (getchar)
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
                printf "button=%i %i %i %s%s%s%s%s\n" s x y
                    (? shiftdown "shift " "") as rawstring
                    (? altdown "alt " "") as rawstring
                    (? ctrldown "ctrl " "") as rawstring
                    (? dragging "drag " "") as rawstring
                    (? scrolling "scroll" "") as rawstring
            else
                while true
                    let c =
                        getchar;
                    if (c == -1)
                        break;
                    printf "^ %i " c
                printf "\n"
        elseif (c == -1)
            on-idle;
        else
            printf "key %i " c
            while true
                let c =
                    getchar;
                if (c == -1)
                    break;
                printf "%i " c
            printf "\n"


reset;
mouse-off;
clear-screen;
locate 1 1
cursor-on;
exit-terminal;

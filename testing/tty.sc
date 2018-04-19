
using
    import-c "tty.c"
        """"
            #include <sys/ioctl.h>
            #include <stdio.h>
            #include <unistd.h>
            #include <termios.h>
            #include <fcntl.h>
            #include <signal.h>
        '()

let tty-file =
    fopen "/dev/tty" "w"
setvbuf tty-file null _IONBF 0
let tty-fd =
    fileno tty-file
freopen "test_tty.log" "a+" stdout
setvbuf stdout null _IONBF 0
freopen "test_tty.log" "a+" stderr
setvbuf stderr null _IONBF 0

# reserve enough space to serve a 6x6 font at 3840 pixels × 2160, which is 640x360
let MAX_WIDTH = 640:usize
let MAX_HEIGHT = 360:usize
# page 0: character, page 1: character color, page 2: background color
let NUM_PAGES = 3:usize
let NUM_BUFFERS = 2:usize

fn PageType (T)
    array
        array T
            usize MAX_WIDTH
        usize MAX_HEIGHT

fn hexrgb (code)
    return
        (code >> 16) & 0xff
        (code >> 8) & 0xff
        code & 0xff

fn rgbhex (r g b)
    | r
        g << 8
        b << 16

struct Framebuffer
    glyph : (PageType (array i8 4:usize))
    fg : (PageType i32)
    bg : (PageType i32)

    method 'clear (self)
        let loop (y x) = (unconst 0) (unconst 0)
        if (y < MAX_HEIGHT)
            if (x < MAX_WIDTH)
                self.glyph @ y @ x = (arrayof i8 32 0 0 0)
                self.fg @ y @ x = 0xffffff
                self.bg @ y @ x = (rgbhex (i32 x) (i32 y) 0)
                loop y (x + 1)
            else
                loop (y + 1) (unconst 0)

let buffers =
    (malloc (array Framebuffer (usize NUM_BUFFERS))) @ 0

for i in (range 2)
    'clear (buffers @ i)

fn countof-utf8str (s)
    """"counts the codepoints of any value that is convertible to a
        zero-terminated rawstring in UTF-8 encoding
    let s = (s as rawstring)
    let loop (i j) = 0 0
    if ((s @ i) != 0:i8)
        loop (i + 1)
            ? (((s @ i) & 0xc0:i8) != 0x80:i8) (j + 1) j
    else
        j

fn writech (x)
    fputc (i32 x) tty-file
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

fn render-buffer (b)
    let w h = (window-size)
    locate 1 1
    let loop (y x fg bg) = (unconst 0) (unconst 0) 0xffffff 0x000000
    if (y < h)
        if (x < w)
            let glyph p-fg p-bg =
                load (b.glyph @ y @ x)
                load (b.fg @ y @ x)
                load (b.bg @ y @ x)
            if (p-fg != fg)
                fgcolor (hexrgb p-fg)
            if (p-bg != bg)
                bgcolor (hexrgb p-bg)
            for i in (range 4)
                let ch = (glyph @ i)
                if (ch == 0:i8)
                    break
                writech ch
            loop y (x + 1) p-fg p-bg
        else
            loop (y + 1) (unconst 0) fg bg


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
        clear-screen;
        mouse-on;
        cursor-off;

    fn on-idle ()
        usleep (1000 * 16)
        if (load window-size-changed)
            let w h = (window-size)
            window-size-changed = false
            width = w
            height = h
            reset-screen;
        render-buffer (buffers @ 0)
        locate 2 1
        writescr "console size: %i x %i (aspect %f)" (i32 width) (i32 height) (width / height)
        locate 3 1
        writescr "frame: %i" (i32 frame)
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

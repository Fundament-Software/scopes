
using import testing

test (0xffff == 65535)

test (0b1111111111111111 == 65535)

test (-0b1111 == -15)

test ((typeof 0x111:u16) == u16)

do
    # string literal prefix

    inline prefix:<> (s)
        .. "<" s ">"

    test (<>"test" == "<test>")
    test (<>"test" == "<test>")

    let S =
        <>""""line1
              line2
              line3

    test (S == "<line1\nline2\nline3\n>")

    # string literal prefix does not escape
    inline prefix:r (s) s

    test (r"\\test\n\"test\"" == "\\\\test\\n\"test\"")

# \ followed by \n is a line continuation
test
    ==
        "the\
        quick\
        brown\
        \
        fox\
"
        "thequickbrownfox"

;

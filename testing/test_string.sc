using import testing

# raw string blocks
###################

# keeps first trailing LF
test
    "hello world\n" == """"hello world

# doesn't escape anything
test
    "hello \"world\"\\n\n" == """"hello "world"\n

# sub 4-space indentation is illegal
#test
    "hello world\nwhat's up\n" == """"hello world
                                     what's up

# all indentation up to 4-spaces is trimmed
test
    "hello world\nwhat's up\n" == """"hello world
                                      what's up

# empty first line isn't trimmed
test
    ==
        """"
            hi everyone
        "\nhi everyone\n"

# nested use of string block token has no effect
test
    ==
        "\"\"\"\"\n\"\"\"\"\n    \"\"\"\"\n"
        """"""""
            """"
                """"

# multiline block with indented lines
test
    ==
        """"first line
            second line

            third "line"
                fourth line

        # all string compares are done at runtime
        "first line\nsecond line\n\nthird \"line\"\n    fourth line\n"


<style type="text/css" rel="stylesheet">
body { counter-reset: chapter 5; }
</style>

Notation
========

Scopes source code is written in a notation that introduces syntactic rules
even before the first function is even written: *Scopes List Notation*,
abbreviated **SLN**.

Closely related to [S-Expressions](https://en.wikipedia.org/wiki/S-expression),
SLN can be regarded as a human-readable serialization format comparable to
YAML, XML or JSON. It has been optimized for simplicity and terseness.

SLN files do not have to contain valid Scopes source code on their own--they
are more likely to store configuration or metadata. As a result, the examples
in this document should be regarded as schema-free and containing arbitrary
data.

At a Glance
-----------

As a summary, here is an example that provides an overview of all
the notation aspects:

```scopes
# below is some random data without any schema

# a naked list of five 32-bit signed integers
1 2 3 4 5

# a list that begins with a symbol 'float-values:' and contains a braced
# sublist of floats.
float-values: (1.0 2.0 3.1 4.2 5.5:f64 inf nan)

# we can also nest the sublist using indentation
# note the extravagant heading, another context-free symbol.
==string-values==
    "A" "B" "NCC-1701\n" "\xFFD\xFF" "\"E\""

# a single top-level element, a single-line string
"I am Locutus of Borg."

# a raw block string; four double quotes mark the start
""""
    Ma'am is acceptable in a crunch, but I prefer Captain.
                                    -- Kathryn Janeway

# a list of pairs (also lists), arranged horizontally
(1 x) (2 y) (3 z)
# same list, with last two entries arranged vertically
(1 x)
    (2 y)
    (3 z)
# we can line up all entries by using a semicolon to indicate an empty head
;
    (1 x)
    (2 y)
    (3 z)
# parentheses can also be removed for each line entry
;
    1 x
    2 y
    3 z

# appending values to the parent list in the next line
symbol-values one two three four five \
    six seven-of-nine ten

# line continuation can also begin at the start of the next line
::typed-integers:: 0:u8 1:i8 2:i16 3:u16
    \ 4:u32 5:i32 6:u64 7:i64

# which comes in handy when we want to continue the parent list
people like
    jim kirk
    commander spock
    hikari sulu
    \ and many more

# a list with a symbol header and two entries
address-list
    # a list with a header and three more lists of two values each
    entry
        name: "Jean-Luc Picard"
        age: 59
        address: picard@enterprise.org
    entry
        # the semicolon acts as list separator
        name: "Worf, Son of Mogh"; age: 24; address: worf@house-of-mogh.co.klingon
    # line comments double as block comments
    #entry
        name: "Natasha Yar"
        age: 27
        address: natasha.yar@enterprise.org

# the same list with braced notation; within braced lists,
    indentation is meaningless.
(address-list
    # a list with a header and three more lists of two values each
    (entry
        (name: "Jean-Luc Picard")
        (age: 59)
        (address: picard@enterprise.org))
    (entry (name: "Worf, Son of Mogh") (age: 24)
        (address: worf@house-of-mogh.co.klingon)))

# a list of comma separated values - a comma is always recorded as
    a separate symbol, so the list has nine entries
1, 2, 3,4, 5

# a list of options beginning with a symbol in a list with
    square brace style
[task]
    cmd = "bash"
    # the last element is a symbol in a list with curly brace style
    working-dir = {project-base}
```


Formatting Rules
----------------

SLN files are always assumed to be encoded as UTF-8.

Whitespace controls scoping in the SLN format. Therefore, to avoid possible
ambiguities, SLN files must always use spaces, and one indentation level equals
four spaces.

Element Types
-------------

SLN recognizes only five kinds of elements:

* **Numbers**
* **Strings**
* **Symbols**
* **Lists**
* **Comments**

!!! note
    Comments are not part of the data structure.

### Comments ###

Both line and block comments are initiated with a single token, `#`. A comment
lasts from its beginning token to the first non-whitespace character with an equal
or lower indentation level. Some examples of valid comments:

```scopes
# a line comment
not a comment
# a block comment that continues
    in the next line because the line has
    a higher indentation level. Note, that
        comments do not need to respect
    indentation rules
but this line is not a comment
```

### Strings ###

Strings describe sequences of unsigned 8-bit characters in the range of 0-255.
A string begins and ends with `"` (double quotes).  The `\` escape character
can be used to include quotes in a string and describe unprintable control
characters such as `\n` (return) and `\t` (tab). Other unprintable
characters can be encoded via `\xNN`, where `NN` is the character's
hexadecimal code. Strings are parsed as-is, so UTF-8 encoded strings will be
copied over verbatim.

Here are some examples of valid strings:

```scopes
"a single-line string in double quotations"
"return: \n, tab: \t, backslash: \\, double quote: \", nbsp: \xFF."
```

### Raw Block Strings ###

Raw block strings provide a way to quote multiple lines of text with characters
that should not be escaped. A raw block string begins with `""""` (four
double quotes). A raw block string ends at the first newline before a printable
character that has a lower indentation.

Here are some examples of valid raw block strings:

```scopes
""""a single-line string as a block string
# commented line inbetween
""""// a multi-line string that describes a valid C function
    #include <stdio.h>
    void a_function_in_c() {
        printf("hello world\n");
    }
```

### Symbols ###

Like strings, a symbol describes a sequence of 8-bit characters, but acts as a
label or bindable name. Symbols may contain any character from the UTF-8
character set and terminate when encountering any character from the set
`#;()[]{},`. A symbol always terminates when one of these characters is
encountered. Any symbol that parses as a number is also excluded. Two symbols
sharing the same sequence of characters always map to the same value.

As a special case, `,` is always parsed as a single character.

Here are some examples of valid symbols:

```scopes
# classic underscore notation
some_identifier _some_identifier
# hyphenated
some-identifier
# mixed case
SomeIdentifier
# fantasy operators
&+ >~ >>= and= str+str
# numbered
_42 =303
```

### Numbers ###

Numbers come in two forms: integers and reals. The parser understands integers
in the range `-(2^63)` to `2^64-1` and records them as signed 32-bit values
unless the value is too big, in which case it will be extended to 64-bit
signed, then 64-bit unsigned. Reals are floating point numbers parsed and
stored as [IEEE-754](https://wikipedia.org/wiki/IEEE_754) binary32 values.

Numbers can be explicitly specified to be of a certain type by appending a `:`
to the number as well as a numerical typename that is one of: `i8`, `i16`,
`i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32` or `f64`.

Here are some examples of valid numbers:

```scopes
# positive and negative integers in decimal and hexadecimal notation
0 +23 42 -303 12 -1 -0x20 0xAFFE
# positive and negative reals
0.0 1.0 3.14159 -2.0 0.000003 0xa400.a400
# reals in scientific notation
1.234e+24 -1e-12
# special reals
+inf -inf nan
# zero as unsigned 64-bit integer and as signed 8-bit integer
0:u64 0:i8
# a floating-point number with double precision
1.0:f64
```

### Lists ###

Lists are the only nesting type, and can be either scoped by braces or
indentation. For braces, `()`, `[]` and `{}` are accepted.

Lists can be empty or contain a virtually unlimited number of elements,
only separated by whitespace. They typically describe expressions in Scopes.

Here are some examples of valid lists:

```scopes
# a list of numbers in naked format
1 2 3 4 5
# three empty braced lists within a naked list
() () ()
# a list containing a symbol, a string, an integer, a real, and an empty list
(print (.. "hello world") 303 606 909)
# three nesting lists
((()))
```

Naked & Braced Lists
--------------------

Every Scopes source file is parsed as a tree of expresion lists.

The classic notation (what we will call *braced notation*) uses a syntax close
to what [Lisp](http://en.wikipedia.org/wiki/Lisp_(programming_language)) and
[Scheme](http://en.wikipedia.org/wiki/Scheme_(programming_language)) authors
know as *restricted* [S-expressions](https://en.wikipedia.org/wiki/S-expression>):

```scopes
(print
    (.. "Hello" "World")
    303 606 909)
```

As a modern alternative, Scopes offers a *naked notation* where the scope of
lists is implicitly balanced by indentation, an approach used by
[Python](http://en.wikipedia.org/wiki/Python_(programming_language)),
[Haskell](http://en.wikipedia.org/wiki/Haskell_(programming_language)),
[YAML](http://en.wikipedia.org/wiki/YAML),
[Sass](http://en.wikipedia.org/wiki/Sass_(stylesheet_language)) and many
other languages.

This source parses as the same list in the previous, braced example:

```scopes
# The same list as above, but in naked format.
    A sub-paragraph continues the list.
print
    # elements on a single line with or without sub-paragraph are wrapped
        in a list.
    .. "Hello" "World"

    # values that should not be wrapped have to be prefixed with an
        escape token which causes a continuation of the parent list
    \ 303 606 909
```

### Mixing Modes ###

Naked lists can contain braced lists, and braced lists can
contain naked lists:

```scopes
# compute the value of (1 + 2 + (3 * 4)) and print the result
(print
    (+ 1 2
        (3 * 4)))

# the same list in naked notation.
    indented lists are appended to the parent list:
print
    + 1 2
        3 * 4

# any part of a naked list can be braced
print
    + 1 2 (3 * 4)

# and a braced list can contain naked parts.
    the escape character \ enters naked mode at its indentation level.
print
    (+ 1 2
        \ 3 * 4) # parsed as (+ 1 2 (3 * 4))
```

Naked notation is strongly encouraged as it is more convenient for authors
without specialized editors to write and balancing parentheses can be
challenging for beginners. However, purists and Scheme enthusiasts may prefer
to work with braced notation almost exclusively. As a result, Scopes'
reference documentation describes all available symbols in braced notation,
while code examples make ample use of naked notation.

Brace Styles
------------

In addition to regular parentheses `()`, SLN parses curly `{}` and
square `[]` brace styles. They are merely meant for providing variety for
writing SLN-based formats, and are expanded to simple lists during parsing.
Some examples:

```scopes
[a b c d]
# expands to
(\[\] a b c d)

{1 2 3 4}
# expands to
(\{\} 1 2 3 4)
```

List Separators
---------------

Both naked and braced lists support a special control character, the list
separator `;` (semicolon). Known as a statement separator in other languages,
it groups atoms into separate lists and allows the omission of required
parentheses or lines in complex trees.

In addition, it is possible to list-wrap the first element of a list in naked
mode by starting the head of the block with `;`.

Here are some examples:

```scopes
# in braced notation
(print a; print (a;b;); print c;)
# parses as
((print a) (print ((a) (b))) (print c))

# in naked notation
;
    print a; print b
    ;
        print c; print d
# parses as
((print a) (print b) ((print c) (print d)))
```

!!! warning

    If semicolons are used with braced notation then any trailing elements that
    are not terminated with `;` will not be wrapped:

        :::scopes
        # in braced notation
        (print a; print (a;b;); print c)
        # parses as
        ((print a) (print ((a) (b))) print c)

Pitfalls of Naked Notation
--------------------------

As naked notation gives the author the freedom to care less about parentheses,
it also takes away. The following section discusses small challenges that
might be encountered when using naked notation and how to resolve them.

### Single Elements ###

Special care must be taken when single elements are defined when the author
wants to wrap them in a list.

Here is a braced list describing an expression printing the number 42:

```scopes
(print 42)
```

The naked equivalent declares two elements in a single line, which are implicitly
wrapped in a single list:

```scopes
print 42
```

A single element on its own line is not wrapped:

```scopes
print           # (print
    42          #        42)
```

What if we want to just print a newline, passing no arguments?:

```scopes
print           # print
```

The statement above will be ignored because a symbol is resolved but not called.
One can make use of the `;` (split-statement) control
character, which ends the current list:

```scopes
print;          # (print)
```

### Continuation Lines ###

There are often situations when a high number of elements in a list
interferes with best practices of formatting source code and exceeds the line
column limit (typically 80 or 100).

In braced lists, the problem is easily corrected:

```scopes
# import many symbols from an external module into the active namespace
(import-from "OpenGL"
    glBindBuffer GL_UNIFORM_BUFFER glClear GL_COLOR_BUFFER_BIT
    GL_STENCIL_BUFFER_BIT GL_DEPTH_BUFFER_BIT glViewport glUseProgram
    glDrawArrays glEnable glDisable GL_TRIANGLE_STRIP)
```

The naked approach interprets each new line as a nested list:

```scopes
# produces runtime errors
import-from "OpenGL"
    glBindBuffer GL_UNIFORM_BUFFER glClear GL_COLOR_BUFFER_BIT
    GL_STENCIL_BUFFER_BIT GL_DEPTH_BUFFER_BIT glViewport glUseProgram
    glDrawArrays glEnable glDisable GL_TRIANGLE_STRIP

# braced equivalent of the term above; each line is interpreted
# as a function call and fails.
(import-from "OpenGL"
    (glBindBuffer GL_UNIFORM_BUFFER glClear GL_COLOR_BUFFER_BIT)
    (GL_STENCIL_BUFFER_BIT GL_DEPTH_BUFFER_BIT glViewport glUseProgram)
    (glDrawArrays glEnable glDisable GL_TRIANGLE_STRIP))
```

This can be fixed by using the `splice-line` control character, `\`:

```scopes
# correct solution using splice-line, postfix-style
import-from "OpenGL" \
    glBindBuffer GL_UNIFORM_BUFFER glClear GL_COLOR_BUFFER_BIT \
    GL_STENCIL_BUFFER_BIT GL_DEPTH_BUFFER_BIT glViewport glUseProgram \
    glDrawArrays glEnable glDisable GL_TRIANGLE_STRIP
```

Unlike in other languages, and as previously demonstrated, `\` splices at the
token level rather than the character level, and can also be placed at the
beginning of nested lines, where the parent is still the active list:

```scopes
# correct solution using the splice-line control character '\', prefix-style
import-from "OpenGL"
    \ glBindBuffer GL_UNIFORM_BUFFER glClear GL_COLOR_BUFFER_BIT
    \ GL_STENCIL_BUFFER_BIT GL_DEPTH_BUFFER_BIT glViewport glUseProgram
    \ glDrawArrays glEnable glDisable GL_TRIANGLE_STRIP
```

### Tail Splicing ###

Naked notation is ideal for writing nested lists that accumulate at the tail:

```scopes
# braced
(a b c
    (d e f
        (g h i))
    (j k l))

# naked
a b c
    d e f
        g h i
    j k l
```

However, there are complications when additional elements need to be spliced
back into the parent list:

```scopes
(a b c
    (d e f
        (g h i))
    j k l)
```

Once again, we can reuse the splice-line control character `\` to get what we
want:

```scopes
a b c
    d e f
        g h i
    \ j k l
```

### Left-Hand Nesting ###

When using infix notation, conditional blocks, or functions producing functions,
lists occur that nest at the head level rather than the tail:

```scopes
((((a b)
    c d)
        e f)
            g h)
```

The equivalent naked mode version makes extensive use of list separator and
splice-line characters to describe the same tree:

```scopes
# equivalent structure
;
    ;
        ;
            a b
            \ c d
        \ e f
    \ g h
```

A more complex tree which also requires splicing elements back into the parent
list can be implemented with the same combination of list separator and
splice-line characters:

```scopes
# braced
(a
    ((b
        (c d)) e)
    f g
    (h i))

# naked
a
    ;
        b
            c d
        \ e
    \ f g
    h i
```

While this example demonstrates the versatility of the splice-line and list
separator characters, use of partially braced notation may be easier to
read.

As usual, the best format is the one that fits the context.

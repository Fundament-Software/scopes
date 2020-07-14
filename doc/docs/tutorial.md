<style type="text/css" rel="stylesheet">
body { counter-reset: chapter 3; }
</style>

The Scopes Tutorial
===================

This tutorial does not attempt to cover every single feature, but focuses
on Scopes' most noteworthy features to give you a good idea of the
language’s flavor and style. After reading it, you will be able to read and
write Scopes modules and programs.

Using the Scopes Live Compiler
------------------------------

After downloading and unpacking the latest release of Scopes, the easiest way
to start it is to simply launch the executable shipped with the archive. It
is usually located in the root directory, and on Unix-compatible systems
it can simply be started from the terminal with:

    $ ./scopes

On Windows, and on systems where Scopes has been installed system-wide, it can
be started from the command line without the preceding dot:

    > scopes

### Interactive Console ###

When `scopes` is launched without arguments, it enters an interactive
read-eval-print loop (REPL), also called a console. Here is an example:

    $ ./scopes
      \\\
       \\\
     ///\\\
    ///  \\\  Scopes 0.14 (Apr 17 2019, 19:43:48)
    $0 ▶

!!! attention

    At the time of writing, some console emulators such as Mintty (used by
    default by the MSYS2 shell applications) may not display the Interactive
    Console's output properly. Windows users are advised to use the `-defterm`
    option when running the MSYS2 shell applications, or install
    [ConEmu](https://conemu.github.io/) and change them to use the `-conemu`
    option instead.

Simple expressions can be written on a single line, followed by hitting the
return key:

    $0 ▶ print "hello world"
    hello world
    $0 ▶

Multiline expressions can be entered by trailing the first line with a space
character, and exited by entering nothing on the last line:

    $0 ▶ print#put a space here
    ....     "yes"
    ....     "this"
    ....     "is"
    ....     "dog"
    ....
    yes this is dog
    $0 ▶

Entering a value binds it to the name indicated by the prompt, and can then
be reused:

    $0 ▶ 3
    $0 = 3
    $1 ▶ print $0
    3
    $1 ▶

A special keyboard shortcut (`Control+D`) at the prompt exits the program.
You can also exit the program by typing `exit;` followed by hitting the
return key.

### Launcher ###

Most of the time you would like to use Scopes to compile and execute your own
written Scopes programs. This is simply done by appending the name of the
Scopes file you would like to launch to the executable:

    $ scopes path/to/my/program.sc

A Fistful of Scopes
-------------------

Many of the examples in this tutorial include comments, even those entered at
the console. Comments in Scopes start with a hash character `#` and extend
to the first line starting with a character at a lower or equal indentation.

Some examples:

    # this is the first comment
    print "hey!" # and this is a second comment
                   and a third, continuing on the same indentation
    let str = "# hash characters inside string quotes do not count as comments"

### Using Scopes as a Calculator ###

Scopes is not only a fully-fledged compiler infrastructure, but also works
nicely as a comfy calculator:

    $0 ▶ 1 + 2 + 3
    $0 = 6
    $1 ▶ 23 + 2 * 21
    $1 = 65
    $2 ▶ (23 + 2 * 21) / 5
    $2 = 13.0
    $3 ▶ 8 / 5 # all divisions return a floating point number
    $3 = 1.6

Integer numbers like `6` or `65` have type `i32`, real numbers with a
fractional part like `13.0` or `1.6` have type `f32`.

!!! note

    You will likely notice that if the whitespace characters between the
    operators and numbers are omitted that the Interactive Console will
    display an error when evaluating the expression. For example:

        $0 ▶ 1+2+3
        <string>:1:1: while expanding
            1+2+3
        error: syntax: identifier '1+2+3' is not declared in scope. Did you mean 'u32', 'f128', 'f32',
        'i32', '+' or '+='?

    This is because symbol identifiers in Scopes may contain any character
    from the UTF-8 character set except whitespace characters and characters
    from the set `()[]{}"';#,`, where `,` is in itself a context-free
    symbol.  See [Notation](dataformat.md) for details.

Division always returns a real number. On the off-chance that you want an
integer result without the fractional part, use the floor division operator
`//`:

    $0 ▶ 23 / 3 # regular division returns a real
    $0 = 7.666667
    $1 ▶ 23 // 3 # floor division returns an integer
    $1 = 7
    $2 ▶ 23 % 3 # modulo returns the remainder
    $2 = 2
    $3 ▶ $1 * 3 + $2 # result * divisor + remainder
    $3 = 23

### Binding Names ###

Notice how the last example leveraged the auto-memorization function of the
console to bind any result to a name for reuse. But we can also make use of
`let` to bind values to specific names:

    $0 ▶ let width = 23
    23
    $0 ▶ let height = 42
    42
    $0 ▶ width * height
    $0 = 966

If a name is not bound to anything, using it will give you an error, which is
useful when you've just mistyped it:

    $0 ▶ let color = "red"
    $0 ▶ colour
    <string>:1:1: while expanding
        colour
    error: syntax: identifier 'colour' is not declared in scope. Did you mean 'color'?

### Strings ###

Life can be tedious and boring at times. Why not perform some string operations
to pass the time? We start with some light declarations of string literals:

    $0 ▶ "make it so" # every string is wrapped in double quotes
    $0 = "make it so"
    $1 ▶ "\"make it so!\", he said" # nested quotes need to be escaped
    $1 = "\"make it so!\", he said"
    $2 ▶ "'make it so!', he said" # single quotes are no problem though
    $2 = "'make it so!', he said"
    $3 ▶ """"1. make it so
             2. ???
             3. profit!
    ....
    $3 = "1. make it so\n2. ???\n3. profit!\n"

In the interactive console output, the output string is enclosed in quotes and
special characters are escaped with backslashes, to match the way the string
has been declared. Sometimes this might look a little different from the input,
but the strings are equivalent. The `print` function produces a more readable
output that produces the intended look:

    $0 ▶ print "make it so"
    make it so
    $0 ▶ print "\"make it so!\", he said"
    "make it so!", he said
    $0 ▶ print """"1. "make it so!", he said
                   2. ???
                   3. profit!"
    ....
    1. "make it so!", he said
    2. ???
    3. profit!

Sometimes it is necessary to join several strings into one. Strings can be
joined with the `..` operator:

    $0 ▶ "Sco" .. "pes" .. "!" # joining three strings together
    $0 = "Scopes!"
    $1 ▶ .. "Sco" "pes" "!" # using prefix notation
    $1 = "Scopes!"

The inverse operation, slicing strings, can be performed with the `lslice`,
`rslice` and `slice` operations:

    $0 ▶ "scopes" # bind the string we are working on to $0
    $0 = "scopes"
    $1 ▶ rslice $0 1 # slice right side starting at the second character
    $1 = "copes"
    $2 ▶ slice $0 1 5 # slice four letters from the center
    $2 = "cope"
    $3 ▶ lslice $0 ((countof $0) - 1) # a negative index selects from the back
    $3 = "scope"
    $4 ▶ rslice $0 ((countof $0) - 2) # get the last two characters
    $4 = "es"
    $5 ▶ slice $0 2 3 # get the center character
    $5 = "o"

One way to remember how slices work is to think of the indices as pointing
*between* characters, with the left edge of the first character numbered 0. Then
the right edge of the last character of a string of *n* characters has index *n*,
for example:

     +---+---+---+---+---+---+
     | S | c | o | p | e | s |
     +---+---+---+---+---+---+
     0   1   2   3   4   5   6

If we are interested in the byte value of a single character from a string, we
can use the `@` operator, also called the at-operator, to extract it:

    $0 ▶ "abc" @ 0
    $0 = 97:i8
    $1 ▶ "abc" @ 1
    $1 = 98:i8
    $2 ▶ "abc" @ 2
    $2 = 99:i8
    $3 ▶ "abc" @ ((countof "abc") - 1) # get the last character
    $3 = 99:i8

The `countof` operation returns the byte length of a string:

    $2 ▶ countof "six"
    $2 = 3:usize
    $3 ▶ countof "three"
    $3 = 5:usize
    $4 ▶ countof "five"
    $4 = 4:usize

### A Mild Breeze of Programming ###

Many calculations require repeating an operation several times, and of course
Scopes can also do that. For instance, here is one of the typical examples
for such a task, computing the first few numbers of the fibonacci sequence:

    $0 ▶ loop (a b = 0 1)
    ....     if (b < 10)
    ....         print b
    ....         repeat b (a + b)
    ....     else
    ....         break b
    ....
    1
    1
    2
    3
    5
    8
    $0 = 13

In Scopes, indentation is how the grouping of statements is determined which
is why the conditional block is indented. A tab or four spaces must start each
indented line within the block. Additionally, each line within a block must be
indented by the same amount.

!!! note
    When entering a block of statements in the Interactive Console, a space
    must be entered at the end of the line that starts the block.

This example introduces several new features:

* The first line declares the entry point of a loop so we can jump back
  (see the fourth line), bind new values to `a` and `b`, and perform the
  same operations again. The first line also performs multiple assignments at
  the same time. `a` is initially bound to `0`, while `b` is initialized
  to `1`:

        $0 ▶ loop (a b = 0 1)

* On the second line, we perform a *conditional operation*. That is, the
  indented block formed by lines three and four is only executed if the
  expression `(b < 10)` evaluates to `true`. In other words: we are going
  to be performing the loop as long as `b` is smaller than `10`:

        ....     if (b < 10)

    !!! tip

        Scopes offers a set of comparison operators for all basic types. You can
        compare any two numbers using `<` (less than), `>` (greater than),
        `==` (equal to), `<=` (less than or equal to), `>=` (greater than or equal to)
        and `!=` (not equal to).

* On line 4, the loop will be repeated with `a` bound to the value of `b`,
  while `b` will be bound to the result of calculating `(a + b)`:

        ....         repeat b (a + b)

* On line 5, we introduce the alternative block to be executed when `b`
  is greater or equal to `10`:

        ....     else

* On line 6, we break from the loop, returning the final value of `b`:

        ....         break b

Controlling Flow
----------------

Let us get a little deeper into ways you can structure control flow in Scopes.

### `if` Expressions ###

You have seen a small bit of `if` in that fibonacci example. `if` is your
go-to solution for any task that requires the program to make decisions.
Another example:

    $0 ▶ sc_prompt "please enter a word: " ""
    please enter a word: bang
    $0 $1 = true "bang"
    $2 ▶ if ($1 < "n")
    ....     print "early in the dictionary, good choice!"
    .... elseif ($1 == "scopes")
    ....     print "oh, a very good word!"
    .... elseif ($1 == "")
    ....     print "that is no word at all!"
    .... else
    ....     print "late in the dictionary, nice!"
    ....
    early in the dictionary, good choice!

You can also use `if` to decide on an expression:

    $0 ▶ let chosen = true
    true
    $0 ▶ print "you chose"
    ....     if x
    ....         "poorly"
    ....     elseif
    ....         "wisely"
    ....
    you chose poorly

### Defining Functions ###

Let us generalize the fibonacci example from earlier to a function that can
write numbers from the fibonacci sequence up to an arbitrary boundary:

    $0 ▶ fn fib (n) # write Fibonacci series up to n
    ....     loop (a b = 0 1)
    ....         if (a < n)
    ....             io-write! (repr a)
    ....             io-write! " "
    ....             repeat b (a + b)
    ....         else
    ....             io-write! "\n"
    ....             break b
    ....
    fib:Closure
    $0 ▶ fib 2000 # call the function we just defined
    0 1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987 1597
    $0 = 4181

The keyword `fn` introduces a function definition. It must be followed by an
optional name and a list of formal parameters. All expressions that follow
form the body of the function and it is good taste to indent them.

Executing (also called *applying*) a function binds the passed arguments to its
formal parameters and performs the actions within the function with that
argument standing in.

In this example, `n` is bound to `2000`, all instances of `n` in the body
of `fib` are replaced with `2000`, and therefore the loop is executed until
the condition `a < 2000` is `true`.


#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""testing
    =======

    The testing module simplifies writing and running tests in an ad-hoc
    fashion.

fn __test-modules (module-dir modules)
    let total =
        i32 (countof modules)

    loop (modules failed-modules) = modules '()
    if (empty? modules)
        let failed = (i32 (countof failed-modules))
        if (failed > 0)
            print;
            print "List of failed modules"
            print "======================"
            for m in failed-modules
                print "*" (m as Symbol as string)
        print;
        print total "tests executed," (total - failed) "succeeded," failed "failed."
        print "done."
        return;

    let module modules = (decons modules)
    let module = (module as Symbol)
    print "* running" (module as string)
    print "***********************************************"
    let ok =
        try
            require-from module-dir module
            true
        except (err)
            io-write!
                format-error err
            false
    repeat modules
        if ok
            failed-modules
        else
            cons module failed-modules
    return;

# (test-modules module ...)
define-macro test-modules
    list __test-modules 'module-dir
        list quote
            args

define-macro assert-error
    inline test-function (f)
        xpcall
            inline fn ()
                f;
                unconst false
            inline fn (exc)
                io-write! "ASSERT OK: "
                io-write!
                    format-exception exc
                unconst true

    inline assertion-error! (constant anchor msg)
        let assert-msg =
            .. "error assertion failed: "
                if (== (typeof msg) string) msg
                else (repr msg)
        if constant
            compiler-error! assert-msg
        else
            syntax-error! anchor assert-msg
    let cond body = (decons args)
    let sxcond = (as cond Syntax)
    let anchor = (Syntax-anchor sxcond)
    let tmp =
        Parameter-new anchor 'tmp Unknown
    list do
        list let tmp '=
            list test-function
                list fn '() cond
        list if tmp
        list 'else
            cons assertion-error!
                list constant? tmp
                active-anchor;
                if (empty? body)
                    list (repr (Syntax->datum sxcond))
                else body

define-macro assert-compiler-error
    inline test-function (f)
        xpcall
            inline fn ()
                compile
                    typify (unconst f)
                unconst false
            inline fn (exc)
                io-write! "ASSERT OK: "
                io-write!
                    format-exception exc
                unconst true

    inline assertion-error! (constant anchor msg)
        let assert-msg =
            .. "compiler error assertion failed: "
                if (== (typeof msg) string) msg
                else (repr msg)
        if constant
            compiler-error! assert-msg
        else
            syntax-error! anchor assert-msg
    let cond body = (decons args)
    let sxcond = (as cond Syntax)
    let anchor = (Syntax-anchor sxcond)
    let tmp =
        Parameter-new anchor 'tmp Unknown
    list do
        list let tmp '=
            list test-function
                list fn '() cond
        list if tmp
        list 'else
            cons assertion-error!
                list constant? tmp
                active-anchor;
                if (empty? body)
                    list (repr (Syntax->datum sxcond))
                else body

locals;

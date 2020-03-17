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

    loop (modules failed-modules = modules '())
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
                    'format err
                io-write! "\n"
                false
        repeat modules
            if ok
                failed-modules
            else
                cons module failed-modules

let __test =
    spice-macro
        fn "__test" (args)
            fn check-assertion (result anchor msg)
                if (not result)
                    hide-traceback;
                    error
                        .. "test failed: " msg

            let argc = ('argcount args)
            verify-count argc 2 2
            let expr msg =
                'getarg args 0
                'getarg args 1
            if (('typeof msg) != string)
                error "string expected as second argument"
            let anchor = ('anchor args)
            'tag `(check-assertion expr anchor msg) anchor

define-sugar-macro test
    let cond msg body = (decons args 2)
    let msg = (convert-assert-args args cond msg)
    list __test cond msg

# (test-modules module ...)
define-sugar-macro test-modules
    list __test-modules 'module-dir
        list sugar-quote
            args

define-sugar-macro test-error
    inline test-function (f)
        try
            if true
                f;
            false
        except (err)
            io-write! "ASSERT OK: "
            static-if ((typeof err) == Error)
                print
                    'format err
            else
                print
                    typeof err
            true

    inline assertion-error! (msg)
        let assert-msg =
            .. "error test failed: "
                if (== (typeof msg) string) msg
                else (repr msg)
        error assert-msg
    let cond body = (decons args)
    let sxcond = cond
    let anchor = ('anchor sxcond)
    let tmp =
        sc_symbol_new_unique "tmp"
    list do
        list let tmp '=
            list test-function
                #list fn '() cond
                list inline '() cond
        list if tmp
        list 'else
            cons assertion-error!
                if (empty? body)
                    list
                        if (('typeof sxcond) == list)
                            repr (sxcond as list)
                        else
                            repr sxcond
                else body

sugar test-compiler-error (args...)
    spice test-function (f)
        let f = (f as Closure)
        try
            sc_compile (sc_typify f 0 null) 0:u64
            false
        except (err)
            io-write! "COMPILER ERROR TEST OK: "
            print
                'format err
            true

    inline assertion-error! (anchor msg)
        let assert-msg =
            .. "compiler error test failed: "
                if (== (typeof msg) string) msg
                else (repr msg)
        hide-traceback;
        error@ anchor "while checking test" assert-msg
    let cond body = (decons args...)
    let cond =
        try (sc_expand cond '() sugar-scope)
        except (err)
            io-write! "COMPILER ERROR TEST OK (while expanding): "
            print
                'format err
            return '()
    let sxcond = cond
    let anchor = ('anchor sxcond)
    let tmp =
        sc_symbol_new_unique "tmp"
    list do
        list let tmp '=
            list test-function
                list fn '() cond
        list if tmp
        list 'else
            cons assertion-error! anchor
                if (empty? body)
                    list
                        if (('typeof sxcond) == list)
                            repr (sxcond as list)
                        else
                            repr sxcond
                else body

sugar features (args...)
    """"A feature matrix that tests 2-d permutations

        usage:
        features    B1  B2  B3 ...
            ---
            A1      Y   N   Y
            A2      N   Y   N
            A3      Y   N   Q

        will expand to:
        do
            Y A1 B1; N A1 B2; Y A1 B3
            N A2 B1; Y A2 B2; N A2 B3
            Y A3 B1; N A3 B2; Q A3 B3

    let header rest =
        loop (header rest = '() args...)
            if (empty? rest)
                error "-* expected"
            let arg rest = (decons rest)
            if (('typeof arg) == Symbol)
                let s = (arg as Symbol as string)
                if ((lslice s 1) == "-")
                    break header rest
            repeat
                cons arg header
                rest
    let numcolumns = (countof header)
    let reversed_header = ('reverse header)
    cons do
        'reverse
            fold (result = '()) for row in rest
                let func outcomes = (decons (row as list))
                fold (result = result) for outcome title in
                    zip outcomes reversed_header
                    cons (list outcome func title) result

run-stage;

""""this type is used for discovering leaks and double frees. It holds an integer
    value as well as a pointer to a single reference on the heap which is 1 as
    long as the object exists, otherwise 0. The refcount is leaked in
    order to not cause segfaults when a double free occurs.

    In addition, a global refcounter is updated which can be checked for balance.
typedef One :: (tuple i32 (mutable pointer i32))
    global _refcount = 0

    fn refcount ()
        deref _refcount

    fn reset-refcount ()
        _refcount = 0
        ;

    fn test-refcount-balanced ()
        # this also fixes the refcount for subsequent tests
        let balanced? = (_refcount == 0)
        _refcount = 0
        test balanced?

    inline __typecall (cls value)
        static-assert (not (none? value))
        test (_refcount >= 0)
        _refcount += 1
        let one_is_the_loneliest_number = (malloc i32)
        store 1 one_is_the_loneliest_number
        bitcast (tupleof value one_is_the_loneliest_number) this-type

    fn __repr (self)
        let vals = (storagecast self)
        .. "<" (tostring (@ vals 1)) "=" (tostring (@ vals 0)) ">"

    inline make-binop (op)
        inline "binop" (cls T)
            static-if (cls == T)
                inline (a b) (op ('value a) ('value b))
            else
                ;

    let __== = (make-binop ==)
    let __!= = (make-binop !=)
    let __< = (make-binop <)
    let __<= = (make-binop <=)
    let __> = (make-binop >)
    let __>= = (make-binop >=)

    unlet make-binop

    fn value (self)
        deref (@ (storagecast (view self)) 0)

    fn check (self)
        let ref = (@ (@ (storagecast self) 1))
        test (ref == 1)
        ;

    fn __drop (self)
        viewing self
        _refcount -= 1
        assert (_refcount >= 0)
        let ref = (@ (@ (storagecast self) 1))
        if (ref != 1)
            report "reference ==" ref
        assert (ref == 1)
        ref = 0
        ;

do
    let One features test-compiler-error test-error test test-modules

    locals;

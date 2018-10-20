
sugar fn... (name...)
    let letdef = (alloca bool)
    store false letdef
    let fn-name =
        syntax-match name...
        case (name as Symbol;)
            store true letdef
            name
        case (name as string;) (Symbol name)
        case () unnamed
        default
            compiler-error!
                """"syntax: (fn... name|"name") (case pattern body...) ...
    let outexpr = (sc_expression_new)
    loop (next) = next-expr
    syntax-match next
    case (('case condv body...) rest...)
        do
            let tmpl = (sc_template_new fn-name)
            sc_expression_append outexpr tmpl
            let scope = (Scope syntax-scope)
            let types =
                sc_argument_list_new;
            loop (expr) = (uncomma (condv as list))
            syntax-match expr
            case ()
                let body = (sc_expand (cons do body...) '() scope)
                sc_template_set_body tmpl body
                let tmpl = `(typify tmpl types)
                if (load letdef)
                    'set-symbol syntax-scope fn-name tmpl

                #for name in ('args names)

                #for arg T in (zip ('args names) ('args types))

                    print arg T
                #'dump names
                'dump types
                _;
            case (((arg as Symbol) ': T) rest...)
                let T = (sc_expand T '() syntax-scope)
                T as type
                let param = (sc_parameter_new arg)
                sc_template_append_parameter tmpl param
                sc_argument_list_append types T
                'set-symbol scope arg param
                repeat rest...
            default
                syntax-error! ('anchor condv) "syntax: (parameter-name : type, ...)"
        repeat rest...
    default
        #print next-expr
        syntax-match name...
        case (name as Symbol;)
            print name
            _;
        case (name as string;)
            print name
            _;
        case ()
            print "()"
            _;
        default
            compiler-error!
                """"syntax: (fn... name|"name") (case pattern body...) ...
        return outexpr next

spice test (x y z args...)
    assert ((x as i32) == 1)
    assert ((y as i32) == 2)
    assert ((z as i32) == 3)
    assert (('argcount args...) == 3)
    let u v w =
        'getarg args... 0
        'getarg args... 1
        'getarg args... 2
    assert ((u as i32) == 4)
    assert ((v as i32) == 5)
    assert ((w as i32) == 6)
    `(+ x y z u v w)

spice test-match (args...)
    match-args args...
    case (a as i32, b as i32, c...)
        # both a and b must be i32 constants and will be unwrapped as such.
        # we can precompute a + b here
        `(print "case1" a b [(a + b)] "|" c...)
    case (a : i32, b : i32, c...)
        # a and b must implicitly cast to i32, and the cast will be
          auto-generated.
        `(print "case2" a b (a + b) "|" c...)
    case (x y z)
        # three arguments only
        `(print "case3" x y z)
    case (x y...)
        # at least one argument
        `(print "case4" x "|" y...)
    default
        compiler-error! "wrong!"

compile-stage;

fn... testf
case (a : i32, b : u32)
    (a + (b as i32))

print testf

do
    test-match 1 2 3 4          # case 1
    test-match (4 - 3) 2 3 4    # case 2
    test-match 1:i8 2 3 4       # case 2
    test-match "a" "b" "c"      # case 3
    test-match "a" "b"          # case 4

assert
    (test 1 2 3 4 5 6) == 21

return;
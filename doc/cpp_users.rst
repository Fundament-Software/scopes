Scopes for C/C++ Users
======================

This is an introduction of Scopes for C/C++ users. We are going to highlight
commonalities and differences between both languages. At the end of this
introduction, you will have a better understanding of how C/C++ concepts
translate to Scopes, and which idiosyncrasies to expect.

Execution
---------

C/C++ requires programs to be built and linked before they can be executed. For
this, a build system is typically employed.

In contrast, Scopes' primary execution mode is live, which means that the
compiler remains on-line while the program is running, permitting to compile
additional functions on demand, and to use other services provided by the
Scopes runtime library. Programs are compiled transparently and cached in the
background to optimize loading times. This mechanism is designed to be
maintenance-free.

However, Scopes can also build object files compatible with GCC and Clang at
runtime using `compile-object`, which makes it both suitable for classic offline
compilation and as foundation for a build system.

When building objects with Scopes, certain restrictions apply. The generated
object file has no ties to the Scopes runtime, and so you must not use any first
class objects as constants in your program. You may use ``sc_*`` functions
exported by the Scopes runtime, but must link with ``libscopesrt`` to make
those symbols available.

Compiler Errors
---------------

At compile time, C/C++ compilers attempt to catch and report as many errors
encountered as possible by default, with minimal contextual information provided
to prevent bloating the error report further.

In contrast, Scopes terminates compilation at the first error encountered,
providing a compiler stack trace highlighting the individual steps in
chronological order that have led to this error.

Runtime Debugging
-----------------

C/C++ programs are debugged at runtime by using a step debugger like GDB or
WinDbg, or by what is commonly referred to as "printf debugging".

Scopes generates DWARF/COFF debug information for its programs and therefore
supports step debuggers like GDB which understand the format. Due to a
limitation in LLVM, debug information generated at runtime is currently not
available on Windows. Object files are not affected by this limitation.

Two ways of "printf debugging" are provided, the function `report`, which prints
the runtime value of the arguments provided, and the builtin `dump`, which
prints, at compile time, the instruction and type of the arguments provided,
allowing to inspect constants and types of variables with inferred type. Both
functions prefix their output by the source location of where they were
called, so they are easy to remove when the session is over.

Indentation
-----------

As indentation has no significance in C/C++, many indentation styles are known,
and they constitute a cherished subject of much discussion and many pointless
arguments.

In Scopes, scoping is controlled either by parentheses or indentation. To
permit users to freely exchange code without friction, the indentation level is
fixed at four spaces outside of parenthesed expressions, and the use of tab
characters for indentation is not permitted.

Symbols
-------

For symbolic tokens that can be used to bind names to values and types, C
accepts the character set of ``0-9A-Za-z_``. This allows users to concatenate
many tokens without separating whitespace, such as in ``x+y``.

Instead of a whitelist of permitted characters, Scopes only maintains a
blacklist of characters that terminate a symbolic sequence. These characters are
the whitespace characters and characters from the set ``()[]{}"';#,``, where
``,`` is in itself a context free symbol.

This means that ``x+y`` would be read as a single token. A semantically
equivalent expression in Scopes would have to be written as ``x + y``.

Keywords
--------

C/C++ uses many different declarative forms which are recognized and translated
during parsing using specific keywords. Like many other Scheme-likes, Scopes
only parses programs as symbolic lists and postpones the interpretation of
declarations until the last possible moment, expecting all expressions to follow
just one basic form::

    # classic braced expression
    (head argument1 ... argumentN)
    # naked syntax
    head argument1 ... argumentN
    # naked paragraph form
    head argument1 ...
        ...
        argumentN

The value and type of the head controls whether the expression is dispatched to

* A syntax macro (called a `sugar`), called at expansion time, which has
  complete control over which, when and how remaining arguments will be expanded
  and evaluated, and can either return new symbolic lists to be expanded
  further, or a template IL.
* An IL macro (called a `spice`), called at compile time, which receives typed
  arguments and can generate new template IL to be specialized.
* A call expression, equivalent to the ``head(argument1, ..., argumentN);``
  syntax in C/C++, called at runtime.

As a result of this principle, there are no keywords in Scopes. Every single
symbol can be rebound or deleted, globally or just for one scope.

Scopes also supports wildcard syntax macros (wildcard sugars), which can be
applied to either any expression or symbol before the expansion begins, and
which can be used to implement exceptions to the head dispatch rule. One of
these exceptions for instance is infix notation support.

Infix Expressions
-----------------

C/C++ offers a fixed set of infix operators which are recognized during parsing
and translated to AST nodes right then. They can be used within expressions
but must be put in parentheses or separated by semicolon or comma from
neighboring expressions in argument or statement lists.

Scopes also offers a set of commonly used infix operators not unlike the ones
provided by C, utilizing the same associativity and nearly the same precedence
(the `<<` and `>>` operators use a different precedence), but renaming and
adding operators where it improves clarity.

Unlike C/C++, Scopes allows to define new scoped infix operators at the users
convenience using `define-infix<` and `define-infix>`, as a simple alias to an
existing sugar, spice or function.

An infix expression is recognized by looking for an infix definition for the
second token of an expression. If a matching definition is found, all tokens
within that expression are treated as left/right-hand arguments and operators.

The infix expression must follow the pattern ``(x0 op x1 op ... op xN)`` to be
recognized. If an odd argument is not a valid infix token, a syntax error
will be raised.

Scopes does deliberately not implement any concept of mixed infix, prefix
or postfix expressions to keep confusion to a minimum. Even infix expressions
can be entirely disabled by replacing the default wildcard sugar.

A special symbol sugar exists which aims to simplify trivial container lookups.
An expression like ``(object . attribute . attribute)`` can also be written
as a single symbol, ``object.attribute.attribute``, which will be expanded
to the former form, provided no value is already bound to this symbol in the
current scope.

Declarations, Statements and Expressions
----------------------------------------

C/C++ distinguishes between three major lexical contexts: declaration level,
statement level and expression level.

.. code-block:: c++

    // declaration level
    typedef int MyInt;

    // illegal at this level
    // printf("hello!\n");

    MyInt test (MyInt x) {
        // statement level
        int k =
            // expression level
            x * x
        ;
        int m = ({
            // statement expressions, a GCC extension
            // usage of statements here is legal.
            printf("hello again!\n");
            k * k;
        });
        return m;
    }

Scopes does not make such a distinction, and instead treats every declaration
as an expression with a result type. The top level of a program is equivalent
to its main function::

    # the right hand side is not limited to constant expressions.
    let MyInt = int

    # legal at this level.
    print "hello!"

    fn test (x)
        let k = (x * x)

        let m =
            do
                # equivalent to statement expressions in GCC.
                print "hello again!"
                k * k
        # even `return` declares an expression of type `noreturn`.
        return m

Constants and Variables
-----------------------

C/C++ expects named values to be declared with a type. Each value is mutable
by default unless qualified by ``const``. Outside of a function it represents
a globally accessible value, within a function it represents a stack value.

.. code-block:: c++

    const int constant = 100;

    // a global value mapped to data segment
    int variable1 = 0;
    int variable2 = 0;

    // conceptually a copy operation
    const int variable1_copy = variable1;

    void test () {
        // conceptually declared on the stack
        // initialization from value conceptually a copy operation
        int variable3 = constant;

        // mutable by default
        variable3 = variable2;

        printf("%i\n", constant);
        printf("%i\n", variable1);
        printf("%i\n", variable2);
        printf("%i\n", variable3);
    }

In Scopes, expressions are bound to names using `let`. `let` does only perform
the binding. The type of the value and where it is stored depends entirely on
the expression that produces it. The `local` and `global` forms must be used to
explicitly allocate a stack or data segment value::

    # a compile time constant integer
    let constant = 100

    # not a global value, but allocated on the main function's stack
    local variable1 = 0
    # a global value mapped to data segment
    global variable2 = 0

    # variable1 is bound to another name - not a copy operation.
      variable1_copy remains mutable.
    let variable1_copy = variable1

    fn test ()
        # just a rebind - not a copy operation
        let variable3 = constant
        # variable3 is not a reference, so can not be mutated.
          we should have declared it as local for that.
        # variable3 = variable2

        print constant
        # illegal: variable1 is a stack variable outside of function scope
        # print variable1
        # legal: variable2 is a global
        print variable2
        # variable3 is a constant
        print variable3

Unlike in C/C++, declarations of the same name within the same scope are also
permitted, and the previous binding is still accessible during evaluation
of the right-hand side::

    let x = 1
    # x is now bound to the value 3
    let x = (x + 2)
    # x is now bound to a string
    let x = "test"

Lexical Scope
-------------

Both C/C++ and Scopes employ lexical scope to control visibility of bound names.

Unlike in C/C++, lexical scope is a first order object in Scopes and can be used
by new declarative forms as well as to export symbols from modules::

    let scope =
        do
            let x = 1
            let y = "test"

            # build a new scope from locally bound names
            locals;

    # scope is constant if all values in it are constant
    static-assert (constant? scope)
    # prints 1 "test"
    print scope.x scope.y

Macros
------

The C preprocessor provides the only means of using macros in C/C++ code. The
latest edition permits variadic arguments, but reflection and conditional
behavior can only be achieved through tricks. C macros are also unable to
bind names in a way that prevents collision with existing names in scope, which
is called "unhygienic" in the Scheme community. Macros are able to transparently
override call expressions and symbolic tokens, and do not have to respect
semantic structure.

As a Scheme-like, Scopes' macro facilities are extensive.

Sugars are functions able to rewrite expressions at the syntactical level during
syntax expansion. Wildcard sugars can rewrite symbols or even just parts of
symbols. They are evaluated top-down and can produce hygienic and unhygienic
expressions.

Spices are evaluated bottom-up during typechecking and receive eagerly evaluated
arguments. Both forms can generate new code in the form of untyped IL.

Hygienic macro functions are provided by the `inline` form, which is expanded
during typechecking. Inlines must respect semantic structure and are not
programmable, but can make use of spices to perform reflection and conditional
code generation, as well as generate new functions.

Templates
---------

In C, every function must be typed as it is (forward) declared. C++ introduces
the concept of templates, which are functions that can be lazily typed. As of
C++14, templates can now also deduct their return type. Templates can be forward
declared, but forward declared templates with automatic return type can not
be instantiated.

.. code-block:: c++

    // forward declaration of typed function
    int typed_forward_decl (int x, const char *text, bool toggle);

    // declaration of typed function
    int typed_decl (int x, const char *text, bool toggle) {
        return 0;
    }

    // forward declaration of template
    template<typename A, typename B, typename C>
    void lazy_typed_decl_returns_void (A a, B b, C c);

    void test1 () {
        // forward declaration can be used
        lazy_typed_decl_returns_void(1,2,3);
    }

    // forward declaration of template with auto return type
    template<typename A, typename B, typename C>
    auto lazy_typed_decl_returns_auto (A a, B b, C c);

    void test2 () {
        // error: use before deduction of ‘auto’
        lazy_typed_decl_returns_auto(1,2,3);
    }

    // implementation of template with auto return type
    template<typename A, typename B, typename C>
    auto lazy_typed_decl_returns_auto (A a, B b, C c) {
        return 0;
    }

In Scopes, all function declarations are lazily typed, and `static-typify` can
be used to instantiate concrete functions at compile time. Forward declarations
are possible but must be completed within the same scope::

    # forward declarations can not be typed
    #fn typed_forward_decl

    fn typed_decl (x text toggle)
        return 0

    # create typed function
    let typed_decl = (static-typify typed_decl i32 rawstring bool)

    # forward declaration of template has no parameter list
    fn lazy_typed_decl_returns_void

    # test1 is another template
    fn test1 ()
        # legal because test1 is not instantiated yet
        # note: lazy_typed_decl_returns_void must be implemented before
                test1 is instantiated.
        lazy_typed_decl_returns_void 1 2 3

    # forward declaration of template with auto return type
      note that all our forward declarations have no return type
    fn lazy_typed_decl_returns_auto

    fn test2 ()
        # legal because test2 is not instantiated yet
        lazy_typed_decl_returns_auto 1 2 3

    # implementation of template with auto return type
    fn lazy_typed_decl_returns_auto (a b c)
        return 0

    # instantiate test2
    let test2 = (static-typify test2)

Variadic Arguments
------------------

C introduced variadic arguments at runtime using the ``va_list`` type, in order
to support variadic functions like ``printf()``. C++ improved upon this
concept by introducing variadic template arguments. It remains difficult to
perform reflection on variadic arguments, such as iteration or targeted
capturing.

Functions in Scopes do not support runtime variadic functions (although calling
variadic C functions is supported), but support compile time variadic arguments.
See the following example::

    # any trailing parameter ending in '...' is interpreted to be variadic.
    fn takes-varargs (x y rest...)
        # count the number of arguments in rest...
        let numargs = (va-countof rest...)

        # use let's support for variadic values to split arguments into
          first argument and remainder.
        let z rest... = rest...

        # get the 5th argument from rest...
        let fifth_arg = (va@ 5 rest...)

        # iterate through all arguments, perform an action on each one
          and store the result in a new variadic value.
        let processed... =
            va-map
                inline (value)
                    print value
                    value + 1
                rest...

        # return variadic result as multiple return values
        return processed...

Overloading
-----------

C++ allows overloading of functions by specifying multiple functions with
the same name but different type signatures. On call, the call arguments types
are used to deduce the correct function to use.

.. code-block:: c++

    int overloaded (int a, int b) { return 0; }
    int overloaded (int a, float b) { return 1; }
    int overloaded (float a, int b) { return 2; }

    // this new form of overloaded could be specified in a different file
    int overloaded (float a, float b) { return 3; }

Scopes offers a similar mechanism as a library form, but requires that
overloads must be grouped at the time of declaration. The first form that
matches argument types implicitly is selected, in order of declaration::

    fn... overloaded
    case (a : i32, b : i32)
        return 0
    case (a : i32, b : f32)
        return 1
    case (a : f32, b : i32)
        return 2

    # expanding overloaded in a different file, limited to local scope

    # overwrites the previous declaration
    fn... overloaded
    case using overloaded # chains the previous declaration
    case (a : f32, b : f32) # will be tried last
        return 3

Code Generation
---------------

In C/C++, files are interpreted either as translation units (the root file of a
compiler invocation) or as header files, which are type and forward declarations
that typically do not generate code on their own, embedded into translation
units. Fully declared functions are guaranteed to generate code, and will
only be optimized out at linking stage.

In Scopes, every invocation of `sc_compile`, typically through `compile` or
`import`, opens a new translation unit. Function declarations are template
declarations, so they do not generate any code, nor does any other compile time
construct. Instantiating a function through `static-typify` does also not
guarantee that code will be generated. Only actual first time use will generate
code for whatever translation unit is currently active, and make that code
available to every future translation unit. This guarantees that a particular
template is only instantiated once.

When objects are compiled through `compile-object`, only functions exported
through the scope argument are guaranteed to be included, and all functions
they depend on. Objects are complete. Previously generated functions will not
be externally defined, but will be redefined as private functions within the
objects translation unit. The same rules apply to global variables.

Using Third Party Libraries
---------------------------

With C/C++, third party libraries are typically built in a separate build
process provided by the libraries developer, either as static or shared
libraries. Their definitions are made available through include files that one
can embed into one's own translation units using the ``#include`` preprocessing
command. The libraries' precompiled symbols are merged into the executable
during linking or at runtime, either when the process is mapped into memory
or function pointers are loaded manually from the library.

Scopes provides a module system which allows shipping libraries as sources.
Any scopes source file can be imported as a module. When a module is first
imported using `import` or `using import`, its main body is compiled and
executed. The returned scope which contains the modules' exported functions and
types is cached under the modules name and returned to the importing program.
The modules' functions and types are now available to the program and can be
embedded directly into its translation unit. No code is generated until the
libraries' functions are actually used.

Scopes also supports embedding existing third party C libraries in the classical
way, using `include`, `load-library` and `load-object`::

    # how to create trivial bindings for a C library

    # include thirdparty.h and make its declarations available as a scope object
    let thirdparty =
        include "thirdparty.h"
            options "-I" (module-dir .. "/../include") # specify options for clang

    # access a define from thirdparty.h
    if thirdparty.define.USE_SHARED_LIBRARY
        # load thirdparty as a shared library from system search paths
        if (operating-system == 'windows)
            load-library "thirdparty.dll"
        else
            load-library "libthirdparty.so"
    else
        # load thirdparty as a static library from an object file
        load-object (module-dir .. "/../lib/thirdparty.o")

    # assemble a ready-to-use scope object for this module
    do
        # import only symbols beginning with thirdparty
        using thirdparty.define filter "^THIRDPARTY_.*$"
        using thirdparty.typedef filter "^thirdparty_.*$"
        using thirdparty.const filter "^thirdparty_.*$"
        using thirdparty.extern filter "^thirdparty_.*$"

        locals;

Externals using C signatures can also be defined and used directly::

    let puts = (extern 'puts (function i32 rawstring))
    # definition becomes immediately available
    puts "hello\n"

Type Primitives
---------------

C/C++'s type primitives map to Scopes in the following way:

=============================================== =======================
C++                                             Scopes
=============================================== =======================
`bool`                                          `bool`
`int8_t`                                        `i8`
`int16_t`                                       `i16`
`int32_t`                                       `i32`
`int64_t`                                       `i64`
`uint8_t`                                       `u8`
`uint16_t`                                      `u16`
`uint32_t`                                      `u32`
`uint64_t`                                      `u64`
`float`                                         `f32`
`double`                                        `f64`
``typedef U V``                                 `let V = U`
``using V = U``                                 `let V = U`
`const T *`                                     `@ T`
`T *`                                           `mutable (@ T)`
`const T &`                                     `& T`
`T &`                                           `mutable (& T)`
``std::array<T, N>``                            `array T N`
``std::tuple<T0, ..., Tn>``                     `tuple T0 ... Tn`
``const T [N] __attribute__((aligned (A)))``    `vector T N`
=============================================== =======================

Initializer Lists
-----------------

Scopes provides convenience constructors for arrays and tuple types, as well as
a special initializer type called `typeinit`, which can be used to initialize
fields without knowing their type. `typeinit` stores passed arguments in a
temporary closure which is turned into a constructor call as soon as the
typeinit instance is cast to its target type during assignment.

=========================================== =====================================
C++                                         Scopes
=========================================== =====================================
`std::array<T> _ = { arg0, ..., argN };`    `arrayof T arg0 ... argN`
`std::tuple<auto> _ = { arg0, ..., argN };` `tupleof arg0 ... argN`
`x.member = { arg0, ..., argN };`           `x.member = (typeinit arg0 ... argN)`
=========================================== =====================================

Structs
-------

Scopes supports structs in a format not unlike the one C/C++ provides, but does
not permit composition by inheritance. Composition must be strictly explicit.

Compare this C++ example, which makes use of recently introduced default
initializers and designated initializers:

.. code-block:: c++

    struct Example {
        int value;
        // default initializers only supported in C++11 and up
        bool choice = false;
        const char *text = "";
    };

    // designated initializers only supported in C99
    Example example = { .value = 100, .text = "test" };

to this equivalent declaration in Scopes::

    using import struct

    struct Example plain
        value : i32
        # type can be deduced from initializer
        choice = false
        text : rawstring = ""

    global example : Example
        value = 100
        text = "test"

Methods
-------

C++ introduced methods as a way to associate functions directly with structs
and classes. In C++, the argument referencing the object argument is hidden
and implicitly bound to the ``this`` symbol. Members and other methods of the
struct are in the lexical scope of the method.

.. code-block:: c++

    // this example is a little contrived for illustrational purposes
    struct Example {
        int value;

        // a method declaration
        int get_add_value (int n) {
            return this->value + n;
        }

        // another method declaration
        void print_value_plus_one () {
            printf("%i\n", get_add_value(1));
        }
    };

    void use_example (Example example) {
        example.print_value_plus_one();
    }

Scopes supports methods in a more explicit way that makes refactorings from
function to method and back easier, both in declaration and in usage::

    struct Example plain
        value : i32

        # note the explicit presence of the object parameter
        fn get_add_value (self n)
            self.value + n

        fn print_value_plus_one (self)
            print ('get_add_value self 1)

    fn use_example (example)
        'print_value_plus_one example

What happens here is that we call a quoted symbol with arguments. The call
handler for the `Symbol` type rewrites ``'methodname object arg0 ... argN``
as ``(getattr (typeof object) 'methodname) object arg0 ... argN``.

Virtual Methods
---------------

As Scopes doesn't provide a native abstraction for composition by inheritance,
virtual methods are not supported out of the box, but can be implemented through
its extensive domain specific language support.

Classes
-------

C++'s concept of classes is only indirectly supported through structs in Scopes.
Access modifiers are not available, but methods can be made "private" by keeping
their definition local. Fields can not be hidden, but they can be visibly
marked as private by convention::

    struct Example plain
        # an underscore indicates that the attribute is not meant to be
          accessed directly.
        _value : i32

        fn get_add_value (self n)
            self._value + n

        fn print_value_plus_one (self)
            # use get_add_value directly
            print (get_add_value self 1)

        # unbind get_add_value from local scope to prevent it
          from being added as an attribute to Example.
        unlet get_add_value

    fn use_example (example)
        # this operation is not possible from here:
        # 'get_add_value example 1
        'print_value_plus_one example

Template Classes
----------------

C++ supports generics in the form of template classes, which are lazily typed
structs.

.. code-block:: c++

    template<typename T, int x>
    struct Example {
        T value;

        bool compare () {
            value == x;
        }
    };

Scopes leverages constant expression folding and compile time closures to
trivially provide this feature via `inline` functions::

    # a function decorator memoizes the result so we get the same type for
      the same arguments
    @@ memo
    inline Example (T x)
        # construct type name from string
        struct ("Example<" .. (tostring T) .. ">")
            value : T

            fn compare ()
                value == x

Partial template specialization allows to choose different implementations
depending on instantiation arguments. The same mechanism is also used to do
type based dispatch. Here is an example:

.. code-block:: c++

    #include <stdlib.h>

    template<typename T> struct to_int {
        // linker complains: missing symbol
        int operator()(T x);
    };

    template<> struct to_int<int> {
        int operator()(int x) {
            return x;
        }
    };

    template<> struct to_int<const char *> {
        int operator()(const char *x) {
            return atoi(x);
        }
    };

In Scopes, it is not necessary to create types in order to build single
type based dispatch operators. Here are three ways to supply the same
functionality::

    include "stdlib.h"

    # a function that generates a function
    @@ memo
    inline to_int1 (T)
        static-match T
        case i32 _
        case rawstring atoi
        default
            static-error "unsupported type"

    # a function that performs the operation directly
    inline to_int2 (x)
        let T = (typeof x)
        static-if (T == i32) x
        elseif (T == rawstring) (atoi x)
        else
            static-error "unsupported type"

    # using the overloaded function abstraction
    fn... to_int3
    case (x : i32,) x
    case (x : rawstring,) (atoi x)

Constructors
------------

Scopes supports construction from type through the `__typecall` special method.
A type implementing a method under this name becomes callable. By convention,
it is used to construct both specialized types and to instantiate a type. Its
first argument is the name of the type that has been called.

Here is an example that changes the default constructor of a struct::

    struct Example plain
        _value : i32

        inline __typecall (cls n)
            # within the context of a struct definition, super-type is bound
              to the super type of the struct we are defining. In this case
              the supertype is `CStruct`.
            super-type.__typecall cls
                _value = (n * n)

Destructors
-----------

C++ provides so-called destructors which permit to execute code when a value goes
out of scope. Destructors typically free resources, but can also be used to
switch contexts.

.. code-block:: c++

    struct Handle {
        void *_handle;

        // constructor
        Handle(void *handle) : _handle(handle) {}
        // destructor
        ~Handle() {
            printf("destroying handle\n");
            free(_handle);
        }
    };

Values of non-plain type, so-called unique types, are guaranteed to be
referenced only at a single point within a program. Because of this guarantee,
a unique type is able to supply a destructor through the `__drop` special method
that is automatically called when the value goes out of scope::

    struct Handle
        _handle : voidstar

        # constructor
        inline __typecall (cls handle)
            super-type.__typecall cls handle

        # destructor
        inline __drop (self)
            print "destroying handle"
            free self._handle
            return;


Operator Overloading
--------------------

C++ allows overloading type operators through special methods defined either
in a struct, class or namespace.

..  code-block:: c++

    class Accumulable {
    public:
        // overload the addition operator for class + class
        Accumulable operator +(Accumulable x) {
            return Accumulable(this->value + x.value);
        }
        // another overload for class + int
        Accumulable operator +(int x) {
            return Accumulable(this->value + x);
        }

        Accumulable (int _value) : value(_value) {}

        int value;
    };

    // a third overload for supporting int + class
    Accumulable operator +(Accumulable a, int b) {
        return Accumulable(a.value + b.value);
    }

Scopes supports operator overloading through informally specified operator
protocols that that any type can support by exposing dispatch methods bound to
special attributes. See this equivalent example, which applies not only to
structs, but any type definition::

    struct Accumulable
        # one compile time function for all left-hand side variants receives
            left-hand and right-hand types and returns a function which can
            perform the operation or void.
        inline __+ (cls T)
            # test for type + type
            static-if (T == this-type)
                # return new closure
                inline (self other)
                    this-type (self.value + other.value)
            # if T can be implicitly cast to i32, support it
            elseif (imply? T i32)
                inline (self other)
                    this-type (self.value + other)

        # another function covers all right-hand side variants
        inline __r+ (T cls)
            static-if (imply? T i32)
                inline (self other)
                    this-type (self + other.value)

        value : i32

        inline __repr (self)
            tostring self.value

Standard Library
----------------

C and C++ support an extensive standard library, covering many system functions
and algorithmic container types.

Scopes supports the C standard library through the clang bridge accessible
by the `include` mechanism.

Only a few containers from the C++ standard library have functional equivalents
in Scopes yet. Here is a comparison table:

======================= ====================
C++                     Scopes
======================= ====================
``std::array``          `array`
``std::tuple``          `tuple`
``std::vector``         `Array.GrowingArray`
``std::unordered_set``  `Map.Set`
``std::unordered_map``  `Map.Map`
``std::unique_ptr``     `Box.Box`
``std::function``       `Capture.capture`
======================= ====================

Memory Handling & Management
----------------------------

C and C++ use a stack based machine model that is compatible with native
targets. Within this model, mutable memory can be pre-allocated globally,
on the function's stack and in main memory, called the "heap".

Scopes uses an identical model. No garbage collection is employed at runtime.

A comparison of concepts by example:

=================================== ===================================
C/C++                               Scopes
=================================== ===================================
``T value;`` (globally)             `global value : T`
``T value;`` (locally)              `local value : T`
``T values[size];`` (static size)   `local value : (array T size)`
``T values[size];`` (dynamic size)  `let value = (alloca-array T size)`
``alloca(sizeof(T))``               `alloca T`
``alloca(sizeof(T) * size)``        `alloca-array T size`
``malloc(sizeof(T))``               `malloc T`
``malloc(sizeof(T) * size)``        `malloc-array T size`
=================================== ===================================

In addition, C++ allows to manage memory by recursively invoking a type-defined
destructor on stack values when exiting a bracketed scope, as well as on globals
when the program is exited or a library unloaded. Based on this mechanism and
intricate elision rules, various smart pointer types are implemented, of which
the most useful is ``std::unique_ptr``, which is a type that manages the
lifetime of a single heap value until it goes out of scope.

In Scopes, types can be defined as unique, which instructs scope to manage the
lifetime of values in a similar fashion as C++ does. In addition, weak
references to these values (direct or indirect) can be borrowed as "views",
which become inaccessible as soon as the viewed unique value goes out of scope.
This mechanism incurs no performance cost at runtime. See `Destructors`_ for
an example.

Closures
--------

C++ supports runtime closures through the ``std::function`` type, which allows
to implicitly bind values to a function, so that when the function is called,
the bound values become available to the function without having to pass them
as arguments. In functional programming, this process can be used to implement
currying.

..  code-block:: c++

    void print_bound_constant () {
        const int y = 42;
        // capture `y` along with the function
        auto f = [y](int x) -> int { return x + y; }
        // prints 65
        std::cout << f(23) << std::endl;
    }

    void print_bound_value (int y) {
        // capture `y` along with the function
        auto f = [y](int x) -> int { return x + y; }
        // prints 23+y
        std::cout << f(23) << std::endl;
    }

Scopes supports compile time closures natively, as demonstrated previously, but
runtime closures are also supported through so-called captures. Above example
would be translated as follows::

    fn print_bound_constant ()
        let y = 42
        # capture constant `y` along with the function
        fn f (x)
            x + y
        # prints 65
        print (f 23)

    fn print_bound_value (y)
        # capture variable `y` along with the function
        capture f (x) {y}
            x + y
        # prints 65
        print (f 23)

Loops
-----

C offers two structured control flows for loops which depend on mutation of
an exit variable, namely ``while`` and ``for``. In addition, C++11 introduced
the range-based ``for`` loop, which provides syntactical sugar for iterating
elements of a collection.

..  code-block:: c++

    // C-style for-loop implementing a counter
    for (int i = 0; i < 10; ++i) {
        printf("%i\n", i);
    }

    // C-style for-loop implementing an iterator
    for (iter_t it = first(container), int k = 0; is_valid(it); it = next(it), k++) {
        process(k, at(it));
    }

    // while-loop implementing a counter
    int i = 0;
    while (i < 10) {
        printf("%i\n", i);
        ++i;
    }

    // range-based for-loop implementing an iterator (C++17 form)
    for (auto &&[first,second] : map) {
        process(first, second);
    }

Scopes defines a single builtin primitive for loops which leverages
backpropagation of immutable values, upon which various other library forms are
implemented::

    # implementing a counter using the range-based form
    for i in (range 10)
        print i

    # implementing an iterator using the loop primitive and immutable values
    loop (it k = (first container) 0)
        if (is_valid it)
            process k (at it)
            repeat (next it) (k + 1)
        else
            # break can return values
            break it k

    # implementing a counter using a while loop and mutation
    local i = 0
    while (i < 10)
        print i
        i += 1

    # range-based form implementing an iterator
    for key value in map
        process key value

In addition, with the `fold .. for .. in` form, Scopes combines both immutable
loop and range-based form.

Targeting Shader Programs
-------------------------

C/C++ do not offer a native way to compile functions to shader code. However,
there exist various third party solutions to provide equivalent features. The
GLSL (GL shader language) offers a C-like domain specific language to write
shaders that has enough overlap with C/C++ in order to allow users to share
definitions.

Scopes is able to natively compile functions to SPIR-V as well as GLSL at
compile time using the builtins `compile-spirv` and `compile-glsl` respectively,
allowing the CPU and GPU side to share all definitions. To aid in this task,
Scopes provides the :doc:`module-glm` and :doc:`module-glsl` modules, which
implement native GLSL types and functions.

See the following example implementing and compiling a pixel shader::

    using import glm
    using import glsl

    in uv : vec2 (location = 0)
    out color : vec4 (location = 1)
    fn main ()
        color = (vec4 (uv * 0.5 + 0.5) 0 1)

    print
        compile-glsl 330 'fragment
            static-typify main

The program output is as follows:

.. code-block:: glsl

    #version 330
    #ifdef GL_ARB_shading_language_420pack
    #extension GL_ARB_shading_language_420pack : require
    #endif

    in vec2 uv;
    layout(location = 1) out vec4 color;

    void main()
    {
        vec2 _14 = (uv * vec2(0.5)) + vec2(0.5);
        vec4 _19 = vec4(0.0);
        _19.x = _14.x;
        vec4 _21 = _19;
        _21.y = _14.y;
        vec4 _22 = _21;
        _22.z = 0.0;
        vec4 _24 = _22;
        _24.w = 1.0;
        color = _24;
    }



Exceptions
----------

C provides only a primitive kind of unstructured exception handling via the
``setjmp()`` and ``longjmp()`` functions provided by ``setjmp.h``.

C++ provides structured and polymorphic exception handling at runtime. Any value
can be thrown as an exception using the ``throw`` keyword, and caught using
the ``try .. catch`` form.

.. code-block:: c++

    struct myexception {
        const char *what;
    };

    void main () {
        try {
            // throw value of type myexception
            myexception exc = { "an error occurred" };
            throw exc;
        } catch (myexception& e) {
            // print content to screen
            std::cout << e.what << std::endl;
        }
    }

Scopes supports a form of structured exception handling that is monomorphic,
light weight and C compatible. A value of any type can be raised using the
``raise`` form, and handled using the ``try .. except`` form::

    using import struct

    struct myexception
        what : string

    try
        # raise value of type myexception
        raise (myexception "an error occurred")
    except (e)
        # print content to screen
        print e.what

The presence of an exception modifies the return type of a function to a hidden
tagged union type which returns which path the function returned on, and
both return and exception value, of which only the appropriate value has been
set.

Monomorphic means that in contrast to C++, Scopes does not allow more than one
exception type per expression to be backpropagated. If you wish to support a
polymorphic type, you can use `enum` to define a tagged union type which can be
dispatched to the correct exception type.

ABI Compliance
--------------

Scopes aims to achieve full compliance with the C ABI used on x64 platforms
for Linux, MacOS X and Windows, defaulting to the ``cdecl`` calling convention.
Other calling conventions are not yet supported. Any Scopes function can be
passed as a callback to a C library, and C functions can be called from Scopes
without any additional hinting required.

All types aim to follow the same alignment and size conventions as C types,
including plain unions.

On Windows, Scopes is built for and communicates with system resources through
mingw64. Operating with WINAPI functions directly has not been extensively
tested yet.

globals
=======

These names are bound in every fresh module and main program by default.
Essential symbols are created by the compiler, and subsequent utility
functions, macros and types are defined and documented in `core.sc`.

The core module implements the remaining standard functions and macros,
parses the command-line and optionally enters the REPL.

.. define:: backslash-char

   A constant of type `i8`.
.. define:: barrier-kind-control

   A constant of type `i32`.
.. define:: barrier-kind-memory

   A constant of type `i32`.
.. define:: barrier-kind-memory-buffer

   A constant of type `i32`.
.. define:: barrier-kind-memory-group

   A constant of type `i32`.
.. define:: barrier-kind-memory-image

   A constant of type `i32`.
.. define:: barrier-kind-memory-shared

   A constant of type `i32`.
.. define:: cache-dir

   A constant of type `String`.
.. define:: compile-flag-O1

   A constant of type `u64`.
.. define:: compile-flag-O2

   A constant of type `u64`.
.. define:: compile-flag-O3

   A constant of type `u64`.
.. define:: compile-flag-cache

   A constant of type `u64`.
.. define:: compile-flag-dump-disassembly

   A constant of type `u64`.
.. define:: compile-flag-dump-function

   A constant of type `u64`.
.. define:: compile-flag-dump-module

   A constant of type `u64`.
.. define:: compile-flag-dump-time

   A constant of type `u64`.
.. define:: compile-flag-no-debug-info

   A constant of type `u64`.
.. define:: compiler-dir
   
   A string containing the folder path to the compiler environment. Typically
   the compiler environment is the folder that contains the ``bin`` folder
   containing the compiler executable.
.. define:: compiler-file-kind-asm

   A constant of type `i32`.
.. define:: compiler-file-kind-bc

   A constant of type `i32`.
.. define:: compiler-file-kind-llvm

   A constant of type `i32`.
.. define:: compiler-file-kind-object

   A constant of type `i32`.
.. define:: compiler-path
   
   A string constant containing the file path to the compiler executable.
.. define:: compiler-timestamp
   
   A string constant indicating the time and date the compiler was built.
.. define:: debug-build?
   
   A boolean constant indicating if the compiler was built in debug mode.
.. define:: default-target-triple

   A constant of type `String`.
.. define:: e
   
   Euler's number, also known as Napier's constant. Explicitly type-annotated
   versions of the constant are available as `e:f32` and `e:f64`
.. define:: e:f32
   
   See `e`.
.. define:: e:f64
   
   See `e`.
.. define:: false

   A constant of type `bool`.
.. define:: global-flag-block

   A constant of type `u32`.
.. define:: global-flag-buffer-block

   A constant of type `u32`.
.. define:: global-flag-coherent

   A constant of type `u32`.
.. define:: global-flag-flat

   A constant of type `u32`.
.. define:: global-flag-non-readable

   A constant of type `u32`.
.. define:: global-flag-non-writable

   A constant of type `u32`.
.. define:: global-flag-restrict

   A constant of type `u32`.
.. define:: global-flag-volatile

   A constant of type `u32`.
.. define:: infinite-range
   
   A `Generator` that iterates through all 32-bit signed integer values starting
   at 0. This generator does never terminate; when it exceeds the maximum
   positive integer value of 2147483647, it overflows and continues with the
   minimum negative integer value of -2147483648.
.. define:: list-handler-symbol

   A constant of type `Symbol`.
.. define:: none

   A constant of type `Nothing`.
.. define:: null

   A constant of type `NullType`.
.. define:: operating-system
   
   A string constant indicating the operating system the compiler was built
   for. It equals to ``"linux"`` for Linux builds, ``"windows"`` for Windows
   builds, ``"macos"`` for macOS builds and ``"unknown"`` otherwise.
.. define:: pi
   
   The number π, the ratio of a circle's circumference C to its diameter d.
   Explicitly type-annotated versions of the constant are available as `pi:f32`
   and `pi:f64`.
.. define:: pi:f32
   
   See `pi`.
.. define:: pi:f64
   
   See `pi`.
.. define:: pointer-flag-non-readable

   A constant of type `u64`.
.. define:: pointer-flag-non-writable

   A constant of type `u64`.
.. define:: question-mark-char

   A constant of type `i8`.
.. define:: slash-char

   A constant of type `i8`.
.. define:: style-comment

   A constant of type `Symbol`.
.. define:: style-error

   A constant of type `Symbol`.
.. define:: style-function

   A constant of type `Symbol`.
.. define:: style-instruction

   A constant of type `Symbol`.
.. define:: style-keyword

   A constant of type `Symbol`.
.. define:: style-location

   A constant of type `Symbol`.
.. define:: style-none

   A constant of type `Symbol`.
.. define:: style-number

   A constant of type `Symbol`.
.. define:: style-operator

   A constant of type `Symbol`.
.. define:: style-sfxfunction

   A constant of type `Symbol`.
.. define:: style-string

   A constant of type `Symbol`.
.. define:: style-symbol

   A constant of type `Symbol`.
.. define:: style-type

   A constant of type `Symbol`.
.. define:: style-warning

   A constant of type `Symbol`.
.. define:: symbol-handler-symbol

   A constant of type `Symbol`.
.. define:: true

   A constant of type `bool`.
.. define:: type-kind-arguments

   A constant of type `i32`.
.. define:: type-kind-array

   A constant of type `i32`.
.. define:: type-kind-function

   A constant of type `i32`.
.. define:: type-kind-image

   A constant of type `i32`.
.. define:: type-kind-integer

   A constant of type `i32`.
.. define:: type-kind-pointer

   A constant of type `i32`.
.. define:: type-kind-qualify

   A constant of type `i32`.
.. define:: type-kind-real

   A constant of type `i32`.
.. define:: type-kind-sampled-image

   A constant of type `i32`.
.. define:: type-kind-tuple

   A constant of type `i32`.
.. define:: type-kind-typename

   A constant of type `i32`.
.. define:: type-kind-vector

   A constant of type `i32`.
.. define:: typed-symbol-handler-symbol

   A constant of type `Symbol`.
.. define:: typename-flag-plain

   A constant of type `u32`.
.. define:: unknown-anchor

   A constant of type `Anchor`.
.. define:: unnamed

   A constant of type `Symbol`.
.. define:: unroll-limit
   
   A constant of type `i32` indicating the maximum number of recursions
   permitted for an inline. When this number is exceeded, an error is raised
   during typechecking. Currently, the limit is set at 64 recursions. This
   restriction has been put in place to prevent the compiler from overflowing
   its stack memory.
.. define:: value-kind-alloca

   A constant of type `i32`.
.. define:: value-kind-annotate

   A constant of type `i32`.
.. define:: value-kind-argument-list

   A constant of type `i32`.
.. define:: value-kind-argument-list-template

   A constant of type `i32`.
.. define:: value-kind-atomicrmw

   A constant of type `i32`.
.. define:: value-kind-barrier

   A constant of type `i32`.
.. define:: value-kind-binop

   A constant of type `i32`.
.. define:: value-kind-call

   A constant of type `i32`.
.. define:: value-kind-call-template

   A constant of type `i32`.
.. define:: value-kind-cast

   A constant of type `i32`.
.. define:: value-kind-cmpxchg

   A constant of type `i32`.
.. define:: value-kind-compile-stage

   A constant of type `i32`.
.. define:: value-kind-condbr

   A constant of type `i32`.
.. define:: value-kind-const-aggregate

   A constant of type `i32`.
.. define:: value-kind-const-int

   A constant of type `i32`.
.. define:: value-kind-const-pointer

   A constant of type `i32`.
.. define:: value-kind-const-real

   A constant of type `i32`.
.. define:: value-kind-discard

   A constant of type `i32`.
.. define:: value-kind-exception

   A constant of type `i32`.
.. define:: value-kind-execution-mode

   A constant of type `i32`.
.. define:: value-kind-expression

   A constant of type `i32`.
.. define:: value-kind-extract-argument

   A constant of type `i32`.
.. define:: value-kind-extract-argument-template

   A constant of type `i32`.
.. define:: value-kind-extract-element

   A constant of type `i32`.
.. define:: value-kind-extract-value

   A constant of type `i32`.
.. define:: value-kind-fcmp

   A constant of type `i32`.
.. define:: value-kind-free

   A constant of type `i32`.
.. define:: value-kind-function

   A constant of type `i32`.
.. define:: value-kind-get-element-ptr

   A constant of type `i32`.
.. define:: value-kind-global

   A constant of type `i32`.
.. define:: value-kind-icmp

   A constant of type `i32`.
.. define:: value-kind-if

   A constant of type `i32`.
.. define:: value-kind-image-query-levels

   A constant of type `i32`.
.. define:: value-kind-image-query-lod

   A constant of type `i32`.
.. define:: value-kind-image-query-samples

   A constant of type `i32`.
.. define:: value-kind-image-query-size

   A constant of type `i32`.
.. define:: value-kind-image-read

   A constant of type `i32`.
.. define:: value-kind-image-write

   A constant of type `i32`.
.. define:: value-kind-insert-element

   A constant of type `i32`.
.. define:: value-kind-insert-value

   A constant of type `i32`.
.. define:: value-kind-keyed

   A constant of type `i32`.
.. define:: value-kind-keyed-template

   A constant of type `i32`.
.. define:: value-kind-label

   A constant of type `i32`.
.. define:: value-kind-label-template

   A constant of type `i32`.
.. define:: value-kind-load

   A constant of type `i32`.
.. define:: value-kind-loop

   A constant of type `i32`.
.. define:: value-kind-loop-arguments

   A constant of type `i32`.
.. define:: value-kind-loop-label

   A constant of type `i32`.
.. define:: value-kind-loop-label-arguments

   A constant of type `i32`.
.. define:: value-kind-malloc

   A constant of type `i32`.
.. define:: value-kind-merge

   A constant of type `i32`.
.. define:: value-kind-merge-template

   A constant of type `i32`.
.. define:: value-kind-parameter

   A constant of type `i32`.
.. define:: value-kind-parameter-template

   A constant of type `i32`.
.. define:: value-kind-pure-cast

   A constant of type `i32`.
.. define:: value-kind-quote

   A constant of type `i32`.
.. define:: value-kind-raise

   A constant of type `i32`.
.. define:: value-kind-repeat

   A constant of type `i32`.
.. define:: value-kind-return

   A constant of type `i32`.
.. define:: value-kind-sample

   A constant of type `i32`.
.. define:: value-kind-select

   A constant of type `i32`.
.. define:: value-kind-shuffle-vector

   A constant of type `i32`.
.. define:: value-kind-store

   A constant of type `i32`.
.. define:: value-kind-switch

   A constant of type `i32`.
.. define:: value-kind-switch-template

   A constant of type `i32`.
.. define:: value-kind-template

   A constant of type `i32`.
.. define:: value-kind-triop

   A constant of type `i32`.
.. define:: value-kind-undef

   A constant of type `i32`.
.. define:: value-kind-unop

   A constant of type `i32`.
.. define:: value-kind-unquote

   A constant of type `i32`.
.. define:: value-kind-unreachable

   A constant of type `i32`.
.. type:: Anchor

   A plain type of storage type `_Anchor<*>`.

.. type:: Arguments

   An opaque type.

   .. spice:: (__typecall ...)
.. type:: Builtin

   A plain type of storage type `u64`.

   .. spice:: (__hash ...)
.. type:: CEnum

   An opaque type of supertype `immutable`.

   .. spice:: (__!= ...)
   .. spice:: (__& ...)
   .. spice:: (__* ...)
   .. spice:: (__+ ...)
   .. spice:: (__- ...)
   .. spice:: (__/ ...)
   .. spice:: (__// ...)
   .. spice:: (__< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. spice:: (__^ ...)
   .. spice:: (__imply ...)
   .. inline:: (__neg self)
   .. spice:: (__rimply ...)
   .. spice:: (__| ...)
   .. inline:: (__~ self)
.. type:: CStruct

   An opaque type.

   .. spice:: (__drop ...)
   .. spice:: (__getattr ...)
   .. spice:: (__typecall ...)
.. type:: CUnion

   An opaque type.

   .. spice:: (__getattr ...)
   .. inline:: (__typecall cls value...)
.. type:: Closure

   A plain type of storage type `_Closure<*>`.

   .. spice:: (__!= ...)
   .. spice:: (__== ...)
   .. spice:: (__hash ...)
   .. spice:: (__imply ...)
   .. compiledfn:: (docstring ...)

      An external function of type ``String<-(Closure)``.
.. type:: Collector

   A plain type of storage type `_Closure<*>`.

   .. spice:: (__call ...)
   .. inline:: (__typecall cls init valid? at collect)
.. type:: CompileStage

   A plain type of storage type `{_Value Anchor}`.

.. type:: Error

   A plain type of storage type `_Error<*>`.

   .. inline:: (append self anchor traceback-msg)
   .. compiledfn:: (dump ...)

      An external function of type ``void<-(Error)``.
   .. compiledfn:: (format ...)

      An external function of type ``String<-(Error)``.
.. type:: Generator

   
   Generators provide a protocol for iterating the contents of containers and
   enumerating sequences. They are primarily used by `for` and `fold`, but can
   also be used separately.
   
   Each generator instance is equivalent to a closure that when called returns
   four functions:
   
   * A function ``state... <- fn start ()`` which returns the initial state of
     the generator as an arbitrary number of arbitrarily typed values. The
     initially returned state defines the format of the generators internal
     state.
   * A function ``bool <- fn valid? (state...)`` which takes the current
     generator state and returns `true` when the generator can resolve the
     state to a collection item, otherwise `false`, indicating that the
     generator has been depleted.
   * A function ``value... <- fn at (state...)`` which takes the current
     generator state and returns the collection item this state maps to. The
     function may not be called for a state for which ``valid?`` has reported
     to be depleted.
   * A function ``state... <- fn next (state...)`` which takes the current
     generator state and returns the state mapping to the next item in the
     collection. The new state must have the same type signature as the
     previous state. The function may not be called for a state for which
     ``valid?`` has reported to be depleted.
   
   It is allowed to call any of these functions multiple times with any valid
   state, effectively restarting the Generator at an arbitrary point, as
   Generators are not expected to have side effects. In controlled
   circumstances a Generator may choose to be impure, but should be documented
   accordingly.
   
   Here is a typical pattern for constructing a generator::
   
       inline make-generator (container)
           Generator
               inline "start" ()
                   # return the first iterator of sequence (might not be valid)
                   'start container
               inline "valid?" (it...)
                   # return true if the iterator is still valid
                   'valid-iterator? container it...
               inline "at" (it...)
                   # return variadic result at iterator
                   '@ container it...
               inline "next" (it...)
                   # return the next iterator in sequence
                   'next container it...
   
   The generator can then be subsequently used like this::
   
       # this example prints up to two elements returned by a generator
       # generate a new instance bound to container
       let gen = (make-generator container)
       # extract all methods
       let start valid? at next = (gen)
       # get the init state
       let state... = (start)
       # check if the state is valid
       if (valid? state...)
           # container has at least one item; print it
           print (at state...)
           # advance to the next state
           let state... = (next state...)
           if (valid? state...)
               # container has one more item; print it
               print (at state...)
       # we are done; no cleanup necessary

   
   .. spice:: (__call self)
   
      Returns, in this order, the four functions ``start``, ``valid?``,
      ``init`` and ``next`` which are required to enumerate generator
      `self`.
   .. inline:: (__typecall cls start valid? at next)
      
      Takes four functions ``start``, ``valid?``, ``at`` and ``next``
      and returns a new generator ready for use.
.. type:: Image

   An opaque type.

   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``.
.. type:: Nothing

   A plain type of storage type `{}`.

   .. inline:: (__tobool)
.. type:: NullType

   A plain type of storage type `void<*>`.

   .. spice:: (__== ...)
   .. spice:: (__imply ...)
   .. spice:: (__r== ...)
   .. inline:: (__repr self)
.. type:: OverloadedFunction

   An opaque type.

   .. spice:: (__typecall ...)
   .. spice:: (append ...)
.. type:: Qualify

   An opaque type.

.. type:: Raises

   An opaque type.

.. type:: SampledImage

   An opaque type.

   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<-(type)``.
.. type:: Sampler

   An opaque type.

.. type:: Scope

   A plain type of storage type `_Scope<*>`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<->Error(Scope Value)``.
   .. spice:: (__.. ...)
   .. spice:: (__== ...)
   .. spice:: (__as ...)
   .. spice:: (__getattr ...)
   .. spice:: (__hash ...)
   .. spice:: (__typecall ...)
   .. spice:: (bind ...)
   .. inline:: (bind-symbols self values...)
   .. compiledfn:: (bind-with-docstring ...)

      An external function of type ``Scope<-(Scope Value Value String)``.
   .. spice:: (define ...)
   .. inline:: (define-symbols self values...)
   .. inline:: (deleted self)
   .. compiledfn:: (docstring ...)

      An external function of type ``String<-(Scope Value)``.
   .. inline:: (lineage self)
   .. compiledfn:: (local@ ...)

      An external function of type ``Value<->Error(Scope Value)``.
   .. compiledfn:: (module-docstring ...)

      An external function of type ``String<-(Scope)``.
   .. compiledfn:: (next ...)

      An external function of type ``λ(Value Value i32)<-(Scope i32)``.
   .. compiledfn:: (next-deleted ...)

      An external function of type ``λ(Value i32)<-(Scope i32)``.
   .. compiledfn:: (parent ...)

      An external function of type ``Scope<-(Scope)``.
   .. compiledfn:: (reparent ...)

      An external function of type ``Scope<-(Scope Scope)``.
   .. compiledfn:: (unbind ...)

      An external function of type ``Scope<-(Scope Value)``.
   .. compiledfn:: (unparent ...)

      An external function of type ``Scope<-(Scope)``.
.. type:: SourceFile

   A plain type of storage type `_SourceFile<*>`.

.. type:: SpiceMacro

   A plain type of storage type `Value<->Error(Value)<*>`.

   .. spice:: (__rimply ...)
.. type:: SpiceMacroFunction

   A plain type labeled ``Value<->Error(Value)<*>`` of supertype `pointer` and of storage type `Value<->Error(Value)<*>`.

.. type:: Struct

   An opaque type.

   .. spice:: (__drop ...)
   .. builtin:: (__getattr ...)
   .. spice:: (__typecall ...)
.. type:: SugarMacro

   A plain type of storage type `λ(List Scope)<->Error(List Scope)<*>`.

   .. spice:: (__call ...)
.. type:: SugarMacroFunction

   A plain type labeled ``λ(List Scope)<->Error(List Scope)<*>`` of supertype `pointer` and of storage type `λ(List Scope)<->Error(List Scope)<*>`.

.. type:: Symbol

   A plain type of supertype `immutable` and of storage type `u64`.

   .. spice:: (__!= ...)
   .. spice:: (__== ...)
   .. spice:: (__as ...)
   .. spice:: (__call ...)
   .. spice:: (__hash ...)
   .. inline:: (__typecall cls str)
   .. inline:: (unique cls name)
   .. compiledfn:: (variadic? ...)

      An external function of type ``bool<-(Symbol)``.
.. type:: TypeArrayPointer

   A plain type labeled ``type(*)`` of supertype `pointer` and of storage type `type(*)`.

.. type:: TypeInitializer

   An opaque type.

   .. inline:: (__static-imply cls T)
.. type:: Unknown

   A plain type of storage type `_type<*>`.

.. type:: Value

   A plain type of storage type `{_Value Anchor}`.

   .. spice:: (__== ...)
   .. inline:: (__as vT T)
   .. compiledfn:: (__repr ...)

      An external function of type ``String<-(Value)``.
   .. inline:: (__rimply vT T)
   .. spice:: (__typecall ...)
   .. compiledfn:: (anchor ...)

      An external function of type ``Anchor<-(Value)``.
   .. compiledfn:: (argcount ...)

      An external function of type ``i32<-(Value)``.
   .. inline:: (arglist-sink N)
   .. inline:: (args self)
   .. compiledfn:: (constant? ...)

      An external function of type ``bool<-(Value)``.
   .. fn:: (dekey self)
   .. inline:: (dump self)
   .. compiledfn:: (getarg ...)

      An external function of type ``Value<-(Value i32)``.
   .. compiledfn:: (getarglist ...)

      An external function of type ``Value<-(Value i32)``.
   .. compiledfn:: (kind ...)

      An external function of type ``i32<-(Value)``.
   .. compiledfn:: (none? ...)

      A compiled function of type ``bool<-(Value)``.
   .. compiledfn:: (pure? ...)

      An external function of type ``bool<-(Value)``.
   .. compiledfn:: (qualified-typeof ...)

      An external function of type ``type<-(Value)``.
   .. inline:: (reverse-args self)
   .. compiledfn:: (spice-repr ...)

      An external function of type ``String<-(Value)``.
   .. inline:: (tag self anchor)
   .. compiledfn:: (typeof ...)

      An external function of type ``type<-(Value)``.
.. type:: ValueArrayPointer

   A plain type labeled ``Value(*)`` of supertype `pointer` and of storage type `Value(*)`.

.. type:: Variadic

   An opaque type labeled ``...``.

.. type:: aggregate

   An opaque type.

   .. spice:: (__drop ...)
.. type:: array

   An opaque type of supertype `aggregate`.

   .. inline:: (__@ self index)
   .. spice:: (__as ...)
   .. spice:: (__countof ...)
   .. spice:: (__typecall ...)
   .. spice:: (__unpack ...)
   .. inline:: (type element-type size)
.. type:: bool

   A plain type of supertype `integer` and of storage type `bool`.

.. type:: constant

   An opaque type.

.. type:: f128

   A plain type of supertype `real` and of storage type `f128`.

.. type:: f16

   A plain type of supertype `real` and of storage type `f16`.

.. type:: f32

   A plain type of supertype `real` and of storage type `f32`.

.. type:: f64

   A plain type of supertype `real` and of storage type `f64`.

.. type:: f80

   A plain type of supertype `real` and of storage type `f80`.

.. type:: function

   An opaque type.

   .. spice:: (__typecall ...)
   .. spice:: (type ...)
.. type:: hash

   A plain type of storage type `u64`.

   .. spice:: (__!= ...)
   .. spice:: (__== ...)
   .. spice:: (__as ...)
   .. inline:: (__hash self)
   .. spice:: (__ras ...)
   .. spice:: (__typecall ...)
   .. inline:: (from-bytes data size)
.. type:: i16

   A plain type of supertype `integer` and of storage type `i16`.

.. type:: i32

   A plain type of supertype `integer` and of storage type `i32`.

.. type:: i64

   A plain type of supertype `integer` and of storage type `i64`.

.. type:: i8

   A plain type of supertype `integer` and of storage type `i8`.

.. type:: immutable

   An opaque type.

.. type:: incomplete

   An opaque type.

.. type:: integer

   An opaque type of supertype `immutable`.

   .. spice:: (__!= ...)
   .. spice:: (__% ...)
   .. spice:: (__& ...)
   .. spice:: (__* ...)
   .. spice:: (__** ...)
   .. spice:: (__+ ...)
   .. spice:: (__- ...)
   .. spice:: (__/ ...)
   .. spice:: (__// ...)
   .. spice:: (__< ...)
   .. spice:: (__<< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. spice:: (__>> ...)
   .. spice:: (__^ ...)
   .. spice:: (__as ...)
   .. spice:: (__hash ...)
   .. spice:: (__imply ...)
   .. inline:: (__neg self)
   .. inline:: (__rcp self)
   .. spice:: (__static-imply ...)
   .. spice:: (__tobool ...)
   .. spice:: (__typecall ...)
   .. builtin:: (__vector!= ...)
   .. spice:: (__vector% ...)
   .. builtin:: (__vector& ...)
   .. builtin:: (__vector* ...)
   .. builtin:: (__vector+ ...)
   .. builtin:: (__vector- ...)
   .. spice:: (__vector// ...)
   .. spice:: (__vector< ...)
   .. builtin:: (__vector<< ...)
   .. spice:: (__vector<= ...)
   .. builtin:: (__vector== ...)
   .. spice:: (__vector> ...)
   .. spice:: (__vector>= ...)
   .. spice:: (__vector>> ...)
   .. builtin:: (__vector^ ...)
   .. builtin:: (__vector| ...)
   .. spice:: (__| ...)
   .. inline:: (__~ self)
.. type:: intptr

   A plain type labeled ``u64`` of supertype `integer` and of storage type `u64`.

.. type:: list

   A plain type labeled ``List`` of storage type `_List<*>`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<-(List)``.
   .. spice:: (__.. ...)
   .. spice:: (__== ...)
   .. spice:: (__as ...)
   .. spice:: (__countof ...)
   .. inline:: (__repr self)
   .. spice:: (__typecall ...)
   .. spice:: (__unpack ...)
   .. inline:: (cons-sink self)
   .. spice:: (decons ...)
   .. compiledfn:: (dump ...)

      An external function of type ``List<-(List)``.
   .. compiledfn:: (join ...)

      An external function of type ``List<-(List List)``.
   .. compiledfn:: (next ...)

      An external function of type ``List<-(List)``.
   .. compiledfn:: (reverse ...)

      An external function of type ``List<-(List)``.
   .. fn:: (rjoin lside rside)
   .. fn:: (token-split expr token errmsg)
.. type:: nodefault

   An opaque type.

.. type:: noreturn

   An opaque type.

.. type:: opaquepointer

   An opaque type.

.. type:: package

   
   A symbol table of type `Scope` which holds configuration options and module
   contents. It is managed by the module import system.
   
   ``package.path`` holds a list of all search paths in the form of simple
   string patterns. Changing it alters the way modules are searched for in
   the next run stage.
   
   ``package.modules`` is another scope symbol table mapping full module
   paths to their contents. When a module is first imported, its contents
   are cached in this table. Subsequent imports of the same module will be
   resolved to these cached contents.

.. type:: pointer

   An opaque type.

   .. spice:: (__== ...)
   .. inline:: (__@ self index)
   .. spice:: (__as ...)
   .. spice:: (__call ...)
   .. inline:: (__getattr self key)
   .. spice:: (__hash ...)
   .. spice:: (__imply ...)
   .. inline:: (__toref self)
   .. spice:: (__typecall ...)
   .. inline:: (type T)
.. type:: rawstring

   A plain type labeled ``i8(*)`` of supertype `pointer` and of storage type `i8(*)`.

.. type:: real

   An opaque type of supertype `immutable`.

   .. spice:: (__!= ...)
   .. spice:: (__% ...)
   .. spice:: (__* ...)
   .. spice:: (__** ...)
   .. spice:: (__+ ...)
   .. spice:: (__- ...)
   .. spice:: (__/ ...)
   .. spice:: (__// ...)
   .. spice:: (__< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. spice:: (__as ...)
   .. spice:: (__hash ...)
   .. spice:: (__imply ...)
   .. inline:: (__neg self)
   .. inline:: (__rcp self)
   .. inline:: (__tobool self)
   .. inline:: (__typecall cls value)
   .. builtin:: (__vector!= ...)
   .. builtin:: (__vector% ...)
   .. builtin:: (__vector* ...)
   .. builtin:: (__vector** ...)
   .. builtin:: (__vector+ ...)
   .. builtin:: (__vector- ...)
   .. builtin:: (__vector/ ...)
   .. builtin:: (__vector< ...)
   .. builtin:: (__vector<= ...)
   .. builtin:: (__vector== ...)
   .. builtin:: (__vector> ...)
   .. builtin:: (__vector>= ...)
.. type:: string

   A plain type labeled ``String`` of supertype `opaquepointer` and of storage type `_String<*>`.

   .. spice:: (__!= ...)
   .. spice:: (__.. ...)
   .. spice:: (__< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. fn:: (__@ self i)
   .. spice:: (__as ...)
   .. compiledfn:: (__countof ...)

      An external function of type ``usize<-(String)``.
   .. inline:: (__hash self)
   .. spice:: (__imply ...)
   .. compiledfn:: (__lslice ...)

      An external function of type ``String<-(String usize)``.
   .. spice:: (__ras ...)
   .. compiledfn:: (__rslice ...)

      An external function of type ``String<-(String usize)``.
   .. compiledfn:: (buffer ...)

      An external function of type ``λ(i8(*) usize)<-(String)``.
   .. inline:: (collector maxsize)
   .. compiledfn:: (join ...)

      An external function of type ``String<-(String String)``.
   .. compiledfn:: (match? ...)

      An external function of type ``λ(bool i32 i32)<->Error(String String)``.
   .. inline:: (range self start end)
.. type:: tuple

   An opaque type of supertype `aggregate`.

   .. builtin:: (__@ ...)
   .. spice:: (__countof ...)
   .. builtin:: (__getattr ...)
   .. spice:: (__typecall ...)
   .. spice:: (__unpack ...)
   .. spice:: (packed ...)
   .. spice:: (packed-type ...)
   .. spice:: (type ...)
.. type:: type

   A plain type of supertype `opaquepointer` and of storage type `_type<*>`.

   .. compiledfn:: (@ ...)

      An external function of type ``Value<->Error(type Symbol)``.
   .. spice:: (__!= ...)
   .. spice:: (__< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. compiledfn:: (__@ ...)

      An external function of type ``type<->Error(type i32)``.
   .. spice:: (__call ...)
   .. spice:: (__countof ...)
   .. spice:: (__getattr ...)
   .. spice:: (__hash ...)
   .. spice:: (__toptr ...)
   .. inline:: (__toref self)
   .. compiledfn:: (alignof ...)

      An external function of type ``usize<->Error(type)``.
   .. compiledfn:: (bitcount ...)

      An external function of type ``i32<-(type)``.
   .. fn:: (change-element-type cls ET)
   .. fn:: (change-storage-class cls storage-class)
   .. spice:: (define-symbol ...)
   .. inline:: (define-symbols self values...)
   .. spice:: (dispatch-attr ...)
   .. compiledfn:: (docstring ...)

      An external function of type ``String<-(type Symbol)``.
   .. compiledfn:: (element-count ...)

      An external function of type ``i32<->Error(type)``.
   .. compiledfn:: (element@ ...)

      An external function of type ``type<->Error(type i32)``.
   .. inline:: (elements self)
   .. fn:: (function-pointer? cls)
   .. fn:: (function? cls)
   .. fn:: (immutable cls)
   .. inline:: (key-type self key)
   .. compiledfn:: (keyof ...)

      An external function of type ``λ(Symbol type)<-(type)``.
   .. compiledfn:: (kind ...)

      An external function of type ``i32<-(type)``.
   .. compiledfn:: (local@ ...)

      An external function of type ``Value<->Error(type Symbol)``.
   .. fn:: (mutable cls)
   .. fn:: (mutable& cls)
   .. compiledfn:: (offsetof ...)

      An external function of type ``usize<->Error(type i32)``.
   .. compiledfn:: (opaque? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (plain? ...)

      An external function of type ``bool<-(type)``.
   .. fn:: (pointer->refer-type cls)
   .. fn:: (pointer-storage-class cls)
   .. fn:: (pointer? cls)
   .. spice:: (raises ...)
   .. fn:: (readable? cls)
   .. fn:: (refer->pointer-type cls)
   .. compiledfn:: (refer? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (return-type ...)

      An external function of type ``λ(type type)<-(type)``.
   .. compiledfn:: (set-docstring ...)

      An external function of type ``void<-(type Symbol String)``.
   .. inline:: (set-opaque type)
   .. inline:: (set-plain-storage type storage-type)
   .. inline:: (set-storage type storage-type)
   .. spice:: (set-symbol ...)
   .. inline:: (set-symbols self values...)
   .. compiledfn:: (signed? ...)

      An external function of type ``bool<-(type)``.
   .. compiledfn:: (sizeof ...)

      An external function of type ``usize<->Error(type)``.
   .. compiledfn:: (storageof ...)

      An external function of type ``type<->Error(type)``.
   .. compiledfn:: (string ...)

      An external function of type ``String<-(type)``.
   .. fn:: (strip-pointer-storage-class cls)
   .. compiledfn:: (strip-qualifiers ...)

      An external function of type ``type<-(type)``.
   .. compiledfn:: (superof ...)

      An external function of type ``type<-(type)``.
   .. inline:: (symbols self)
   .. compiledfn:: (unique-type ...)

      An external function of type ``type<-(type i32)``.
   .. compiledfn:: (variadic? ...)

      An external function of type ``bool<-(type)``.
   .. inline:: (view-type self id)
   .. fn:: (writable? cls)
.. type:: typename

   An opaque type.

   .. spice:: (__!= ...)
   .. spice:: (__= ...)
   .. spice:: (__methodcall ...)
   .. spice:: (__toptr ...)
   .. spice:: (__typecall ...)
   .. compiledfn:: (type ...)

      An external function of type ``type<->Error(String type)``.
.. type:: u16

   A plain type of supertype `integer` and of storage type `u16`.

.. type:: u32

   A plain type of supertype `integer` and of storage type `u32`.

.. type:: u64

   A plain type of supertype `integer` and of storage type `u64`.

.. type:: u8

   A plain type of supertype `integer` and of storage type `u8`.

.. type:: union

   An opaque type.

.. type:: usize

   A plain type of supertype `integer` and of storage type `u64`.

.. type:: vector

   An opaque type of supertype `immutable`.

   .. spice:: (__!= ...)
   .. spice:: (__% ...)
   .. spice:: (__& ...)
   .. spice:: (__* ...)
   .. spice:: (__** ...)
   .. spice:: (__+ ...)
   .. spice:: (__- ...)
   .. spice:: (__/ ...)
   .. spice:: (__// ...)
   .. spice:: (__< ...)
   .. spice:: (__<< ...)
   .. spice:: (__<= ...)
   .. spice:: (__== ...)
   .. spice:: (__> ...)
   .. spice:: (__>= ...)
   .. spice:: (__>> ...)
   .. inline:: (__@ self index)
   .. spice:: (__^ ...)
   .. spice:: (__countof ...)
   .. spice:: (__lslice ...)
   .. spice:: (__rslice ...)
   .. spice:: (__typecall ...)
   .. spice:: (__unpack ...)
   .. spice:: (__| ...)
   .. spice:: (smear ...)
   .. inline:: (type element-type size)
.. type:: void

   An opaque type of supertype `Arguments`.

.. type:: voidstar

   A plain type labeled ``void<*>`` of supertype `pointer` and of storage type `void<*>`.

.. inline:: (%= lhs rhs)
.. inline:: (&= lhs rhs)
.. inline:: (*= lhs rhs)
.. inline:: (+= lhs rhs)
.. inline:: (-= lhs rhs)
.. inline:: (..= lhs rhs)
.. inline:: (//= lhs rhs)
.. inline:: (/= lhs rhs)
.. inline:: (<<= lhs rhs)
.. inline:: (>>= lhs rhs)
.. inline:: (^= lhs rhs)
.. inline:: (|= lhs rhs)
.. fn:: (Value-none? value)
.. inline:: (aggregate-type-constructor start f)
.. fn:: (all? v)
.. fn:: (any? v)
.. fn:: (as-converter vT T static?)
.. fn:: (autoboxer T x)
.. inline:: (balanced-binary-op-dispatch symbol rsymbol friendly-op-name)
.. fn:: (balanced-binary-operation args symbol rsymbol friendly-op-name)
.. fn:: (balanced-binary-operator symbol rsymbol lhsT rhsT lhs-static? rhs-static?)
   
   for an operation performed on two argument types, of which either
   type can provide a suitable candidate, return a matching operator.
   This function only works inside a spice macro.
.. inline:: (balanced-lvalue-binary-op-dispatch symbol friendly-op-name)
.. fn:: (balanced-lvalue-binary-operation args symbol friendly-op-name)
.. fn:: (balanced-lvalue-binary-operator symbol lhsT rhsT rhs-static?)
   
   for an operation performed on two argument types, of which only the
   left type type can provide a suitable candidate, return a matching operator.
   This function only works inside a spice macro.
.. fn:: (bin value)
.. fn:: (binary-op-error friendly-op-name lhsT rhsT)
.. fn:: (binary-operator symbol lhsT rhsT)
   
   for an operation performed on two argument types, of which only
   the left type can provide a suitable candidate, find a matching
   operator function. This function only works inside a spice macro.
.. fn:: (binary-operator-r rsymbol lhsT rhsT)
   
   for an operation performed on two argument types, of which only
   the right type can provide a suitable candidate, find a matching
   operator function. This function only works inside a spice macro.
.. fn:: (box-integer value)
.. fn:: (box-pointer value)
.. inline:: (box-spice-macro l)
.. fn:: (box-symbol value)
.. fn:: (build-typify-function f)
.. fn:: (cast-converter symbol rsymbol vT T)
   
   for two given types, find a matching conversion function
   this function only works inside a spice macro
.. inline:: (cast-error intro-string vT T)
.. fn:: (check-count count mincount maxcount)
.. inline:: (clamp x mn mx)
.. fn:: (clone-scope-contents a b)
   
   Join two scopes ``a`` and ``b`` into a new scope so that the
   root of ``a`` descends from ``b``.
.. fn:: (compare-type args f)
.. inline:: (convert-assert-args args cond msg)
.. fn:: (dec value)
.. inline:: (defer-type ...)
.. fn:: (dispatch-and-or args flip)
.. fn:: (dots-to-slashes pattern)
.. fn:: (dotted-symbol? env head)
.. inline:: (empty? value)
.. inline:: (enumerate x)
.. fn:: (error msg)
.. fn:: (error@ anchor traceback-msg error-msg)
   
   usage example::
       error@ ('anchor value) "while checking parameter" "error in value"
.. fn:: (error@+ error anchor traceback-msg)
   
   usage example::
       except (err)
           error@+ err ('anchor value) "while processing stream"
.. fn:: (exec-module expr eval-scope)
.. fn:: (expand-and-or expr f)
.. fn:: (expand-apply expr)
.. fn:: (expand-define expr)
.. fn:: (expand-define-infix args scope order)
.. fn:: (expand-infix-let expr)
.. inline:: (extern-new name T attrs...)
.. fn:: (extract-integer value)
.. fn:: (extract-name-params-body expr)
.. fn:: (extract-single-arg args)
.. fn:: (extract-single-type-arg args)
.. inline:: (floordiv a b)
.. inline:: (function->SpiceMacro f)
.. inline:: (gen-allocator-sugar name copyf newf)
.. inline:: (gen-cast-op f str)
.. inline:: (gen-cast? converterf)
.. inline:: (gen-match-block-parser handle-case)
.. fn:: (gen-match-matcher failfunc expr scope cond)
   
   features:
   <constant> -> (input == <constant>)
   (or <expr_a> <expr_b>) -> (or <expr_a> <expr_b>)
   
   TODO:
   (: x T) -> ((typeof input) == T), let x = input
   <unknown symbol> -> unpack as symbol
.. fn:: (gen-or-matcher failfunc expr scope params)
.. fn:: (gen-sugar-matcher failfunc expr scope params)
.. fn:: (gen-vector-reduction f v sz)
.. fn:: (get-ifx-op env op)
.. fn:: (get-ifx-symbol name)
.. fn:: (has-infix-ops? infix-table expr)
.. fn:: (hex value)
.. fn:: (imply-converter vT T static?)
.. inline:: (infix-op pred)
.. fn:: (infix-op-ge infix-table token prec)
.. fn:: (infix-op-gt infix-table token prec)
.. fn:: (integer->string value base)
.. fn:: (integer-as vT T)
.. fn:: (integer-imply vT T)
.. fn:: (integer-static-imply vT T)
.. fn:: (integer-tobool args)
.. fn:: (load-module module-name module-path opts...)
.. fn:: (ltr-multiop args target mincount)
.. inline:: (make-const-type-property-function func)
.. inline:: (make-const-value-property-function func)
.. inline:: (make-expand-and-or f)
.. inline:: (make-expand-define-infix order)
.. inline:: (make-inplace-let-op op)
.. inline:: (make-inplace-op op)
.. fn:: (make-module-path pattern name)
.. inline:: (make-unpack-function extractf)
.. inline:: (memo f)
.. inline:: (memoize f)
.. fn:: (merge-scope-symbols source target filter)
.. fn:: (next-head? next)
.. fn:: (nodefault? x)
.. fn:: (oct value)
.. fn:: (operator-valid? value)
.. fn:: (patterns-from-namestr base-dir namestr)
.. fn:: (pointer-as vT T)
.. fn:: (pointer-imply vT T)
.. fn:: (pointer-type-imply? src dest)
.. fn:: (powi base exponent)
.. inline:: (print values...)
.. fn:: (ptrcmp!= t1 t2)
.. fn:: (ptrcmp== t1 t2)
.. inline:: (quasiquote-any x)
.. fn:: (quasiquote-list x)
.. inline:: (range a b c)
.. fn:: (real-as vT T)
.. fn:: (real-imply vT T)
.. fn:: (require-from base-dir name)
.. inline:: (rrange a b c)
   
   same as range, but iterates range in reverse; arguments are passed
   in the same format, so rrange can act as a drop-in replacement for range.
.. fn:: (rtl-infix-op-eq infix-table token prec)
.. fn:: (rtl-multiop args target mincount)
.. inline:: (runtime-aggregate-type-constructor f)
.. inline:: (safe-integer-cast self T)
.. fn:: (sc_argument_list_join a b)
.. fn:: (sc_argument_list_join_values a b...)
.. inline:: (sc_argument_list_map_filter_new maxN mapf)
.. inline:: (sc_argument_list_map_new N mapf)
.. inline:: (select-op-macro sop uop fop numargs)
.. inline:: (signed-vector-binary-op sf uf)
.. inline:: (simple-binary-op f)
   
   for cases where the type only interacts with itself
.. inline:: (simple-folding-autotype-binary-op f unboxer)
.. inline:: (simple-folding-autotype-signed-binary-op sf uf unboxer)
.. inline:: (simple-folding-binary-op f unboxer boxer)
.. inline:: (simple-folding-signed-binary-op sf uf unboxer boxer)
.. inline:: (simple-signed-binary-op sf uf)
.. inline:: (slice value start end)
.. inline:: (spice-binary-op-macro f)
   
   to be used for binary operators of which either type can
   provide an operation. returns a callable operator (f lhs rhs) that
   performs the operation or no arguments if the operation can not be
   performed.
.. inline:: (spice-cast-macro f)
   
   to be used for __as, __ras, __imply and __rimply
   returns a callable converter (f value) that performs the cast or
   no arguments if the cast can not be performed.
.. inline:: (spice-converter-macro f)
   
   to be used for converter that need to do additional
   dispatch, e.g. do something else when the value is a constant
   returns a quote that performs the cast (f value T)
.. inline:: (spice-macro l)
.. fn:: (split-dotted-symbol name)
.. fn:: (string@ self i)
.. inline:: (sugar-block-scope-macro f)
.. inline:: (sugar-macro f)
.. inline:: (sugar-scope-macro f)
.. fn:: (swap a b)
   
   safely exchanges the contents of two references
.. inline:: (type-comparison-func f)
.. inline:: (type-factory f)
.. inline:: (typeinit ...)
.. inline:: (unary-op-dispatch symbol friendly-op-name)
.. fn:: (unary-op-error friendly-op-name T)
.. fn:: (unary-operation args symbol friendly-op-name)
.. fn:: (unary-operator symbol T)
   
   for an operation performed on one variable argument type, find a
   matching operator function. This function only works inside a spice
   macro.
.. inline:: (unary-or-balanced-binary-op-dispatch usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
.. fn:: (unary-or-balanced-binary-operation args usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
.. inline:: (unary-or-unbalanced-binary-op-dispatch usymbol ufriendly-op-name symbol rtype friendly-op-name)
.. fn:: (unary-or-unbalanced-binary-operation args usymbol ufriendly-op-name symbol rtype friendly-op-name)
.. inline:: (unbalanced-binary-op-dispatch symbol rtype friendly-op-name)
.. fn:: (unbalanced-binary-operation args symbol rtype friendly-op-name)
.. inline:: (unbox value T)
.. inline:: (unbox-integer value T)
.. inline:: (unbox-pointer value T)
.. inline:: (unbox-symbol value T)
.. fn:: (unbox-verify value wantT)
.. fn:: (uncomma l)
   
   uncomma list l, wrapping all comma separated symbols as new lists
   example::
   
       (uncomma '(a , b c d , e f , g h)) -> '(a (b c d) (e f) (g h))
.. fn:: (unpack-infix-op op)
.. fn:: (unpack2 args)
.. inline:: (va-join a...)
.. fn:: (value-as vT T expr)
.. inline:: (vector-binary-op-dispatch symbol)
.. fn:: (vector-binary-operator symbol lhsT rhsT)
.. fn:: (verify-count count mincount maxcount)
.. sugar:: (. ...)
.. sugar:: (:: ...)
.. sugar:: (:= ...)
.. sugar:: (<- ...)
.. sugar:: (@@ ...)
.. sugar:: (and ...)
.. sugar:: (as:= ...)
.. sugar:: (assert ...)
.. sugar:: (bind ...)
.. sugar:: (chain-typed-symbol-handler ...)
.. sugar:: (decorate-fn ...)
.. sugar:: (decorate-inline ...)
.. sugar:: (decorate-let ...)
.. sugar:: (decorate-struct ...)
.. sugar:: (decorate-typedef ...)
.. sugar:: (decorate-vvv ...)
.. sugar:: (define ...)
.. sugar:: (define-infix< ...)
.. sugar:: (define-infix> ...)
.. sugar:: (define-sugar-block-scope-macro ...)
.. sugar:: (define-sugar-macro ...)
.. sugar:: (define-sugar-scope-macro ...)
.. sugar:: (fn... ...)
.. sugar:: (fold (state ... _:= init...) _:for name ... _:in gen body...)

   This is a combination of the `loop` and `for` forms. It enumerates all
   elements in collection or sequence `gen`, unpacking each element and
   binding its arguments to the names defined by `name ...`, while
   the loop state `state ...` is initialized from `init...`.

   Similar to `loop`, the body expression must return the next state of
   the loop. The state of `gen` is transparently maintained and does not
   have to be managed.

   Unlike `for`, `fold` requires both calls to ``break`` and ``continue``
   to pass a state compatible with `state ...`. Otherwise they serve
   the same function.

   Usage example::

        # add numbers from 0 to 9, skipping number 5, and print the result
        print
            fold (sum = 0) for i in (range 100)
                if (i == 10)
                    # abort the loop
                    break sum
                if (i == 5)
                    # skip this index
                    continue sum
                # continue with the next state for sum
                sum + i

.. sugar:: (fold-locals ...)
.. sugar:: (for name ... _:in gen body...)

Defines a loop that enumerates all elements in collection or sequence
`gen`, unpacking each element and binding its arguments to the names
defined by `name ...`.

`gen` must either be of type `Generator` or provide a cast to
`Generator`.

Within the loop body, special forms ``break`` and ``continue`` can be used
to abort the loop early or skip ahead to the next element. The loop
will always evaluate to no arguments.

For a loop form that permits you to maintain additional state and break
with a value, see `fold`.

Usage example::

    # print numbers from 0 to 9, skipping number 5
    for i in (range 100)
        if (i == 10)
            # abort the loop
            break;
        if (i == 5)
            # skip this index
            continue;
        print i

.. sugar:: (from ...)
.. sugar:: (global ...)
.. sugar:: (import ...)
.. sugar:: (include ...)
.. sugar:: (inline... ...)
.. sugar:: (local ...)
.. sugar:: (locals ...)
   
   Export locals as a chain of up to two new scopes: a scope that contains
   all the constant values in the immediate scope, and a scope that contains
   the runtime values. If all values in the scope are constant, then the
   resulting scope will also be constant.
.. sugar:: (match ...)
.. sugar:: (or ...)
.. sugar:: (qq ...)
.. sugar:: (spice ...)
.. sugar:: (static-assert ...)
.. sugar:: (static-if ...)
.. sugar:: (static-match ...)
.. sugar:: (sugar ...)
.. sugar:: (sugar-eval ...)
.. sugar:: (sugar-if ...)
.. sugar:: (sugar-match ...)
.. sugar:: (sugar-set-scope! ...)
.. sugar:: (typedef ...)
   
   a type declaration syntax; when the name is a string, the type is declared
   at runtime.
.. sugar:: (typedef+ ...)
.. sugar:: (unlet ...)
.. sugar:: (using ...)
.. sugar:: (va-option ...)
.. sugar:: (vvv ...)
.. sugar:: (while ...)
.. builtin:: (? ...)
.. builtin:: (_ ...)
.. builtin:: (Image-query-levels ...)
.. builtin:: (Image-query-lod ...)
.. builtin:: (Image-query-samples ...)
.. builtin:: (Image-query-size ...)
.. builtin:: (Image-read ...)
.. builtin:: (Image-texel-pointer ...)
.. builtin:: (Image-write ...)
.. builtin:: (acos ...)
.. builtin:: (acosh ...)
.. builtin:: (add ...)
.. builtin:: (add-nsw ...)
.. builtin:: (add-nuw ...)
.. builtin:: (alloca ...)
.. builtin:: (alloca-array ...)
.. builtin:: (ashr ...)
.. builtin:: (asin ...)
.. builtin:: (asinh ...)
.. builtin:: (assign ...)
.. builtin:: (atan ...)
.. builtin:: (atan2 ...)
.. builtin:: (atanh ...)
.. builtin:: (atomic ...)
.. builtin:: (atomicrmw ...)
.. builtin:: (band ...)
.. builtin:: (bitcast ...)
.. builtin:: (bnand ...)
.. builtin:: (bor ...)
.. builtin:: (branch ...)
.. builtin:: (break ...)
.. builtin:: (bxor ...)
.. builtin:: (call ...)
.. builtin:: (ceil ...)
.. builtin:: (cmpxchg ...)
.. builtin:: (copy ...)
.. builtin:: (cos ...)
.. builtin:: (cosh ...)
.. builtin:: (cross ...)
.. builtin:: (degrees ...)
.. builtin:: (deref ...)
.. builtin:: (discard ...)
.. builtin:: (distance ...)
.. builtin:: (do ...)
.. builtin:: (dropped? ...)
.. builtin:: (dump ...)
.. builtin:: (dump-debug ...)
.. builtin:: (dump-spice ...)
.. builtin:: (dump-template ...)
.. builtin:: (dump-uniques ...)
.. builtin:: (dupe ...)
.. builtin:: (embed ...)
.. builtin:: (exp ...)
.. builtin:: (exp2 ...)
.. builtin:: (extractelement ...)
.. builtin:: (extractvalue ...)
.. builtin:: (fabs ...)
.. builtin:: (fadd ...)
.. builtin:: (fcmp!=o ...)
.. builtin:: (fcmp!=u ...)
.. builtin:: (fcmp-ord ...)
.. builtin:: (fcmp-uno ...)
.. builtin:: (fcmp<=o ...)
.. builtin:: (fcmp<=u ...)
.. builtin:: (fcmp<o ...)
.. builtin:: (fcmp<u ...)
.. builtin:: (fcmp==o ...)
.. builtin:: (fcmp==u ...)
.. builtin:: (fcmp>=o ...)
.. builtin:: (fcmp>=u ...)
.. builtin:: (fcmp>o ...)
.. builtin:: (fcmp>u ...)
.. builtin:: (fdiv ...)
.. builtin:: (floor ...)
.. builtin:: (fma ...)
.. builtin:: (fmix ...)
.. builtin:: (fmul ...)
.. builtin:: (fn ...)
.. builtin:: (fpext ...)
.. builtin:: (fptosi ...)
.. builtin:: (fptoui ...)
.. builtin:: (fptrunc ...)
.. builtin:: (fract ...)
.. builtin:: (free ...)
.. builtin:: (frem ...)
.. builtin:: (frexp ...)
.. builtin:: (fsign ...)
.. builtin:: (fsub ...)
.. builtin:: (getelementptr ...)
.. builtin:: (getelementref ...)
.. builtin:: (hide-traceback ...)
.. builtin:: (icmp!= ...)
.. builtin:: (icmp<=s ...)
.. builtin:: (icmp<=u ...)
.. builtin:: (icmp<s ...)
.. builtin:: (icmp<u ...)
.. builtin:: (icmp== ...)
.. builtin:: (icmp>=s ...)
.. builtin:: (icmp>=u ...)
.. builtin:: (icmp>s ...)
.. builtin:: (icmp>u ...)
.. builtin:: (if ...)
.. builtin:: (indirect-let ...)
.. builtin:: (inline ...)
.. builtin:: (insertelement ...)
.. builtin:: (insertvalue ...)
.. builtin:: (inttoptr ...)
.. builtin:: (inversesqrt ...)
.. builtin:: (itrunc ...)
.. builtin:: (label ...)
.. builtin:: (ldexp ...)
.. builtin:: (length ...)
.. builtin:: (let ...)
.. builtin:: (load ...)
.. builtin:: (log ...)
.. builtin:: (log2 ...)
.. builtin:: (loop ...)
.. builtin:: (lose ...)
.. builtin:: (lshr ...)
.. builtin:: (malloc ...)
.. builtin:: (malloc-array ...)
.. builtin:: (merge ...)
.. builtin:: (move ...)
.. builtin:: (mul ...)
.. builtin:: (mul-nsw ...)
.. builtin:: (mul-nuw ...)
.. builtin:: (normalize ...)
.. builtin:: (nullof ...)
.. builtin:: (powf ...)
.. builtin:: (ptrtoint ...)
.. builtin:: (ptrtoref ...)
.. builtin:: (radians ...)
.. builtin:: (raise ...)
.. builtin:: (raising ...)
.. builtin:: (rawcall ...)
.. builtin:: (reftoptr ...)
.. builtin:: (repeat ...)
.. builtin:: (return ...)
.. builtin:: (returning ...)
.. builtin:: (round ...)
.. builtin:: (roundeven ...)
.. builtin:: (run-stage ...)
.. builtin:: (sample ...)
.. builtin:: (sdiv ...)
.. builtin:: (set-execution-mode ...)
.. builtin:: (sext ...)
.. builtin:: (shl ...)
.. builtin:: (shufflevector ...)
.. builtin:: (sin ...)
.. builtin:: (sinh ...)
.. builtin:: (sitofp ...)
.. builtin:: (smax ...)
.. builtin:: (smin ...)
.. builtin:: (smoothstep ...)
.. builtin:: (spice-quote ...)
.. builtin:: (spice-unquote ...)
.. builtin:: (spice-unquote-arguments ...)
.. builtin:: (sqrt ...)
.. builtin:: (square-list ...)
.. builtin:: (srem ...)
.. builtin:: (ssign ...)
.. builtin:: (step ...)
.. builtin:: (store ...)
.. builtin:: (sub ...)
.. builtin:: (sub-nsw ...)
.. builtin:: (sub-nuw ...)
.. builtin:: (sugar-log ...)
.. builtin:: (sugar-quote ...)
.. builtin:: (swapvalue ...)
.. builtin:: (switch ...)
.. builtin:: (tan ...)
.. builtin:: (tanh ...)
.. builtin:: (trunc ...)
.. builtin:: (try ...)
.. builtin:: (typeof ...)
.. builtin:: (udiv ...)
.. builtin:: (uitofp ...)
.. builtin:: (umax ...)
.. builtin:: (umin ...)
.. builtin:: (undef ...)
.. builtin:: (unique-visible? ...)
.. builtin:: (unreachable ...)
.. builtin:: (urem ...)
.. builtin:: (va-countof ...)
.. builtin:: (view ...)
.. builtin:: (viewing ...)
.. builtin:: (volatile ...)
.. builtin:: (volatile-load ...)
.. builtin:: (volatile-store ...)
.. builtin:: (xchg ...)
.. builtin:: (zext ...)
.. spice:: (% ...)
.. spice:: (& ...)
.. spice:: (* ...)
.. spice:: (+ ...)
.. spice:: (- ...)
.. spice:: (/ ...)
.. spice:: (< ...)
.. spice:: (= ...)
.. spice:: (> ...)
.. spice:: (@ ...)
.. spice:: (^ ...)
.. spice:: (| ...)
.. spice:: (~ ...)
.. spice:: (!= ...)
.. spice:: (&? value)

   Returns `true` if `value` is a reference, otherwise `false`.

.. spice:: (** ...)
.. spice:: (.. ...)
.. spice:: (// ...)
.. spice:: (<< ...)
.. spice:: (<= ...)
.. spice:: (== ...)
.. spice:: (>= ...)
.. spice:: (>> ...)
.. spice:: (_static-compile ...)
.. spice:: (_static-compile-glsl ...)
.. spice:: (_static-compile-spirv ...)
.. spice:: (Closure->Collector ...)
.. spice:: (Closure->Generator ...)
.. spice:: (abs ...)
.. spice:: (alignof ...)
.. spice:: (and-branch ...)
   
   The type of the `null` constant. This type is uninstantiable.
.. spice:: (append-to-scope ...)
.. spice:: (append-to-type ...)
.. spice:: (argumentsof ...)
.. spice:: (arrayof ...)
.. spice:: (as ...)
.. spice:: (as? ...)
.. spice:: (bindingof ...)
.. spice:: (bitcountof ...)
.. spice:: (coerce-call-arguments ...)
.. spice:: (cons ...)
.. spice:: (const.add.i32.i32 ...)
.. spice:: (const.icmp<=.i32.i32 ...)
.. spice:: (constant? ...)
.. spice:: (countof ...)
.. spice:: (decons ...)
.. spice:: (defer ...)
.. spice:: (drop ...)
.. spice:: (elementof ...)
.. spice:: (elementsof ...)
.. spice:: (extern ...)
.. spice:: (forward-repr ...)
.. spice:: (gen-union-extractvalue ...)
.. spice:: (getattr ...)
.. spice:: (hash-storage ...)
.. spice:: (hash1 ...)
.. spice:: (imply ...)
.. spice:: (imply? ...)
.. spice:: (integer->integer ...)
.. spice:: (integer->real ...)
.. spice:: (key ...)
.. spice:: (keyof ...)
.. spice:: (list-constructor ...)
.. spice:: (locationof ...)
.. spice:: (lslice ...)
.. spice:: (max ...)
.. spice:: (memocall ...)
.. spice:: (min ...)
.. spice:: (mutable ...)
.. spice:: (none? ...)
.. spice:: (not ...)
.. spice:: (offsetof ...)
.. spice:: (opaque ...)
.. spice:: (or-branch ...)
.. spice:: (overloaded-fn-append ...)
.. spice:: (packedtupleof ...)
.. spice:: (parse-compile-flags ...)
.. spice:: (pow ...)
.. spice:: (private ...)
.. spice:: (protect ...)
.. spice:: (qualifiersof ...)
.. spice:: (raises ...)
.. spice:: (real->integer ...)
.. spice:: (real->real ...)
.. spice:: (report ...)
.. spice:: (repr ...)
.. spice:: (returnof ...)
.. spice:: (rslice ...)
.. spice:: (sabs ...)
.. spice:: (safe-shl ...)
.. spice:: (sign ...)
.. spice:: (signed? ...)
.. spice:: (sizeof ...)
.. spice:: (static-branch ...)
.. spice:: (static-error ...)
.. spice:: (static-integer->integer ...)
.. spice:: (static-integer->real ...)
.. spice:: (static-typify ...)
.. spice:: (storagecast ...)
.. spice:: (storageof ...)
.. spice:: (superof ...)
.. spice:: (tostring ...)
.. spice:: (tupleof ...)
.. spice:: (type!= ...)
.. spice:: (type< ...)
.. spice:: (type<= ...)
.. spice:: (type== ...)
.. spice:: (type> ...)
.. spice:: (type>= ...)
.. spice:: (typify ...)
.. spice:: (union-storage-type ...)
.. spice:: (union-storageof ...)
.. spice:: (uniqueof ...)
.. spice:: (unpack ...)
.. spice:: (unqualified ...)
.. spice:: (va-append-va ...)
   
    (va-append-va (inline () (_ b ...)) a...) -> a... b...
.. spice:: (va-empty? ...)
.. spice:: (va-lfold ...)
.. spice:: (va-lifold ...)
.. spice:: (va-map f ...)

   Filter each argument in `...` through `f` and return the resulting list
   of arguments. Arguments where `f` returns void are filtered from the
   result.

.. spice:: (va-option-branch ...)
.. spice:: (va-range a (? b))

   If `b` is not specified, returns a sequence of integers from zero to `b`,
   otherwise a sequence of integers from `a` to `b`.

.. spice:: (va-rfold ...)
.. spice:: (va-rifold ...)
.. spice:: (va-split ...)
   
    (va-split n a...) -> (inline () a...[n .. (va-countof a...)-1]) a...[0 .. n-1]
.. spice:: (va-unnamed ...)
   
    filter all keyed values
.. spice:: (va@ ...)
.. spice:: (vector-reduce ...)
.. spice:: (vectorof ...)
.. spice:: (viewof ...)
.. spice:: (wrap-if-not-run-stage ...)
.. spice:: (zip ...)
.. compiledfn:: (compiler-version ...)

   An external function of type ``λ(i32 i32 i32)<-()``.
.. compiledfn:: (default-styler ...)

   An external function of type ``String<-(Symbol String)``.
.. compiledfn:: (exit ...)

   An external function of type ``noreturn<-(i32)``.
.. compiledfn:: (function->SugarMacro ...)

   A compiled function of type ``SugarMacro<-(λ(List Scope)<->Error(List Scope)<*>)``.
.. compiledfn:: (globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (io-write! ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (launch-args ...)

   An external function of type ``λ(i32 i8(*)(*))<-()``.
.. compiledfn:: (list-handler ...)

   A compiled function of type ``λ(List Scope)<->Error(List Scope)``.
.. compiledfn:: (list-load ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (list-parse ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (load-library ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (load-object ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (parse-infix-expr ...)

   A compiled function of type ``λ(Value List)<->Error(Scope Value List i32)``.
.. compiledfn:: (realpath ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_abort ...)

   An external function of type ``noreturn<-()``.
.. compiledfn:: (sc_anchor_column ...)

   An external function of type ``i32<-(Anchor)``.
.. compiledfn:: (sc_anchor_lineno ...)

   An external function of type ``i32<-(Anchor)``.
.. compiledfn:: (sc_anchor_offset ...)

   An external function of type ``Anchor<-(Anchor i32)``.
.. compiledfn:: (sc_anchor_path ...)

   An external function of type ``Symbol<-(Anchor)``.
.. compiledfn:: (sc_argcount ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_argument_list_new ...)

   An external function of type ``Value<-(i32 Value(*))``.
.. compiledfn:: (sc_arguments_type ...)

   An external function of type ``type<-(i32 type(*))``.
.. compiledfn:: (sc_arguments_type_argcount ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_arguments_type_getarg ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_arguments_type_join ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_array_type ...)

   An external function of type ``type<->Error(type usize)``.
.. compiledfn:: (sc_basename ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_cache_misses ...)

   An external function of type ``i32<-()``.
.. compiledfn:: (sc_call_append_argument ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_call_is_rawcall ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_call_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_call_set_rawcall ...)

   An external function of type ``void<-(Value bool)``.
.. compiledfn:: (sc_closure_get_context ...)

   An external function of type ``Value<-(Closure)``.
.. compiledfn:: (sc_closure_get_docstring ...)

   An external function of type ``String<-(Closure)``.
.. compiledfn:: (sc_closure_get_template ...)

   An external function of type ``Value<-(Closure)``.
.. compiledfn:: (sc_compile ...)

   An external function of type ``Value<->Error(Value u64)``.
.. compiledfn:: (sc_compile_glsl ...)

   An external function of type ``String<->Error(i32 Symbol Value u64)``.
.. compiledfn:: (sc_compile_object ...)

   An external function of type ``void<->Error(String i32 String Scope u64)``.
.. compiledfn:: (sc_compile_spirv ...)

   An external function of type ``String<->Error(Symbol Value u64)``.
.. compiledfn:: (sc_compiler_version ...)

   An external function of type ``λ(i32 i32 i32)<-()``.
.. compiledfn:: (sc_const_aggregate_new ...)

   An external function of type ``Value<-(type i32 Value(*))``.
.. compiledfn:: (sc_const_extract_at ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_const_int_extract ...)

   An external function of type ``u64<-(Value)``.
.. compiledfn:: (sc_const_int_extract_word ...)

   An external function of type ``u64<-(Value i32)``.
.. compiledfn:: (sc_const_int_new ...)

   An external function of type ``Value<-(type u64)``.
.. compiledfn:: (sc_const_int_words_new ...)

   An external function of type ``Value<-(type i32 u64(*))``.
.. compiledfn:: (sc_const_null_new ...)

   An external function of type ``Value<->Error(type)``.
.. compiledfn:: (sc_const_pointer_extract ...)

   An external function of type ``void<*><-(Value)``.
.. compiledfn:: (sc_const_pointer_new ...)

   An external function of type ``Value<-(type void<*>)``.
.. compiledfn:: (sc_const_real_extract ...)

   An external function of type ``f64<-(Value)``.
.. compiledfn:: (sc_const_real_new ...)

   An external function of type ``Value<-(type f64)``.
.. compiledfn:: (sc_default_styler ...)

   An external function of type ``String<-(Symbol String)``.
.. compiledfn:: (sc_default_target_triple ...)

   An external function of type ``String<-()``.
.. compiledfn:: (sc_dirname ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_dump_error ...)

   An external function of type ``void<-(Error)``.
.. compiledfn:: (sc_empty_argument_list ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_enter_solver_cli ...)

   An external function of type ``void<-()``.
.. compiledfn:: (sc_error_append_calltrace ...)

   An external function of type ``void<-(Error Value)``.
.. compiledfn:: (sc_error_new ...)

   An external function of type ``Error<-(String)``.
.. compiledfn:: (sc_eval ...)

   An external function of type ``Value<->Error(Anchor List Scope)``.
.. compiledfn:: (sc_eval_inline ...)

   An external function of type ``Anchor<->Error(Value List Scope)``.
.. compiledfn:: (sc_eval_stage ...)

   An external function of type ``Value<->Error(Anchor List Scope)``.
.. compiledfn:: (sc_exit ...)

   An external function of type ``noreturn<-(i32)``.
.. compiledfn:: (sc_expand ...)

   An external function of type ``λ(Value List Scope)<->Error(Value List Scope)``.
.. compiledfn:: (sc_expression_append ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_expression_new ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_expression_set_scoped ...)

   An external function of type ``void<-(Value)``.
.. compiledfn:: (sc_extract_argument_list_new ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_extract_argument_new ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_format_error ...)

   An external function of type ``String<-(Error)``.
.. compiledfn:: (sc_format_message ...)

   An external function of type ``String<-(Anchor String)``.
.. compiledfn:: (sc_function_type ...)

   An external function of type ``type<-(type i32 type(*))``.
.. compiledfn:: (sc_function_type_is_variadic ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_function_type_raising ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_function_type_return_type ...)

   An external function of type ``λ(type type)<-(type)``.
.. compiledfn:: (sc_get_globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_get_original_globals ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_getarg ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_getarglist ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_global_binding ...)

   An external function of type ``i32<->Error(Value)``.
.. compiledfn:: (sc_global_descriptor_set ...)

   An external function of type ``i32<->Error(Value)``.
.. compiledfn:: (sc_global_location ...)

   An external function of type ``i32<->Error(Value)``.
.. compiledfn:: (sc_global_new ...)

   An external function of type ``Value<-(Symbol type u32 Symbol)``.
.. compiledfn:: (sc_global_set_binding ...)

   An external function of type ``void<->Error(Value i32)``.
.. compiledfn:: (sc_global_set_constructor ...)

   An external function of type ``void<->Error(Value Value)``.
.. compiledfn:: (sc_global_set_descriptor_set ...)

   An external function of type ``void<->Error(Value i32)``.
.. compiledfn:: (sc_global_set_initializer ...)

   An external function of type ``void<->Error(Value Value)``.
.. compiledfn:: (sc_global_set_location ...)

   An external function of type ``void<->Error(Value i32)``.
.. compiledfn:: (sc_global_storage_class ...)

   An external function of type ``Symbol<->Error(Value)``.
.. compiledfn:: (sc_hash ...)

   An external function of type ``u64<-(u64 usize)``.
.. compiledfn:: (sc_hash2x64 ...)

   An external function of type ``u64<-(u64 u64)``.
.. compiledfn:: (sc_hashbytes ...)

   An external function of type ``u64<-(i8(*) usize)``.
.. compiledfn:: (sc_identity ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_if_append_else_clause ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_if_append_then_clause ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_if_new ...)

   An external function of type ``Value<-()``.
.. compiledfn:: (sc_image_type ...)

   An external function of type ``type<-(type Symbol i32 i32 i32 i32 Symbol Symbol)``.
.. compiledfn:: (sc_import_c ...)

   An external function of type ``Scope<->Error(String String List Scope)``.
.. compiledfn:: (sc_integer_type ...)

   An external function of type ``type<-(i32 bool)``.
.. compiledfn:: (sc_integer_type_is_signed ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_is_directory ...)

   An external function of type ``bool<-(String)``.
.. compiledfn:: (sc_is_file ...)

   An external function of type ``bool<-(String)``.
.. compiledfn:: (sc_key_type ...)

   An external function of type ``type<-(Symbol type)``.
.. compiledfn:: (sc_keyed_new ...)

   An external function of type ``Value<-(Symbol Value)``.
.. compiledfn:: (sc_label_new ...)

   An external function of type ``Value<-(i32 Symbol)``.
.. compiledfn:: (sc_label_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_launch_args ...)

   An external function of type ``λ(i32 i8(*)(*))<-()``.
.. compiledfn:: (sc_list_at ...)

   An external function of type ``Value<-(List)``.
.. compiledfn:: (sc_list_compare ...)

   An external function of type ``bool<-(List List)``.
.. compiledfn:: (sc_list_cons ...)

   An external function of type ``List<-(Value List)``.
.. compiledfn:: (sc_list_count ...)

   An external function of type ``i32<-(List)``.
.. compiledfn:: (sc_list_decons ...)

   An external function of type ``λ(Value List)<-(List)``.
.. compiledfn:: (sc_list_dump ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_list_join ...)

   An external function of type ``List<-(List List)``.
.. compiledfn:: (sc_list_next ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_list_repr ...)

   An external function of type ``String<-(List)``.
.. compiledfn:: (sc_list_reverse ...)

   An external function of type ``List<-(List)``.
.. compiledfn:: (sc_list_serialize ...)

   An external function of type ``String<-(List)``.
.. compiledfn:: (sc_load_history ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (sc_load_library ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (sc_load_object ...)

   An external function of type ``void<->Error(String)``.
.. compiledfn:: (sc_loop_arguments ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_loop_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_loop_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_map_get ...)

   An external function of type ``Value<->Error(Value)``.
.. compiledfn:: (sc_map_set ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_merge_new ...)

   An external function of type ``Value<-(Value Value)``.
.. compiledfn:: (sc_mutate_type ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_packed_tuple_type ...)

   An external function of type ``type<->Error(i32 type(*))``.
.. compiledfn:: (sc_parameter_is_variadic ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_parameter_name ...)

   An external function of type ``Symbol<-(Value)``.
.. compiledfn:: (sc_parameter_new ...)

   An external function of type ``Value<-(Symbol)``.
.. compiledfn:: (sc_parse_from_path ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (sc_parse_from_string ...)

   An external function of type ``Value<->Error(String)``.
.. compiledfn:: (sc_pointer_type ...)

   An external function of type ``type<-(type u64 Symbol)``.
.. compiledfn:: (sc_pointer_type_get_flags ...)

   An external function of type ``u64<-(type)``.
.. compiledfn:: (sc_pointer_type_get_storage_class ...)

   An external function of type ``Symbol<-(type)``.
.. compiledfn:: (sc_pointer_type_set_element_type ...)

   An external function of type ``type<-(type type)``.
.. compiledfn:: (sc_pointer_type_set_flags ...)

   An external function of type ``type<-(type u64)``.
.. compiledfn:: (sc_pointer_type_set_storage_class ...)

   An external function of type ``type<-(type Symbol)``.
.. compiledfn:: (sc_prompt ...)

   An external function of type ``λ(bool String)<-(String String)``.
.. compiledfn:: (sc_prove ...)

   An external function of type ``Value<->Error(Value)``.
.. compiledfn:: (sc_quote_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_realpath ...)

   An external function of type ``String<-(String)``.
.. compiledfn:: (sc_refer_flags ...)

   An external function of type ``u64<-(type)``.
.. compiledfn:: (sc_refer_storage_class ...)

   An external function of type ``Symbol<-(type)``.
.. compiledfn:: (sc_refer_type ...)

   An external function of type ``type<-(type u64 Symbol)``.
.. compiledfn:: (sc_sampled_image_type ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_save_history ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (sc_scope_at ...)

   An external function of type ``Value<->Error(Scope Value)``.
.. compiledfn:: (sc_scope_bind ...)

   An external function of type ``Scope<-(Scope Value Value)``.
.. compiledfn:: (sc_scope_bind_with_docstring ...)

   An external function of type ``Scope<-(Scope Value Value String)``.
.. compiledfn:: (sc_scope_docstring ...)

   An external function of type ``String<-(Scope Value)``.
.. compiledfn:: (sc_scope_get_parent ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_scope_local_at ...)

   An external function of type ``Value<->Error(Scope Value)``.
.. compiledfn:: (sc_scope_module_docstring ...)

   An external function of type ``String<-(Scope)``.
.. compiledfn:: (sc_scope_new ...)

   An external function of type ``Scope<-()``.
.. compiledfn:: (sc_scope_new_subscope ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_scope_new_subscope_with_docstring ...)

   An external function of type ``Scope<-(Scope String)``.
.. compiledfn:: (sc_scope_new_with_docstring ...)

   An external function of type ``Scope<-(String)``.
.. compiledfn:: (sc_scope_next ...)

   An external function of type ``λ(Value Value i32)<-(Scope i32)``.
.. compiledfn:: (sc_scope_next_deleted ...)

   An external function of type ``λ(Value i32)<-(Scope i32)``.
.. compiledfn:: (sc_scope_reparent ...)

   An external function of type ``Scope<-(Scope Scope)``.
.. compiledfn:: (sc_scope_unbind ...)

   An external function of type ``Scope<-(Scope Value)``.
.. compiledfn:: (sc_scope_unparent ...)

   An external function of type ``Scope<-(Scope)``.
.. compiledfn:: (sc_set_autocomplete_scope ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (sc_set_globals ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (sc_set_signal_abort ...)

   An external function of type ``void<-(bool)``.
.. compiledfn:: (sc_string_buffer ...)

   An external function of type ``λ(i8(*) usize)<-(String)``.
.. compiledfn:: (sc_string_compare ...)

   An external function of type ``i32<-(String String)``.
.. compiledfn:: (sc_string_count ...)

   An external function of type ``usize<-(String)``.
.. compiledfn:: (sc_string_join ...)

   An external function of type ``String<-(String String)``.
.. compiledfn:: (sc_string_lslice ...)

   An external function of type ``String<-(String usize)``.
.. compiledfn:: (sc_string_match ...)

   An external function of type ``λ(bool i32 i32)<->Error(String String)``.
.. compiledfn:: (sc_string_new ...)

   An external function of type ``String<-(i8(*) usize)``.
.. compiledfn:: (sc_string_new_from_cstr ...)

   An external function of type ``String<-(i8(*))``.
.. compiledfn:: (sc_string_rslice ...)

   An external function of type ``String<-(String usize)``.
.. compiledfn:: (sc_strip_qualifiers ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_switch_append_case ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_switch_append_default ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_switch_append_do ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_switch_append_pass ...)

   An external function of type ``void<-(Value Value Value)``.
.. compiledfn:: (sc_switch_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_symbol_count ...)

   An external function of type ``usize<-()``.
.. compiledfn:: (sc_symbol_is_variadic ...)

   An external function of type ``bool<-(Symbol)``.
.. compiledfn:: (sc_symbol_new ...)

   An external function of type ``Symbol<-(String)``.
.. compiledfn:: (sc_symbol_new_unique ...)

   An external function of type ``Symbol<-(String)``.
.. compiledfn:: (sc_symbol_style ...)

   An external function of type ``Symbol<-(Symbol)``.
.. compiledfn:: (sc_symbol_to_string ...)

   An external function of type ``String<-(Symbol)``.
.. compiledfn:: (sc_template_append_parameter ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_template_get_name ...)

   An external function of type ``Symbol<-(Value)``.
.. compiledfn:: (sc_template_is_inline ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_template_new ...)

   An external function of type ``Value<-(Symbol)``.
.. compiledfn:: (sc_template_parameter ...)

   An external function of type ``Value<-(Value i32)``.
.. compiledfn:: (sc_template_parameter_count ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_template_set_body ...)

   An external function of type ``void<-(Value Value)``.
.. compiledfn:: (sc_template_set_inline ...)

   An external function of type ``void<-(Value)``.
.. compiledfn:: (sc_template_set_name ...)

   An external function of type ``void<-(Value Symbol)``.
.. compiledfn:: (sc_tuple_type ...)

   An external function of type ``type<->Error(i32 type(*))``.
.. compiledfn:: (sc_type_alignof ...)

   An external function of type ``usize<->Error(type)``.
.. compiledfn:: (sc_type_at ...)

   An external function of type ``Value<->Error(type Symbol)``.
.. compiledfn:: (sc_type_bitcountof ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_type_compatible ...)

   An external function of type ``bool<-(type type)``.
.. compiledfn:: (sc_type_countof ...)

   An external function of type ``i32<->Error(type)``.
.. compiledfn:: (sc_type_debug_abi ...)

   An external function of type ``void<-(type)``.
.. compiledfn:: (sc_type_element_at ...)

   An external function of type ``type<->Error(type i32)``.
.. compiledfn:: (sc_type_field_index ...)

   An external function of type ``i32<->Error(type Symbol)``.
.. compiledfn:: (sc_type_field_name ...)

   An external function of type ``Symbol<->Error(type i32)``.
.. compiledfn:: (sc_type_get_docstring ...)

   An external function of type ``String<-(type Symbol)``.
.. compiledfn:: (sc_type_is_default_suffix ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_opaque ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_plain ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_refer ...)

   An external function of type ``bool<-(type)``.
.. compiledfn:: (sc_type_is_superof ...)

   An external function of type ``bool<-(type type)``.
.. compiledfn:: (sc_type_key ...)

   An external function of type ``λ(Symbol type)<-(type)``.
.. compiledfn:: (sc_type_kind ...)

   An external function of type ``i32<-(type)``.
.. compiledfn:: (sc_type_local_at ...)

   An external function of type ``Value<->Error(type Symbol)``.
.. compiledfn:: (sc_type_next ...)

   An external function of type ``λ(Symbol Value)<-(type Symbol)``.
.. compiledfn:: (sc_type_offsetof ...)

   An external function of type ``usize<->Error(type i32)``.
.. compiledfn:: (sc_type_set_docstring ...)

   An external function of type ``void<-(type Symbol String)``.
.. compiledfn:: (sc_type_set_symbol ...)

   An external function of type ``void<-(type Symbol Value)``.
.. compiledfn:: (sc_type_sizeof ...)

   An external function of type ``usize<->Error(type)``.
.. compiledfn:: (sc_type_storage ...)

   An external function of type ``type<->Error(type)``.
.. compiledfn:: (sc_type_string ...)

   An external function of type ``String<-(type)``.
.. compiledfn:: (sc_typename_type ...)

   An external function of type ``type<->Error(String type)``.
.. compiledfn:: (sc_typename_type_get_super ...)

   An external function of type ``type<-(type)``.
.. compiledfn:: (sc_typename_type_set_opaque ...)

   An external function of type ``void<->Error(type)``.
.. compiledfn:: (sc_typename_type_set_storage ...)

   An external function of type ``void<->Error(type type u32)``.
.. compiledfn:: (sc_typify ...)

   An external function of type ``Value<->Error(Closure i32 type(*))``.
.. compiledfn:: (sc_typify_template ...)

   An external function of type ``Value<->Error(Value i32 type(*))``.
.. compiledfn:: (sc_union_storage_type ...)

   An external function of type ``type<->Error(i32 type(*))``.
.. compiledfn:: (sc_unique_type ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_unquote_new ...)

   An external function of type ``Value<-(Value)``.
.. compiledfn:: (sc_value_anchor ...)

   An external function of type ``Anchor<-(Value)``.
.. compiledfn:: (sc_value_ast_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_block_depth ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_value_compare ...)

   An external function of type ``bool<-(Value Value)``.
.. compiledfn:: (sc_value_content_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_is_constant ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_value_is_pure ...)

   An external function of type ``bool<-(Value)``.
.. compiledfn:: (sc_value_kind ...)

   An external function of type ``i32<-(Value)``.
.. compiledfn:: (sc_value_kind_string ...)

   An external function of type ``String<-(i32)``.
.. compiledfn:: (sc_value_qualified_type ...)

   An external function of type ``type<-(Value)``.
.. compiledfn:: (sc_value_repr ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_tostring ...)

   An external function of type ``String<-(Value)``.
.. compiledfn:: (sc_value_type ...)

   An external function of type ``type<-(Value)``.
.. compiledfn:: (sc_value_unwrap ...)

   An external function of type ``Value<-(type Value)``.
.. compiledfn:: (sc_value_wrap ...)

   An external function of type ``Value<-(type Value)``.
.. compiledfn:: (sc_valueref_tag ...)

   An external function of type ``Value<-(Anchor Value)``.
.. compiledfn:: (sc_vector_type ...)

   An external function of type ``type<->Error(type usize)``.
.. compiledfn:: (sc_view_type ...)

   An external function of type ``type<-(type i32)``.
.. compiledfn:: (sc_write ...)

   An external function of type ``void<-(String)``.
.. compiledfn:: (set-autocomplete-scope! ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (set-globals! ...)

   An external function of type ``void<-(Scope)``.
.. compiledfn:: (set-signal-abort! ...)

   An external function of type ``void<-(bool)``.
.. compiledfn:: (spice-macro-verify-signature ...)

   A compiled function of type ``void<-(Value<->Error(Value)<*>)``.
.. compiledfn:: (symbol-handler ...)

   A compiled function of type ``λ(List Scope)<->Error(List Scope)``.
